#pragma once
#include "CommonHeaders.h"
#include "..\Platform\Window.h"
namespace primal::graphics {

	class surface {

	};

	/// <summary>
	/// 结构体 渲染平面
	/// </summary>
	struct render_surface {
		platform::window window{};	// 窗口实例
		surface surface{};	// 平面实例
	};

	/// <summary>
	/// 枚举 图形平台
	/// </summary>
	enum class graphics_platform {
		direct3d12 = 0,
		vulkan = 1,
		open_gl = 2,
	};

	/// <summary>
	/// 初始化图形平台函数
	/// </summary>
	/// <param name="platform"></param>
	/// <returns></returns>
	[[nodiscard]]
	bool initialize(graphics_platform platform);
	
	/// <summary>
	/// 退出图形平台函数
	/// </summary>
	void shutdown();
}