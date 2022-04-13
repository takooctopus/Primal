#include "D3D12Core.h"
#include "D3D12CommonHeaders.h"
#include "D3D12Surface.h"
#include "D3D12Shaders.h"

using namespace Microsoft::WRL;

namespace primal::graphics::d3d12::core {

	namespace {
		// 【static】d3d12命令
		class d3d12_command {
		public:
			d3d12_command() = default;
			DISABLE_COPY_AND_MOVE(d3d12_command);
			explicit d3d12_command(id3d12_device* const device, D3D12_COMMAND_LIST_TYPE type) {
				HRESULT hr{ S_OK };
				D3D12_COMMAND_QUEUE_DESC desc{};
				desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				desc.NodeMask = 0;
				desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
				desc.Type = type;
				DXCall(hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_cmd_queue)));
				if (FAILED(hr)) goto _error;

				NAME_D3D12_OBJECT(_cmd_queue,
					type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command Queue" :
					type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command Queue" : L"Command Queue"
				);

				for (u32 i{ 0 }; i < frame_buffer_count; ++i) {
					command_frame& frame{ _cmd_frames[i] };
					DXCall(hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&frame.cmd_allocator)));
					if (FAILED(hr)) goto _error;

					NAME_D3D12_OBJECT_INDEXED(frame.cmd_allocator, i,
						type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command Allocator" :
						type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command Allocator" : L"Command Allocator"
					);
				}

				DXCall(hr = device->CreateCommandList(0, type, _cmd_frames[0].cmd_allocator, nullptr, IID_PPV_ARGS(&_cmd_list)));
				if (FAILED(hr)) goto _error;

				DXCall(_cmd_list->Close());
				NAME_D3D12_OBJECT(_cmd_list,
					type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command List" :
					type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command List" : L"Command List"
				);

				DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));
				if (FAILED(hr)) goto _error;
				NAME_D3D12_OBJECT(_fence, L"D3D12 Fence");

				_fence_event = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
				assert(_fence_event);

				return;

			_error:
				release();
			}

			~d3d12_command() {
				assert(!_cmd_queue && !_cmd_list && !_fence);
			}

			/// <summary>
			/// 阻塞当前的frame直到被通知 最后重置command list/allocator
			/// </summary>
			void begin_frame() {
				command_frame& frame{ _cmd_frames[_frame_index] };
				frame.wait(_fence_event, _fence);
				DXCall(frame.cmd_allocator->Reset());	// 释放前一次recorded command所占用的内存
				DXCall(_cmd_list->Reset(frame.cmd_allocator, nullptr));	// 重新打开list以记录command
			}

			/// <summary>
			/// singnal the fence with the new fence value
			/// </summary>
			void end_frame() {
				DXCall(_cmd_list->Close());
				ID3D12CommandList* const cmd_lists[]{ _cmd_list };	//常指针， 只能指向_cmd_list
				_cmd_queue->ExecuteCommandLists(_countof(cmd_lists), &cmd_lists[0]);	//处理所有的cmd

				u64& fence_value{ _fence_value };	//引用获取当前的栅栏值
				++fence_value;	//因为完成了一帧，计数加1
				command_frame& frame{ _cmd_frames[_frame_index] };	//引用获取当前帧
				frame.fence_value = fence_value;	//更新当前帧的栅栏值
				_cmd_queue->Signal(_fence, fence_value);	//唤醒所有栅栏值小于这个的帧

				_frame_index = (_frame_index + 1) % frame_buffer_count;	//idx循环+1
			}

			/// <summary>
			/// 遍历所有帧，阻塞直到当前帧的值到达栅栏值
			/// </summary>
			void flush() {
				for (u32 i{ 0 }; i < frame_buffer_count; ++i) {
					_cmd_frames[i].wait(_fence_event, _fence);
				}
				_frame_index = 0;
			}
			/// <summary>
			/// 释放
			/// </summary>
			void release() {
				flush();	//先全部阻塞到完成
				core::release(_fence);	//释放栅栏对象
				_fence_value = 0;	// 重置栅栏值

				CloseHandle(_fence_event);	//关闭事件句柄
				_fence_event = nullptr;	//重置句柄指针

				core::release(_cmd_queue);	//释放D3D object
				core::release(_cmd_list);	//释放D3D object

				for (u32 i{ 0 }; i < frame_buffer_count; ++i) {
					_cmd_frames[i].release();	//释放D3D object
				}
			}
			[[nodiscard]]
			constexpr ID3D12CommandQueue* const command_queue() const { return _cmd_queue; }
			[[nodiscard]]
			constexpr id3d12_graphics_command_list* const command_list() const { return _cmd_list; }
			[[nodiscard]]
			constexpr u32 frame_index() const { return _frame_index; }

		private:
			/// <summary>
			/// 【private】命令帧
			/// </summary>
			struct command_frame
			{
				ID3D12CommandAllocator* cmd_allocator{ nullptr };
				u64 fence_value{ 0 };

				/// <summary>
				/// 阻塞当前的frame直到被通知
				/// </summary>
				/// <param name="fence_event"></param>
				/// <param name="fence"></param>
				void wait(HANDLE fence_event, ID3D12Fence1* fence) {
					assert(fence && fence_event);
					// 如果当前的fence小于给出的fence，则说明gpu上的command list 还没有完成
					// 因为它还没有 _cmd_queue->Signal()
					if (fence->GetCompletedValue() < fence_value) {
						DXCall(fence->SetEventOnCompletion(fence_value, fence_event));
						// 等待直到fence触发trigger【当前值到达fence】
						WaitForSingleObject(fence_event, INFINITE);
					}
				}
				/// <summary>
				/// 释放objects
				/// </summary>
				void release() {
					core::release(cmd_allocator);
					fence_value = 0;
				}
			};

			ID3D12CommandQueue*						_cmd_queue{ nullptr };	//【private】命令队列
			id3d12_graphics_command_list*				_cmd_list{ nullptr };	//【private】图形命令列表
			ID3D12Fence1*							_fence{ nullptr };	//【private】栅栏
			u64										_fence_value{ 0 };	//【private】当前栅栏值
			command_frame							_cmd_frames[frame_buffer_count]{};	//【private】命令帧数组【放的具体命令帧】
			HANDLE									_fence_event{ nullptr };	//【private】栅栏事件句柄
			u32										_frame_index{ 0 };	//【private】当前帧序号
		};

		using surface_collection = utl::free_list<d3d12_surface>;

		id3d12_device*								main_device{ nullptr };	// 【static】 指向主设备的指针
		IDXGIFactory7*								dxgi_factory{ nullptr };	// 【static】工厂指针
		d3d12_command								gfx_command;	//【static】d3d12指令 
		surface_collection							surfaces;	//【static】表面数组【调用create_surface就添加到这里面】

		descriptor_heap								rtv_desc_heap{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV };	//【static】渲染目标缓冲区描述符
		descriptor_heap								dsv_desc_heap{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV };	//【static】深度模板缓冲区描述符
		descriptor_heap								srv_desc_heap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };	//【static】常量缓冲区描述符、着色器资源缓冲描述符和随机访问缓冲描述符 【visible】
		descriptor_heap								uav_desc_heap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };	//【static】常量缓冲区描述符、着色器资源缓冲描述符和随机访问缓冲描述符 【!visible】
		utl::vector<IUnknown*>						deferred_releases[frame_buffer_count]{};	//【static】
		u32											deferred_releases_flag[frame_buffer_count]{};	// 【static】 每个frame推迟的描述符标志
		std::mutex									deferred_releases_mutex{};	// 【static】 互斥量


		constexpr D3D_FEATURE_LEVEL minimum_feature_level{ D3D_FEATURE_LEVEL_11_0 };	//【static】 支持特性的最小版本


		/// <summary>
		/// 【static】 创建失败了记得退出
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		bool failed_init() {
			shutdown();
			return false;
		}

		/// <summary>
		/// 【static】选择主适配器
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		IDXGIAdapter4* determine_main_adapter() {
			IDXGIAdapter4* adapter{ nullptr };
			// get adapters in desc order of performance
			for (u32 i{ 0 };
				dxgi_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
				++i
				) {
				// 选第一个符合我们的特性要求的
				if (SUCCEEDED(D3D12CreateDevice(adapter, minimum_feature_level, __uuidof(ID3D12Device), nullptr))) {
					return adapter;
				}
				release(adapter);
			}
			return nullptr;
		}

		/// <summary>
		///	【static】 获取支持的最大特性
		/// </summary>
		/// <param name="adapter"></param>
		/// <returns></returns>
		[[nodiscard]]
		D3D_FEATURE_LEVEL get_max_feature_level(IDXGIAdapter4* adapter) {
			constexpr D3D_FEATURE_LEVEL feature_levels[4]{
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_12_1
			};

			D3D12_FEATURE_DATA_FEATURE_LEVELS feature_level_info{};
			feature_level_info.NumFeatureLevels = _countof(feature_levels);
			feature_level_info.pFeatureLevelsRequested = feature_levels;

			ComPtr<ID3D12Device> device;
			DXCall(D3D12CreateDevice(adapter, minimum_feature_level, IID_PPV_ARGS(&device)));
			DXCall(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &feature_level_info, sizeof(feature_level_info)));
			return feature_level_info.MaxSupportedFeatureLevel;
		}

		/// <summary>
		/// 【static】 处理frame中被推迟的资源描述符
		/// </summary>
		/// <param name="frame_idx"></param>
		void __declspec(noinline)
			process_deferred_releases(u32 frame_idx) {
			std::lock_guard lock{ deferred_releases_mutex };
			deferred_releases_flag[frame_idx] = 0;

			// release pending resources
			rtv_desc_heap.process_deferred_free(frame_idx);
			dsv_desc_heap.process_deferred_free(frame_idx);
			srv_desc_heap.process_deferred_free(frame_idx);
			uav_desc_heap.process_deferred_free(frame_idx);

			utl::vector<IUnknown*>& resources{ deferred_releases[frame_idx] };
			if (!resources.empty()) {
				for (auto&& resource : resources)  release(resource);
				resources.clear();
			}
		}


	} //anonymous namespace


	namespace detail {
		void deferred_release(IUnknown* resouece) {
			const u32 frame_idx{ current_frame_index() };	//当前frame的id
			std::lock_guard lock{ deferred_releases_mutex };
			deferred_releases[frame_idx].push_back(resouece); //deferred_releases数组中加入要释放的描述符【相当于扔到队列里去了】
			set_deferred_releases_flag();	//设置当前帧的推迟标志

		}
	} // namespace detail


	/// <summary>
	/// D3D12的初始化函数，对外使用函数指针进行调用
	/// </summary>
	/// <returns></returns>
	[[nodiscard]]
	bool initialize()
	{
		// 判断adapter 
		// 判断maxmium feature level
		// create a ID3D12Device (virtual adapter)

		if (main_device) shutdown();

		u32 dxgi_factory_flags{ 0 };
#ifdef _DEBUG
		{
			// 开启这个debug需要"Graphic Tools" optional feature
			ComPtr<ID3D12Debug3> debug_interface;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface)))) {
				debug_interface->EnableDebugLayer();
			}
			else {
				OutputDebugStringA("Warning: D3D12 Debug is not available. Verify that graphic tools optional feature is installed in this device.");
			}
			debug_interface->EnableDebugLayer();
			dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
		}
