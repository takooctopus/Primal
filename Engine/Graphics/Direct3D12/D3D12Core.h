#pragma once
#include "D3D12CommonHeaders.h"

namespace primal::graphics::d3d12 {
	class descriptor_heap;
}

namespace primal::graphics::d3d12::core {

	/// <summary>
	/// 核心初始化函数
	/// </summary>
	[[nodiscard]]
	bool initialize();

	/// <summary>
	/// 核心关闭函数
	/// </summary>
	void shutdown();

	/// <summary>
	/// 模板类 判断null的资源释放
	/// </summary>
	template <typename T>
	constexpr void release(T*& resource) {
		if (resource) {
			resource->Release();
			resource = nullptr;
		}
	}

	namespace detail {
		/// <summary>
		/// 推迟资源释放【相当于加入队列】
		/// </summary>
		void deferred_release(IUnknown* resource);
	}

	/// <summary>
	/// 模板类 推迟资源释放【调用更底层的函数】
	/// </summary>
	template <typename T>
	constexpr void deferred_release(T*& resource) {
		if (resource) {
			detail::deferred_release(resource);
			resource = nullptr;
		}
	}


	/// <summary>
	/// 返回device
	/// </summary>
	[[nodiscard]]
	ID3D12Device8* const device();
	
	[[nodiscard]]
	descriptor_heap& rtv_heap();

	[[nodiscard]]
	descriptor_heap& dsv_heap();

	[[nodiscard]]
	descriptor_heap& srv_heap();

	[[nodiscard]]
	descriptor_heap& uav_heap();

	[[nodiscard]]
	DXGI_FORMAT	default_render_target_format();

	/// <summary>
	/// 当前frame的index
	/// </summary>
	[[nodiscard]]
	u32 current_frame_index();

	/// <summary>
	/// 设定当前frame被推迟
	/// </summary>
	void set_deferred_releases_flag();

	surface create_surface(platform::window window);
	void remove_surface(surface_id id);
	void resize_surface(surface_id id, u32 width, u32 height);
	u32 surface_width(surface_id id);
	u32 surface_height(surface_id id);
	void render_surface(surface_id id);
	
}