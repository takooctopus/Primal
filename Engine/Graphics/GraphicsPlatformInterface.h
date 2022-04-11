#pragma once
#include "CommonHeaders.h"
#include "Renderer.h"

namespace primal::graphics {

	/// <summary>
	/// 底层图形平台接口
	/// </summary>
	struct platform_interface
	{
		bool (*initialize)(void);	//函数指针 初始化
		void (*shutdown)(void);		//函数指针 结束
	};
}

