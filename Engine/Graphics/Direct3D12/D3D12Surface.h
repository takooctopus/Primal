#pragma once
#include "D3D12CommonHeaders.h"
#include "D3D12Resources.h"

namespace primal::graphics::d3d12 {
	class d3d12_surface {
	public:
		explicit d3d12_surface(platform::window window) : _window(window) {
			assert(_window.handle());
		}

		~d3d12_surface() { release(); }

		void create_swap_chain(IDXGIFactory7* factory, ID3D12CommandQueue* cmd_queue, DXGI_FORMAT format);
		void present() const;
		void resize();

		constexpr u32 width() const { return (u32)_viewport.Width; }	//返回视角宽度
		constexpr u32 height() const { return (u32)_viewport.Height; }	//返回视角高度
		constexpr ID3D12Resource* const back_buffer() const { return _render_target_data[_current_bb_index].resource; }	//返回渲染资源
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE rtv() const { return _render_target_data[_current_bb_index].rtv.cpu; }	//返回渲染目标描述符
		constexpr const D3D12_VIEWPORT& viewport() const { return _viewport; }	//返回视角
		constexpr const D3D12_RECT& scissor_rect() const { return _scissor_rect; }	//返回视图框
	private:
		void finalize();
		void release();

		struct render_target_data {
			ID3D12Resource* resource{ nullptr };	//资源指针
			descriptor_handle rtv{};	//描述符【就是放到core::rtv_desc_heap中的描述符】
		};	//渲染目标数据

		IDXGISwapChain4* _swap_chain{ nullptr };	//交换链指针
		render_target_data		_render_target_data[frame_buffer_count]{};	// 渲染目标缓冲区描述符rtv数组
		platform::window		_window{};		//窗口指针
		mutable	u32				_current_bb_index{ 0 };	//当前回返缓冲区序号【mutable因为present()我们设定成了const函数】
		D3D12_VIEWPORT			_viewport{};	//视点
		D3D12_RECT				_scissor_rect{};	//视图框
	};
}