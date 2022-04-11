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
	/// ģ���� �ж�null����Դ�ͷ�
	/// </summary>
	template <typename T>
	constexpr void release(T*& resource) {
		if (resource) {
			resource->Release();
			resource = nullptr;
		}
	}
}