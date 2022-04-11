#pragma once
#include "D3D12CommonHeaders.h"

namespace primal::graphics::d3d12 {
	class descriptor_heap;	// 先制声明

	/// <summary>
	/// 描述句柄【包含一个cpu的和一个gpu的】
	/// </summary>
	struct descriptor_handle {
		D3D12_CPU_DESCRIPTOR_HANDLE		cpu{};	// cpu描述句柄
		D3D12_GPU_DESCRIPTOR_HANDLE		gpu{};	// gpu描述句柄


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
		u32						index{ u32_invalid_id };	// 【debug only】描述符现在序号

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
}