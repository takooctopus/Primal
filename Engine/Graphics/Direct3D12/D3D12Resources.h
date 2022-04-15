#pragma once
#include "D3D12CommonHeaders.h"

namespace primal::graphics::d3d12 {
	//	Descriptor Heap		/////////////////////////////////////////////////////////////////////////////////////////////////////////
	class descriptor_heap;	// 先制声明

	/// <summary>
	/// 描述句柄【包含一个cpu的和一个gpu的】
	/// </summary>
	struct descriptor_handle {
		D3D12_CPU_DESCRIPTOR_HANDLE		cpu{};	// cpu描述句柄
		D3D12_GPU_DESCRIPTOR_HANDLE		gpu{};	// gpu描述句柄
		u32								index{ u32_invalid_id };	// 【debug only】描述符现在序号


		/// <summary>
		/// 这个句柄是否可用
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		constexpr bool is_valid() const { return cpu.ptr != 0; }

		/// <summary>
		/// 这个句柄是否渲染可见
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		constexpr bool is_shader_visible() const { return gpu.ptr != 0; }
#ifdef _DEBUG
	private:
		friend class descriptor_heap;	// 友元类，方便 descriptor_heap 在调试时调用私有属性
		descriptor_heap*		container{ nullptr };	// 【debug only】 这个描述符所属的descriptor_heap指针

#endif // _DEBUG

	};

	/// <summary>
	/// 描述符堆
	/// </summary>
	class descriptor_heap {
	public:
		explicit descriptor_heap(D3D12_DESCRIPTOR_HEAP_TYPE type) :_type(type) {}
		DISABLE_COPY_AND_MOVE(descriptor_heap);
		~descriptor_heap() { assert(!_heap); }

		/// <summary>
		/// 初始化描述符堆
		/// </summary>
		/// <param name="capacity"></param>
		/// <param name="is_shader_visible"></param>
		/// <returns></returns>
		[[nodiscard]]
		bool initialize(u32 capacity, bool is_shader_visible);

		/// <summary>
		/// 释放描述符堆的所有资源
		/// </summary>
		void release();

		/// <summary>
		/// 释放被推迟的【应当空闲】描述符
		/// </summary>
		/// <param name="frame_idx"></param>
		void process_deferred_free(u32 frame_idx);

		/// <summary>
		/// 分配一个文件描述符
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		descriptor_handle allocate();

		/// <summary>
		/// 回收一个文件描述符
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
		ID3D12DescriptorHeap*				_heap;			// 堆指针， 指向文件描述符堆的初始位置
		D3D12_CPU_DESCRIPTOR_HANDLE			_cpu_start{};	// cpu heap的初始位置
		D3D12_GPU_DESCRIPTOR_HANDLE			_gpu_start{};	// gpu heap的初始位置
		std::unique_ptr<u32[]>				_free_handles{};	// 空闲句柄【当作栈用的】
		utl::vector<u32>					_deferred_free_indices[frame_buffer_count]{};	// 被推迟的序号 二维数组还行【每个frame中被推迟的序号集合】
		u32									_capacity{ 0 };	// 堆的总容量
		u32									_size{ 0 };	// 堆现在的大小【应该永远<=_capacity】
		u32									_descriptor_size{ 0 };	//文件描述符大小
		const D3D12_DESCRIPTOR_HEAP_TYPE	_type{};	//文件名描述符类型
	};





	//	RENDER TEXTURE		/////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// d3d12纹理初始化结构体
	/// </summary>
	struct d3d12_texture_init_info {
		ID3D12Heap1*						heap{nullptr};			// 如果有这个堆指针的话，我们在里面初始化时使用CreatePlacedResource()扔到堆中，要不就要放到默认里
		ID3D12Resource*						resource{ nullptr };	// 资源
		D3D12_SHADER_RESOURCE_VIEW_DESC*	srv_desc{ nullptr };	// srv着色器资源视图 描述信息
		D3D12_RESOURCE_DESC*				desc{ nullptr };		// 资源描述信息
		D3D12_RESOURCE_ALLOCATION_INFO1		allocation_info{};		// heap非null时的偏移量等信息
		D3D12_RESOURCE_STATES				initial_state{};		// 初始化状态
		D3D12_CLEAR_VALUE					clear_value{};			// 资源默认清除值
	};


	/// <summary>
	/// d3d12纹理
	/// </summary>
	class d3d12_texture {
	public:
		constexpr static u32 max_mips{ 14 };	//最大支持16K

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
		ID3D12Resource*		_resource{ nullptr };	//资源
		descriptor_handle	_srv;	//资源描述句柄【shader resource view】这里面是一个着色器
	};


	/// <summary>
	/// d3d12渲染纹理
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

		d3d12_texture				_texture{};						// 包含一个d3d12纹理类
		descriptor_handle			_rtv[d3d12_texture::max_mips]{};	// render target view 空间有很多个
		u32							_mip_count{ 0 };	// 当前的指令数

	};

	//	DEPTH BUFFER		/////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// d3d12深度缓冲
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
		d3d12_texture				_texture{};	//同样的，包含一个纹理	
		descriptor_handle			_dsv{};	//depth stencil 深度缓冲
	};



	
}