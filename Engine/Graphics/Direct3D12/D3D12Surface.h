#pragma once
#include "D3D12CommonHeaders.h"

namespace primal::graphics::d3d12 {
	class d3d12_surface {
	public:
		constexpr static u32 buffer_count{ 3 };
		constexpr static DXGI_FORMAT default_back_buffer_format{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
		explicit d3d12_surface(platform::window window) : _window(window) {
			assert(_window.handle());
		}
#if USE_STL_VECTOR
		DISABLE_COPY(d3d12_surface);
		constexpr d3d12_surface(d3d12_surface&& o) :
			_swap_chain{ o._swap_chain }, _window{ o._window }, _current_bb_index{ o._current_bb_index },
			_viewport{ o._viewport }, _scissor_rect{ o._scissor_rect }, _allow_tearing{ o._allow_tearing }, _present_flags{ o._present_flags }{
			for (u32 i{ 0 }; i < frame_buffer_count; ++i) {
				_render_target_data[i].resource = o._render_target_data[i].resource;
				_render_target_data[i].rtv = o._render_target_data[i].rtv;
			}
			o.reset();
		}

		constexpr d3d12_surface& operator=(d3d12_surface&& o) {
			assert(this != std::addressof(o)); //std::addressof(o) ����&o
			if (this != std::addressof(o)) {
				release();
				move(o);
			}
			return *this;
		}
#else
		DISABLE_COPY_AND_MOVE(d3d12_surface);
#endif // USE_STL_VECTOR


		~d3d12_surface() { release(); }

		void create_swap_chain(IDXGIFactory7* factory, ID3D12CommandQueue* cmd_queue, DXGI_FORMAT format = default_back_buffer_format);
		void present() const;
		void resize(u32 width, u32 height);

		constexpr u32 width() const { return (u32)_viewport.Width; }	//�����ӽǿ��
		constexpr u32 height() const { return (u32)_viewport.Height; }	//�����ӽǸ߶�
		constexpr ID3D12Resource* const back_buffer() const { return _render_target_data[_current_bb_index].resource; }	//������Ⱦ��Դ
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE rtv() const { return _render_target_data[_current_bb_index].rtv.cpu; }	//������ȾĿ��������
		constexpr const D3D12_VIEWPORT& viewport() const { return _viewport; }	//�����ӽ�
		constexpr const D3D12_RECT& scissor_rect() const { return _scissor_rect; }	//������ͼ��
	private:
		void finalize();
		void release();

#if USE_STL_VECTOR
		constexpr void move(d3d12_surface& o) {
			_swap_chain = o._swap_chain;
			for (u32 i{ 0 }; i < frame_buffer_count; ++i) {
				_render_target_data[i] = o._render_target_data[i];
			}
			_window = o._window;
			_current_bb_index = o._current_bb_index;
			_allow_tearing = o._allow_tearing;
			_present_flags = o._present_flags;
			_viewport = o._viewport;
			_scissor_rect = o._scissor_rect;

			o.reset();
		}

		constexpr void reset() {
			_swap_chain = nullptr;
			for (u32 i{ 0 }; i < buffer_count; ++i) {
				_render_target_data[i] = {};
			}
			_window = {};
			_current_bb_index = 0;
			_allow_tearing = 0;
			_present_flags = 0;
			_viewport = {};
			_scissor_rect = {};
		}
#endif // USE_STL_VECTOR


		struct render_target_data {
			ID3D12Resource* resource{ nullptr };	//��Դָ��
			descriptor_handle rtv{};	//�����������Ƿŵ�core::rtv_desc_heap�е���������
		};	//��ȾĿ������

		IDXGISwapChain4* _swap_chain{ nullptr };	//������ָ��
		render_target_data		_render_target_data[buffer_count]{};	// ��ȾĿ�껺����������rtv����
		platform::window		_window{};		//����ָ��
		DXGI_FORMAT				_format{ default_back_buffer_format };
		mutable	u32				_current_bb_index{ 0 };	//��ǰ�ط���������š�mutable��Ϊpresent()�����趨����const������
		u32						_allow_tearing{ 0 };	//�������
		u32						_present_flags{ 0 };	//��ʾ��־
		D3D12_VIEWPORT			_viewport{};	//�ӵ�
		D3D12_RECT				_scissor_rect{};	//��ͼ��
	};
}