#pragma once
#include "CommonHeaders.h"
#include "Renderer.h"
#include "..\Platform\Window.h"

namespace primal::graphics {

	/// <summary>
	/// 底层图形平台接口
	/// </summary>
	struct platform_interface
	{
		bool (*initialize)(void);	//函数指针 初始化
		void (*shutdown)(void);		//函数指针 结束

		struct 
		{
			surface(*create)(platform::window);
			void (*remove)(surface_id);
			void (*resize)(surface_id, u32, u32);
			u32 (*width)(surface_id);
			u32 (*height)(surface_id);
			void (*render)(surface_id);
		} surface;

		graphics_platform platform = (graphics_platform)-1;
	};
}

