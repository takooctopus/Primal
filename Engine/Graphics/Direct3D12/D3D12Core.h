#pragma once
#include "D3D12CommonHeaders.h"
namespace primal::graphics::d3d12::core {

	/// <summary>
	/// 
	/// </summary>
	[[nodiscard]]
	bool initialize();
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
}