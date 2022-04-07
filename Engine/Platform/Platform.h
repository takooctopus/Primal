#pragma once
#include "CommonHeaders.h"
#include "Window.h"

namespace primal::platform {

	struct window_init_info;

	
	/// <summary>
	/// �������ڡ��������治��������Ҫ�ӽ��б��
	/// </summary>
	/// <param name="init_info"></param>
	/// <returns></returns>
	[[nodiscard]]
	window create_window(const window_init_info* const init_info = nullptr);

	/// <summary>
	/// �ݻ�window����
	/// </summary>
	/// <param name="id"></param>
	void remove_window(window_id id);


}//namespace primal::platform