#include "D3D12Surface.h"
#include "D3D12Core.h"

namespace primal::graphics::d3d12 {
	namespace {

		constexpr DXGI_FORMAT to_non_srgb(DXGI_FORMAT format) {
			if (format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) return DXGI_FORMAT_R8G8B8A8_UNORM;
			return format;
		}

	}// anonymous namespace

	void d3d12_surface::create_swap_chain(IDXGIFactory7* factory, ID3D12CommandQueue* cmd_queue, DXGI_FORMAT format /*= default_back_buffer_format*/)
	{
		assert(factory && cmd_queue);
		release();

		if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &_allow_tearing, sizeof(u32))) && _allow_tearing) {
			_present_flags = DXGI_PRESENT_ALLOW_TEARING;
		}
		_allow_tearing = _present_flags = 0;

		_format = format;

		DXGI_SWAP_CHAIN_DESC1 desc{};	//交换链描述信息
		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.BufferCount = buffer_count;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.Flags = _allow_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		desc.Format = to_non_srgb(format);
		desc.Height = _window.height();
		desc.Width = _window.width();
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Stereo = false;
		IDXGISwapChain1* swap_chain;
		HWND hwnd{ (HWND)_window.handle() };


		DXCall(factory->CreateSwapChainForHwnd(cmd_queue, hwnd, &desc, nullptr, nullptr, &swap_chain));
		DXCall(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));	// 禁止alt+enter切换全屏
		DXCall(swap_chain->QueryInterface(IID_PPV_ARGS(&_swap_chain)));	//将swap_chain转换到类内设定的IDXGISwapChain4
		core::release(swap_chain);

		_current_bb_index = _swap_chain->GetCurrentBackBufferIndex();

		for (u32 i{ 0 }; i < buffer_count; ++i) {
			_render_target_data[i].rtv = core::rtv_heap().allocate();	// 每一帧都得分配一个描述符堆
		}
		finalize();
	}

	void d3d12_surface::present() const
	{
		assert(_swap_chain);
		DXCall(_swap_chain->Present(0, _present_flags));
		_current_bb_index = _swap_chain->GetCurrentBackBufferIndex();
	}

	void d3d12_surface::resize(u32 width, u32 height)
	{
		assert(_swap_chain);
		for (u32 i{ 0 }; i < buffer_count; ++i) {
			core::release(_render_target_data[i].resource); 
		}

		const u32 flags{ _allow_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0ul };
		DXCall(_swap_chain->ResizeBuffers(buffer_count, 0, 0, DXGI_FORMAT_UNKNOWN, flags));
		_current_bb_index = _swap_chain->GetCurrentBackBufferIndex();
		finalize();

		DEBUG_OP(OutputDebugString(L"::D3D12 Surface Resized.\n"));
	}

	void d3d12_surface::finalize() {
		// 为每个back-buffers创建rtvs 【渲染目标缓冲区描述符】
		for (u32 i{ 0 }; i < buffer_count; ++i) {
			render_target_data& data{ _render_target_data[i] };
			assert(!data.resource);
			DXCall(_swap_chain->GetBuffer(i, IID_PPV_ARGS(&data.resource)));	//为交换链每部分创建buffer
			D3D12_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = _format;
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			core::device()->CreateRenderTargetView(data.resource, &desc, data.rtv.cpu);
		}

		DXGI_SWAP_CHAIN_DESC desc{};
		DXCall(_swap_chain->GetDesc(&desc));
		const u32 width{ desc.BufferDesc.Width };
		const u32 height{ desc.BufferDesc.Height };
		assert(_window.width() == width && _window.height() == height);

		// 设定 viewport和scissor rect
		_viewport.TopLeftX = 0.f;
		_viewport.TopLeftY = 0.f;
		_viewport.Width = (float)width;
		_viewport.Height = (float)height;
		_viewport.MinDepth = 0.f;
		_viewport.MaxDepth = 1.f;

		_scissor_rect = { 0,0,(s32)width, (s32)height };
	}

	void d3d12_surface::release()
	{
		for (u32 i{ 0 }; i < buffer_count; ++i) {
			render_target_data& data{ _render_target_data[i] };
			core::release(data.resource);	// 释放资源
			core::rtv_heap().free(data.rtv);	//从总的描述符堆中释放这个描述符
		}
		core::release(_swap_chain);
	}


}