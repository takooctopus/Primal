#pragma once
#include "D3D12CommonHeaders.h"


namespace primal::graphics {
	struct platform_interface;	// ������

	namespace d3d12 {
		/// <summary>
		/// ��ȡ�ײ�Ľӿ�
		/// </summary>
		/// <param name="pi"></param>
		void get_platform_interface(platform_interface& pi);
	}
}