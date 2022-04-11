#pragma once
#include "D3D12CommonHeaders.h"


namespace primal::graphics {
	struct platform_interface;	// 先声明

	namespace d3d12 {
		/// <summary>
		/// 获取底层的接口
		/// </summary>
		/// <param name="pi"></param>
		void get_platform_interface(platform_interface& pi);
	}
}