#pragma once
#include "CommonHeaders.h"

#ifdef _WIN64
#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif // !WIN32_MEAN_AND_LEAN
#include <Windows.h>

namespace primal::platform {
	using window_proc = LRESULT(*)(HWND, UINT, WPARAM, LPARAM);
	using window_handle = HWND;

	/// <summary>
	/// 窗口初始化信息结构体
	/// </summary>
	struct window_init_info
	{
		window_proc			callback{ nullptr };
		window_handle		parent{ nullptr };
		const wchar_t*		caption{ nullptr };
		s32					left{ 0 };
		s32					top{ 0 };
		s32					width{ 1920 };
		s32					height{ 1080 };
	};
}
#endif // _WIN64
