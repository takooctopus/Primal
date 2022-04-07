#include "Platform.h"
#include "PlatformTypes.h"

namespace primal::platform {

#ifdef _WIN64
	namespace {
		struct window_info {
			HWND			hwnd{ nullptr };
			RECT			client_area{ 0,0,1920,1080 };
			RECT			fullscreen_area{};
			POINT			top_left{ 0,0 };
			DWORD			style{ WS_VISIBLE };
			bool			is_fullscreen{ false };
			bool			is_closed{ false };
		};

		LRESULT CALLBACK internal_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
			LONG_PTR long_ptr{ GetWindowLongPtr(hwnd,0) };
			return long_ptr
				? ((window_proc)long_ptr)(hwnd, msg, wparam, lparam)
				: DefWindowProc(hwnd, msg, wparam, lparam);
		}

	}//匿名命名空间

	bool create_window(const window_init_info* const init_info  /* = nullptr */)
	{
		window_proc callback{ init_info ? init_info->callback : nullptr };
		window_handle parent{ init_info ? init_info->parent : nullptr };

		WNDCLASSEX wc;
		ZeroMemory(&wc, sizeof(wc));

		// 设定window class
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = internal_window_proc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = callback ? sizeof(callback) : 0;
		wc.hInstance = 0;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = CreateSolidBrush(RGB(26, 48, 76));
		wc.lpszMenuName = NULL;
		wc.lpszClassName = L"PrimalWindow";
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		// 注册window class
		RegisterClassExW(&wc);

		window_info info{};
		RECT rc{ info.client_area };

		AdjustWindowRect(&rc, info.style, false);

		const wchar_t* caption{ (init_info && init_info->caption) ? init_info->caption : L"Primal Game" };
		const s32 left{ (init_info && init_info->left) ? init_info->left : info.client_area.left };
		const s32 top{ (init_info && init_info->top) ? init_info->top : info.client_area.top };
		const s32 width{ (init_info && init_info->width) ? init_info->width : rc.right - rc.left };
		const s32 height{ (init_info && init_info->height) ? init_info->height : rc.bottom - rc.top };

		info.style |= parent ? WS_CHILD : WS_OVERLAPPEDWINDOW;

		// 创建窗口实例
		info.hwnd = CreateWindowEx(
			0,					 /* _In_ DWORD dwExStyle,			*/
			wc.lpszClassName,	 /* _In_opt_ LPCWSTR lpClassName,	*/
			caption,			 /* _In_opt_ LPCWSTR lpWindowName,	*/
			info.style,			 /* _In_ DWORD dwStyle,				*/
			left,				 /* _In_ int X,						*/
			top,				 /* _In_ int Y,						*/
			width,				 /* _In_ int nWidth,				*/
			height,				 /* _In_ int nHeight,				*/
			parent,				 /* _In_opt_ HWND hWndParent,		*/
			NULL,				 /* _In_opt_ HMENU hMenu,			*/
			NULL,				 /* _In_opt_ HINSTANCE hInstance,	*/
			NULL				 /* _In_opt_ LPVOID lpParam);		*/
		);

		if (info.hwnd) {
			// TODO: 保存window到数组中
		}
	}
#elif
#error "must implement at least one platform"
#endif // _WIN64

}