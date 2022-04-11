#include "D3D12Core.h"
#include "D3D12CommonHeaders.h"

using namespace Microsoft::WRL;

namespace primal::graphics::d3d12::core {
	namespace {

		class d3d12_command {
		public:
			d3d12_command() = default;
			DISABLE_COPY_AND_MOVE(d3d12_command);
			explicit d3d12_command(ID3D12Device8* const device, D3D12_COMMAND_LIST_TYPE type) {
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
				_cmd_queue->ExecuteCommandLists(_countof(cmd_lists), &cmd_lists[0]);

				u64& fence_value{ _fence_value };
				++fence_value;
				command_frame& frame{ _cmd_frames[_frame_index] };
				frame.fence_value = fence_value;
				_cmd_queue->Signal(_fence, fence_value);

				_frame_index = (_frame_index + 1) % frame_buffer_count;
			}

			void flush() {
				for (u32 i{ 0 }; i < frame_buffer_count; ++i) {
					_cmd_frames[i].wait(_fence_event, _fence);
				}
				_frame_index = 0;
			}

			void release() {
				flush();
				core::release(_fence);
				_fence_value = 0;

				CloseHandle(_fence_event);
				_fence_event = nullptr;

				core::release(_cmd_queue);
				core::release(_cmd_list);

				for (u32 i{ 0 }; i < frame_buffer_count; ++i) {
					_cmd_frames[i].release();
				}
			}

			constexpr ID3D12CommandQueue* const command_queue() const { return _cmd_queue; }
			constexpr ID3D12GraphicsCommandList6* const command_list() const { return _cmd_list; }
			constexpr u32 frame_index() const { return _frame_index; }

		private:
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
				void release() {
					core::release(cmd_allocator);
				}
			};

			ID3D12CommandQueue* _cmd_queue{ nullptr };
			ID3D12GraphicsCommandList6* _cmd_list{ nullptr };
			ID3D12Fence1* _fence{ nullptr };
			u64	_fence_value{ 0 };
			command_frame	_cmd_frames[frame_buffer_count]{};
			HANDLE _fence_event{ nullptr };
			u32 _frame_index{ 0 };
		};

		ID3D12Device8* main_device{ nullptr };	// 指向主设备的指针
		IDXGIFactory7* dxgi_factory{ nullptr };	// 工厂指针
		d3d12_command gfx_command;

		constexpr D3D_FEATURE_LEVEL minimum_feature_level{ D3D_FEATURE_LEVEL_11_0 };	//支持特性的最小版本


		/// <summary>
		/// static 创建失败了记得退出
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		bool failed_init() {
			shutdown();
			return false;
		}

		/// <summary>
		/// static 选择主适配器
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
		///	static 获取支持的最大特性
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



	} //anonymous namespace

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
			DXCall(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface)));
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

		new (&gfx_command) d3d12_command(main_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		if (!gfx_command.command_queue()) return failed_init();

		NAME_D3D12_OBJECT(main_device, L"MAIN DEVICE");

#ifdef _DEBUG
		{
			ComPtr<ID3D12InfoQueue> info_queue;
			DXCall(main_device->QueryInterface(IID_PPV_ARGS(&info_queue)));

			info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		}
#endif // _DEBUG

		return true;
	}

	/// <summary>
	/// D3D12的结束函数
	/// </summary>
	void shutdown()
	{
		gfx_command.release();
		release(dxgi_factory);

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

	void render() {
		// 等待gpu完成command allocator
		// 一旦完成就重置allocator
		// 这里释放被用来存储command的内存空间
		gfx_command.begin_frame();

		ID3D12GraphicsCommandList6* cmd_list{ gfx_command.command_list() };


		//记录操作
		//....
		//完成记录， 执行
		//通知下一帧并将fence_index加一
		gfx_command.end_frame();
	}
}