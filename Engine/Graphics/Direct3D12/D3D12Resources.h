#pragma once
#include "D3D12CommonHeaders.h"

namespace primal::graphics::d3d12 {
	class descriptor_heap;	// ��������

	/// <summary>
	/// �������������һ��cpu�ĺ�һ��gpu�ġ�
	/// </summary>
	struct descriptor_handle {
		D3D12_CPU_DESCRIPTOR_HANDLE		cpu{};	// cpu�������
		D3D12_GPU_DESCRIPTOR_HANDLE		gpu{};	// gpu�������


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
		u32						index{ u32_invalid_id };	// ��debug only���������������

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
}