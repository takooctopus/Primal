#pragma once
#include "D3D12CommonHeaders.h"

namespace primal::graphics::d3d12 {
	//	Descriptor Heap		/////////////////////////////////////////////////////////////////////////////////////////////////////////
	class descriptor_heap;	// ��������

	/// <summary>
	/// �������������һ��cpu�ĺ�һ��gpu�ġ�
	/// </summary>
	struct descriptor_handle {
		D3D12_CPU_DESCRIPTOR_HANDLE		cpu{};	// cpu�������
		D3D12_GPU_DESCRIPTOR_HANDLE		gpu{};	// gpu�������
		u32								index{ u32_invalid_id };	// ��debug only���������������


		/// <summary>
		/// �������Ƿ����
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		constexpr bool is_valid() const { return cpu.ptr != 0; }

		/// <summary>
		/// �������Ƿ���Ⱦ�ɼ�
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		constexpr bool is_shader_visible() const { return gpu.ptr != 0; }
#ifdef _DEBUG
	private:
		friend class descriptor_heap;	// ��Ԫ�࣬���� descriptor_heap �ڵ���ʱ����˽������
		descriptor_heap*		container{ nullptr };	// ��debug only�� ���������������descriptor_heapָ��

#endif // _DEBUG

	};

	/// <summary>
	/// ��������
	/// </summary>
	class descriptor_heap {
	public:
		explicit descriptor_heap(D3D12_DESCRIPTOR_HEAP_TYPE type) :_type(type) {}
		DISABLE_COPY_AND_MOVE(descriptor_heap);
		~descriptor_heap() { assert(!_heap); }

		/// <summary>
		/// ��ʼ����������
		/// </summary>
		/// <param name="capacity"></param>
		/// <param name="is_shader_visible"></param>
		/// <returns></returns>
		[[nodiscard]]
		bool initialize(u32 capacity, bool is_shader_visible);

		/// <summary>
		/// �ͷ��������ѵ�������Դ
		/// </summary>
		void release();

		/// <summary>
		/// �ͷű��Ƴٵġ�Ӧ�����С�������
		/// </summary>
		/// <param name="frame_idx"></param>
		void process_deferred_free(u32 frame_idx);

		/// <summary>
		/// ����һ���ļ�������
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		descriptor_handle allocate();

		/// <summary>
		/// ����һ���ļ�������
		/// </summary>
		/// <param name="handle"></param>
		void free(descriptor_handle& handle);

		[[nodiscard]]
		constexpr D3D12_DESCRIPTOR_HEAP_TYPE type() const { return _type; }
		[[nodiscard]]
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE cpu_start() const { return _cpu_start; }
		[[nodiscard]]
		constexpr D3D12_GPU_DESCRIPTOR_HANDLE gpu_start() const { return _gpu_start; }
		[[nodiscard]]
		constexpr ID3D12DescriptorHeap* const heap() const { return _heap; }
		[[nodiscard]]
		constexpr u32 capacity() const { return _capacity; }
		[[nodiscard]]
		constexpr u32 size() const { return _size; }
		[[nodiscard]]
		constexpr u32 descriptor_size() const { return _descriptor_size; }
		[[nodiscard]]
		constexpr bool is_shader_visible() const { return _gpu_start.ptr != 0; }

	private:
		std::mutex							_mutex{};		// mutex
		ID3D12DescriptorHeap*				_heap;			// ��ָ�룬 ָ���ļ��������ѵĳ�ʼλ��
		D3D12_CPU_DESCRIPTOR_HANDLE			_cpu_start{};	// cpu heap�ĳ�ʼλ��
		D3D12_GPU_DESCRIPTOR_HANDLE			_gpu_start{};	// gpu heap�ĳ�ʼλ��
		std::unique_ptr<u32[]>				_free_handles{};	// ���о��������ջ�õġ�
		utl::vector<u32>					_deferred_free_indices[frame_buffer_count]{};	// ���Ƴٵ���� ��ά���黹�С�ÿ��frame�б��Ƴٵ���ż��ϡ�
		u32									_capacity{ 0 };	// �ѵ�������
		u32									_size{ 0 };	// �����ڵĴ�С��Ӧ����Զ<=_capacity��
		u32									_descriptor_size{ 0 };	//�ļ���������С
		const D3D12_DESCRIPTOR_HEAP_TYPE	_type{};	//�ļ�������������
	};





	//	RENDER TEXTURE		/////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// d3d12�����ʼ���ṹ��
	/// </summary>
	struct d3d12_texture_init_info {
		ID3D12Heap1*						heap{nullptr};			// ����������ָ��Ļ��������������ʼ��ʱʹ��CreatePlacedResource()�ӵ����У�Ҫ����Ҫ�ŵ�Ĭ����
		ID3D12Resource*						resource{ nullptr };	// ��Դ
		D3D12_SHADER_RESOURCE_VIEW_DESC*	srv_desc{ nullptr };	// srv��ɫ����Դ��ͼ ������Ϣ
		D3D12_RESOURCE_DESC*				desc{ nullptr };		// ��Դ������Ϣ
		D3D12_RESOURCE_ALLOCATION_INFO1		allocation_info{};		// heap��nullʱ��ƫ��������Ϣ
		D3D12_RESOURCE_STATES				initial_state{};		// ��ʼ��״̬
		D3D12_CLEAR_VALUE					clear_value{};			// ��ԴĬ�����ֵ
	};


