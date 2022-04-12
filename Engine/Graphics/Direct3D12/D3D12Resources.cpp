#include "D3D12Resources.h"
#include "D3D12Core.h"

namespace primal::graphics::d3d12 {
	//	Descriptor Heap		/////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool descriptor_heap::initialize(u32 capacity, bool is_shader_visible) {
		//要考虑不同线程 可能的 data races
		std::lock_guard lock{ _mutex };
		assert(capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
		assert(!(_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE));
		if (_type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV || _type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV) {
			// 这两种类型是没得看的
			is_shader_visible = false;
		}
		release();	//初始化之前把以前的全部释放

		// 拿到主设备指针
		ID3D12Device* const device{ core::device() };
		assert(device);

		// 通过描述信息创建一个描述符堆
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Flags = is_shader_visible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NumDescriptors = capacity;
		desc.Type = _type;
		desc.NodeMask = 0;
		HRESULT hr{ S_OK };
		DXCall(hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_heap)));
		if (FAILED(hr)) return false;

		// 空闲句柄【因为是固定大小】，我们采用普通数组
		_free_handles = std::move(std::make_unique<u32[]>(capacity));
		_capacity = capacity;
		_size = 0;
		for (u32 i{ 0 }; i < capacity; ++i) {
			// 最开始全部都是空闲的，所以依次放进去序号就是了
			_free_handles[i] = i;
		}

		// 保证没有空闲的描述符是被推迟了的
		DEBUG_OP(for (u32 i{ 0 }; i < frame_buffer_count; ++i) assert(_deferred_free_indices[i].empty()));
		

		// 赋值基本属性
		_descriptor_size = device->GetDescriptorHandleIncrementSize(_type);
		_cpu_start = _heap->GetCPUDescriptorHandleForHeapStart();
		_gpu_start = is_shader_visible ? _heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };

		return true;
	}

	void descriptor_heap::release()
	{
		assert(!_size);
		core::deferred_release(_heap);	// 关闭资源描述符堆
	}

	void descriptor_heap::process_deferred_free(u32 frame_idx)
	{
		std::lock_guard lock{ _mutex };
		assert(frame_idx < frame_buffer_count);

		utl::vector<u32>& indices{ _deferred_free_indices[frame_idx]};	//通过引用得到当前帧中被推迟的文件描述符数组【里面是序号】
		if (!indices.empty()) {
			for ( auto& index : indices) {
				--_size;
				_free_handles[_size] = index;
			}
			indices.clear();
		}
	}

	descriptor_handle descriptor_heap::allocate()
	{
		std::lock_guard lock{ _mutex };
		assert(_heap);
		assert(_size < _capacity);
		const u32 index{ _free_handles[_size] };	// 获取空闲句柄的最后一个位置【想一想_size = 0时就是第一个】
		const u32 offset{ index * _descriptor_size };	// 这个句柄在_heap的偏移量
		++_size;

		descriptor_handle handle;	//要返回的handle【等会根据offset偏移】
		handle.cpu.ptr = _cpu_start.ptr + offset;
		if (is_shader_visible()) {
			handle.gpu.ptr = _gpu_start.ptr + offset;
		}

		DEBUG_OP(handle.container = this);
		DEBUG_OP(handle.index = index);
		return handle;
	}

	void descriptor_heap::free(descriptor_handle& handle)
	{
		if (!handle.is_valid()) return;
		std::lock_guard lock{ _mutex };

		assert(_heap && _size);
		assert(handle.container == this);
		assert(handle.cpu.ptr >= _cpu_start.ptr);
		assert((handle.cpu.ptr - _cpu_start.ptr) % _descriptor_size == 0);
		assert(handle.index < _capacity);

		const u32 index{ (u32)((handle.cpu.ptr - _cpu_start.ptr) / _descriptor_size) };	//找到这个handle在_heap上的序号
		assert(handle.index == index);

		//考虑可能被之前的frame使用，需要安全的推迟释放时间
		const u32 frame_idx{ core::current_frame_index() };	//获取当前的帧idx
		_deferred_free_indices[frame_idx].push_back(index);	//将当前文件描述符推到等待推迟处理的数组中
		core::set_deferred_releases_flag();	//设置推迟处理标志

		handle = {};	//重置handle
	}
}