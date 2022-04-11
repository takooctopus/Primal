#pragma once
#include "D3D12CommonHeaders.h"
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
	/// 核心渲染函数
	/// </summary>
	void render();


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
	ID3D12Device* const device();

	/// <summary>
	/// 当前frame的index
	/// </summary>
	[[nodiscard]]
	u32 current_frame_index();

	/// <summary>
	/// 设定当前frame被推迟
	/// </summary>
	void set_deferred_releases_flag();
	
}