	/// <summary>
	/// d3d12����
	/// </summary>
	class d3d12_texture {
	public:
		constexpr static u32 max_mips{ 14 };	//���֧��16K

		d3d12_texture() = default;
		explicit d3d12_texture(d3d12_texture_init_info info);
		DISABLE_COPY(d3d12_texture);
		constexpr d3d12_texture(d3d12_texture&& o)
			:_resource(o._resource), _srv(o._srv)
		{
			o.reset();
		}

		~d3d12_texture() {
			release();
		}

		constexpr d3d12_texture& operator=(d3d12_texture&& o) {
			assert(this != &o);
			if (this != &o) {
				release();
				move(o);
			}
			return *this;
		}
		void release();
		constexpr ID3D12Resource* resource() const {
			return _resource;
		}
		constexpr descriptor_handle srv() const {
			return _srv;
		}
	private:
		constexpr void move(d3d12_texture &o) {
			_resource = o._resource;
			_srv = o._srv;
			o.reset();
		}

		constexpr void reset() {
			_resource = nullptr;
			_srv = {};
		}
		ID3D12Resource*		_resource{ nullptr };	//��Դ
		descriptor_handle	_srv;	//��Դ���������shader resource view����������һ����ɫ��
	};


	/// <summary>
	/// d3d12��Ⱦ����
	/// </summary>
	class d3d12_render_texture {
	public:
		d3d12_render_texture() = default;

		explicit d3d12_render_texture(d3d12_texture_init_info info);

		~d3d12_render_texture() {
			release();
		}

		DISABLE_COPY(d3d12_render_texture);
		constexpr d3d12_render_texture(d3d12_render_texture&& o) :
			_texture{ std::move(o._texture) }, _mip_count{o._mip_count}{
			for (u32 i{ 0 }; i < _mip_count; ++i) {
				_rtv[i] = o._rtv[i];
			}
			o.reset();
		}

		constexpr d3d12_render_texture& operator=(d3d12_render_texture&& o) {
			assert(this != &o);
			if (this != &o) {
				release();
				move(o);
			}
			return *this;
		}


		void release();
		[[nodiscard]]
		constexpr u32 mip_count() const { return _mip_count; }
		[[nodiscard]]
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE rtv(u32 mip_index) const { assert(mip_index < _mip_count); return _rtv[mip_index].cpu; }
		[[nodiscard]]
		constexpr descriptor_handle srv() const { return _texture.srv(); }
		[[nodiscard]]
		constexpr ID3D12Resource* const resource() const { return _texture.resource(); }

	private:
		constexpr void move(d3d12_render_texture& o) {
			_texture = std::move(o._texture);
			_mip_count = o._mip_count;
			for (u32 i{ 0 }; i < _mip_count; ++i) {
				_rtv[i] = o._rtv[i];
			}
			o.reset();
		}

		constexpr void reset() {
			for (u32 i{ 0 }; i < _mip_count; ++i) {
				_rtv[i] = {};
			}
			_mip_count = 0;
		}

		d3d12_texture				_texture{};						// ����һ��d3d12������
		descriptor_handle			_rtv[d3d12_texture::max_mips]{};	// render target view �ռ��кܶ��
		u32							_mip_count{ 0 };	// ��ǰ��ָ����

	};

	//	DEPTH BUFFER		/////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// d3d12��Ȼ���
	/// </summary>
	class d3d12_depth_buffer {
	public:
		d3d12_depth_buffer() = default;
		explicit d3d12_depth_buffer(d3d12_texture_init_info info);
		DISABLE_COPY(d3d12_depth_buffer);
		~d3d12_depth_buffer() { release(); }
		constexpr d3d12_depth_buffer(d3d12_depth_buffer&& o) :
			_texture{ std::move(o._texture) }, _dsv{o._dsv} {
			o._dsv = {};
		}
		constexpr d3d12_depth_buffer& operator=(d3d12_depth_buffer&& o) {
			assert(this != &o);
			if (this != &o) {
				_texture = std::move(o._texture);
				_dsv = o._dsv;
				o._dsv = {};
			}
			return *this;
		}

		void release();
		[[nodiscard]]
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE dsv() const { return _dsv.cpu; }
		[[nodiscard]]
		constexpr descriptor_handle srv() const { return _texture.srv(); }
		[[nodiscard]]
		constexpr ID3D12Resource* const resource() const { return _texture.resource(); }
	private:
		d3d12_texture				_texture{};	//ͬ���ģ�����һ������	
		descriptor_handle			_dsv{};	//depth stencil ��Ȼ���
	};



	
}