#endif	//_DEBUG
		//CreateDXGIFactory2(0, __uuidof(IDXGIFactory7), (void**)&dxgi_factory);
		HRESULT hr{ S_OK };
		DXCall(hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&dxgi_factory)));	//和上面一样的
		if (FAILED(hr)) return failed_init();

		ComPtr<IDXGIAdapter4> main_adapter;
		main_adapter.Attach(determine_main_adapter());
		if (!main_adapter) return failed_init();

		D3D_FEATURE_LEVEL max_feature_level{ get_max_feature_level(main_adapter.Get()) };
		assert(max_feature_level >= minimum_feature_level);
		if (max_feature_level < minimum_feature_level) return failed_init();

		DXCall(hr = D3D12CreateDevice(main_adapter.Get(), max_feature_level, IID_PPV_ARGS(&main_device)));
		if (FAILED(hr)) return failed_init();

#ifdef _DEBUG
		{
			ComPtr<ID3D12InfoQueue> info_queue;
			DXCall(main_device->QueryInterface(IID_PPV_ARGS(&info_queue)));

			info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		}
#endif // _DEBUG

		bool result{ true };
		result &= rtv_desc_heap.initialize(512, false);
		result &= dsv_desc_heap.initialize(512, false);
		result &= srv_desc_heap.initialize(4096, true);
		result &= uav_desc_heap.initialize(512, false);
		if (!result) return failed_init();

		new (&gfx_command) d3d12_command(main_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		if (!gfx_command.command_queue()) return failed_init();

		// shutdown modules
		if (!shaders::initialize()) {
			return failed_init();
		}

		NAME_D3D12_OBJECT(main_device, L"MAIN DEVICE");
		NAME_D3D12_OBJECT(rtv_desc_heap.heap(), L"RTV Descriptor Heap");
		NAME_D3D12_OBJECT(dsv_desc_heap.heap(), L"DSV Descriptor Heap");
		NAME_D3D12_OBJECT(srv_desc_heap.heap(), L"SRV Descriptor Heap");
		NAME_D3D12_OBJECT(uav_desc_heap.heap(), L"UAV Descriptor Heap");

		return true;
	}

	/// <summary>
	/// D3D12的结束函数
	/// </summary>
	void shutdown()
	{
		gfx_command.release();

		// 得先处理这些被推迟的资源，才能释放heap
		for (u32 i{ 0 }; i < frame_buffer_count; ++i) {
			process_deferred_releases(i);
		}
		
		// initialize modules
		shaders::shutdown();
		
		release(dxgi_factory);

		// 一些模组会在其关闭时释放描述符,得多用一次process_deferred_free()
		rtv_desc_heap.process_deferred_free(0);
		dsv_desc_heap.process_deferred_free(0);
		srv_desc_heap.process_deferred_free(0);
		uav_desc_heap.process_deferred_free(0);

		rtv_desc_heap.release();
		dsv_desc_heap.release();
		srv_desc_heap.release();
		uav_desc_heap.release();

		// 某些资源在它们shutdown/reset/clear时要用deferred release，我们再调用一次去真正地清理
		process_deferred_releases(0);


#ifdef _DEBUG
		{
			{
				ComPtr<ID3D12InfoQueue> info_queue;
				DXCall(main_device->QueryInterface(IID_PPV_ARGS(&info_queue)));

				info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
				info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
				info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			}

			ComPtr<ID3D12DebugDevice2> debug_device;
			DXCall(main_device->QueryInterface(IID_PPV_ARGS(&debug_device)));
			release(main_device);
			DXCall(debug_device->ReportLiveDeviceObjects(
				D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL
			));
		}
#endif // _DEBUG

		release(main_device);
	}


	id3d12_device* const device()
	{
		return main_device;
	}

	descriptor_heap& rtv_heap() {
		return rtv_desc_heap;
	}
	descriptor_heap& dsv_heap() {
		return dsv_desc_heap;
	}
	descriptor_heap& srv_heap() {
		return srv_desc_heap;
	}
	descriptor_heap& uav_heap() {
		return uav_desc_heap;
	}

	u32 current_frame_index()
	{
		return gfx_command.frame_index();
	}

	void set_deferred_releases_flag()
	{
		deferred_releases_flag[current_frame_index()] = 1;
	}
	surface create_surface(platform::window window)
	{
		surface_id id{ surfaces.add(window) }; //传入窗口初始化一个d3d12_surface并加入surfaces数组中
		surfaces[id].create_swap_chain(dxgi_factory, gfx_command.command_queue());
		return surface{ id };
	}
	void remove_surface(surface_id id)
	{
		gfx_command.flush();	//保证gpu中已经结束
		surfaces.remove(id);
	}
	void resize_surface(surface_id id, u32 width, u32 height)
	{
		gfx_command.flush();	//保证gpu中已经结束
		surfaces[id].resize(width, height);
	}
	u32 surface_width(surface_id id)
	{
		return surfaces[id].width();
	}
	u32 surface_height(surface_id id)
	{
		return surfaces[id].height();
	}
	void render_surface(surface_id id)
	{
		// 等待gpu完成command allocator
		// 一旦完成就重置allocator
		// 这里释放被用来存储command的内存空间
		gfx_command.begin_frame();

		id3d12_graphics_command_list* cmd_list{ gfx_command.command_list() };

		const u32 frame_idx{ current_frame_index() };
		if (deferred_releases_flag[frame_idx]) {
			process_deferred_releases(frame_idx);
		}

		const d3d12_surface& surface{ surfaces[id] };
		surface.present();

		//记录操作
		//....
		//完成记录， 执行
		//通知下一帧并将fence_index加一
		gfx_command.end_frame();
	}

}