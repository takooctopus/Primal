#include "D3D12Core.h"
#include "D3D12CommonHeaders.h"

using namespace Microsoft::WRL;

namespace primal::graphics::d3d12::core {
	namespace {
		ID3D12Device8* main_device{ nullptr };	// 指向主设备的指针
		IDXGIFactory7* dxgi_factory{ nullptr };	// 工厂指针
		constexpr D3D_FEATURE_LEVEL minimum_feature_level{D3D_FEATURE_LEVEL_11_0};	//支持特性的最小版本

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
				D3D12_RLDO_SUMMARY	|	D3D12_RLDO_DETAIL	|	D3D12_RLDO_IGNORE_INTERNAL
			));
		}
#endif // _DEBUG

		release(main_device);
	}

}