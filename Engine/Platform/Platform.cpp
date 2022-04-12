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

			~window_info() {
				assert(!is_fullscreen);
			}
		};

		utl::free_list<window_info> windows;


		/// <summary>
		/// 通过传入window类的_id属性获取对应的window_info，方便我们进行修改
		/// </summary>
		/// <param name="id">The identifier.</param>
		/// <returns></returns>
		[[nodiscard]]
		window_info& get_from_id(window_id id) {
			assert(id < windows.size());
			assert(windows[id].hwnd);
			return(windows[id]);
		}

		[[nodiscard]]
		window_info& get_from_handle(window_handle handle) {
			const window_id id{ (window_id)GetWindowLongPtr(handle, GWLP_USERDATA) };
			return get_from_id(id);
		}

		/// <summary>
		/// 这里去处理消息，比方说拖动啦什么的，就靠这个回调来实现了
		/// </summary>
		/// <param name="hwnd">The HWND.</param>
		/// <param name="msg">The MSG.</param>
		/// <param name="wparam">The wparam.</param>
		/// <param name="lparam">The lparam.</param>
		/// <returns></returns>
		LRESULT CALLBACK internal_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

			window_info* info{ nullptr };
			switch (msg) {
			case WM_DESTROY:
				get_from_handle(hwnd).is_closed = true;
				break;
			case WM_EXITSIZEMOVE:
				info = &get_from_handle(hwnd);
				break;
			case WM_SIZE:
				if (wparam == SIZE_MAXIMIZED) {
					info = &get_from_handle(hwnd);
				}
				break;
			case WM_SYSCOMMAND:
				if (wparam == SC_RESTORE) {
					info = &get_from_handle(hwnd);
				}
				break;
			default:
				break;
			}

			if (info) {
				assert(info->hwnd);
				GetClientRect(info->hwnd, info->is_fullscreen ? &info->fullscreen_area : &info->client_area);

			}

			LONG_PTR long_ptr{ GetWindowLongPtr(hwnd,0) };
			return long_ptr
				? ((window_proc)long_ptr)(hwnd, msg, wparam, lparam)
				: DefWindowProc(hwnd, msg, wparam, lparam);
		}

		/// <summary>
		/// 调整window的窗口大小
		/// </summary>
		/// <param name="id">The identifier.</param>
		void resize_window(const window_info& info, const RECT& area) {
			RECT window_rect{ area };
			AdjustWindowRect(&window_rect, info.style, false);
			const s32 width{ window_rect.right - window_rect.left };
			const s32 height{ window_rect.bottom - window_rect.top };
			MoveWindow(info.hwnd, info.top_left.x, info.top_left.y, width, height, true);
		}
		/// <summary>
		/// 根据窗口宽高调整window的窗口大小
		/// </summary>
		/// <param name="id">The identifier.</param>
		void resize_window(window_id id, u32 width, u32 height) {
			window_info& info{ get_from_id(id) };
			if (info.style & WS_CHILD) {
				// 要是是host在editor中，那么就只更新client_area
				GetClientRect(info.hwnd, &info.client_area);
			}
			else {
				RECT& area{ info.is_fullscreen ? info.fullscreen_area : info.client_area };
				area.right = area.left + width;
				area.bottom = area.top + height;
				resize_window(info, area);
			}
		}


		/// <summary>
		/// 根据id切换窗口全屏或者普通视图
		/// </summary>
		/// <param name="id">The identifier.</param>
		/// <param name="is_fullscreen">if set to <c>true</c> [is fullscreen].</param>
		void set_window_full_screen(window_id id, bool is_fullscreen) {
			window_info& info{ get_from_id(id) };
			if (info.is_fullscreen != is_fullscreen) {
				info.is_fullscreen = is_fullscreen;

				if (is_fullscreen) {
					// 保存非全屏时的窗口大小和位置[切换]
					GetClientRect(info.hwnd, &info.client_area);
					RECT rect;
					GetWindowRect(info.hwnd, &rect);
					info.top_left.x = rect.left;
					info.top_left.y = rect.top;
					SetWindowLongPtr(info.hwnd, GWL_STYLE, 0);
					ShowWindow(info.hwnd, SW_MAXIMIZE);
				}
				else {
					info.style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
					SetWindowLongPtr(info.hwnd, GWL_STYLE, info.style);
					resize_window(info, info.client_area);
					ShowWindow(info.hwnd, SW_SHOWNORMAL);
				}
			}
		}


		/// <summary>
		/// 根据id查看是否正在全屏
		/// </summary>
		/// <param name="id">The identifier.</param>
		/// <returns>
		///   <c>true</c> if [is window full screen] [the specified identifier]; otherwise, <c>false</c>.
		/// </returns>
		[[nodiscard]]
		bool is_window_full_screen(window_id id) {
			return get_from_id(id).is_fullscreen;
		}

		/// <summary>
		/// 根据id返回hwnd
		/// </summary>
		/// <param name="id">The identifier.</param>
		/// <returns></returns>
		[[nodiscard]]
		window_handle get_window_handle(window_id id) {
			return get_from_id(id).hwnd;
		}

		/// <summary>
		/// 根据window_id设置标题
		/// </summary>
		/// <param name="id">The identifier.</param>
		/// <param name="caption">The caption.</param>
		void set_window_caption(window_id id, const wchar_t* caption) {
			window_info& info{ get_from_id(id) };
			SetWindowText(info.hwnd, caption);
		}

		/// <summary>
		/// 根据window_id获取左上右下
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		math::u32v4 get_window_size(window_id id) {
			window_info& info{ get_from_id(id) };
			RECT& area{ info.is_fullscreen ? info.fullscreen_area : info.client_area };
			return { (u32)area.left, (u32)area.top, (u32)area.right, (u32)area.bottom };
		}

		/// <summary>
		/// 根据window_id判断窗口是否关闭
		/// </summary>
		/// <param name="id">The identifier.</param>
		/// <returns>
		///   <c>true</c> if [is window closed] [the specified identifier]; otherwise, <c>false</c>.
		/// </returns>
		[[nodiscard]]
		bool is_window_closed(window_id id) {
			return get_from_id(id).is_closed;
		}

	}//匿名命名空间

	[[nodiscard]]
	window create_window(const window_init_info* const init_info  /* = nullptr */)
	{
		// 看用户传callback函数进来了嘛
		window_proc callback{ init_info ? init_info->callback : nullptr };
		// 如果是用编辑器，就有parent进程
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

		// 注册window class [注意这个函数可能会失败，因为可能会多次注册相同的class]
		RegisterClassExW(&wc);

		window_info info{};
		info.client_area.right = (init_info && init_info->width) ? init_info->left + init_info->width : info.client_area.right;
		info.client_area.bottom = (init_info && init_info->height) ? init_info->top + init_info->height : info.client_area.bottom;
		info.style |= parent ? WS_CHILD : WS_OVERLAPPEDWINDOW;

		RECT rect{ info.client_area };

		AdjustWindowRect(&rect, info.style, false);

		const wchar_t* caption{ (init_info && init_info->caption) ? init_info->caption : L"Primal Game" };
		const s32 left{ (init_info) ? init_info->left : info.top_left.x };
		const s32 top{ (init_info) ? init_info->top : info.top_left.y };
		const s32 width{ rect.right - rect.left };
		const s32 height{ rect.bottom - rect.top };


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
			DEBUG_OP(SetLastError(0));
			const window_id id{ windows.add(info) };
			// 将window_id保存到WindowLongPtr里面去作为用户数据（方便我们以后拿到hwnd时去数组找对应的window是哪一个）
			SetWindowLongPtr(info.hwnd, GWLP_USERDATA, (LONG_PTR)id);

			if (callback) {
				SetWindowLongPtr(info.hwnd, 0, (LONG_PTR)callback);
			}
			assert(GetLastError() == 0);
			ShowWindow(info.hwnd, SW_SHOWNORMAL);
			UpdateWindow(info.hwnd);
			return window{ id };
		}

		return {};
	}
	void remove_window(window_id id)
	{
		window_info info{ get_from_id(id) };
		DestroyWindow(info.hwnd);
		windows.remove(id);
}
#else
#error "must implement at least one platform"
#endif // _WIN64



	void window::set_fullscreen(bool is_fullscreen) const {
		assert(is_valid());
		set_window_full_screen(_id, is_fullscreen);
	}
	[[nodiscard]]
	bool window::is_fullscreen() const {
		assert(is_valid());
		return is_window_full_screen(_id);
	}

	void* window::handle() const {
		assert(is_valid());
		return get_window_handle(_id);
	}
	void window::set_caption(const wchar_t* caption) const {
		assert(is_valid());
		set_window_caption(_id, caption);
	}
	[[nodiscard]]
	math::u32v4 window::size() const {
		assert(is_valid());
		return get_window_size(_id);
	}
	void window::resize(u32 width, u32 height) const {
		assert(is_valid());
		resize_window(_id, width, height);
	}
	u32 window::width() const {
		assert(is_valid());
		math::u32v4 s{ size() };
		return s.z - s.x;
	}

	u32 window::height() const {
		assert(is_valid());
		math::u32v4 s{ size() };
		return s.w - s.y;
	}
	bool window::is_closed() const {
		assert(is_valid());
		return is_window_closed(_id);
	}


}// namespace primal::platform