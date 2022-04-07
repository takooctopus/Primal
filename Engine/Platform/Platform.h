#pragma once
#include "CommonHeaders.h"
#include "Window.h"

namespace primal::platform {

	struct window_init_info;

	
	/// <summary>
	/// 创建窗口【反正里面不仅创建还要加进列表里】
	/// </summary>
	/// <param name="init_info"></param>
	/// <returns></returns>
	[[nodiscard]]
	window create_window(const window_init_info* const init_info = nullptr);

	/// <summary>
	/// 摧毁window窗口
	/// </summary>
	/// <param name="id"></param>
	void remove_window(window_id id);


}//namespace primal::platform