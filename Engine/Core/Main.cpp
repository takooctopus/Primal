#include "CommonHeaders.h"
#include <filesystem>

#ifdef _WIN64

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <crtdbg.h>

namespace {
	std::filesystem::path set_current_directory_to_executable_path() {
		//设定我们的工作目录到当前程序执行位置
		wchar_t path[MAX_PATH]{};
		// 获取exe现在的执行目录[${ProjectDir}\${projectName}\x64\Debug\${projectName.exe}]
		const u32 length{ GetModuleFileName(0, &path[0], MAX_PATH) };
		// 要是长度超了，毁灭吧
		if (!length || GetLastError() == ERROR_INSUFFICIENT_BUFFER) return {};
		std::filesystem::path p{ path };
		// 将工作目录设置到Debug[或者Release]那一层
		std::filesystem::current_path(p.parent_path());
		return std::filesystem::current_path();
	}
}//anonymous namespace


#ifndef USE_WITH_EDITOR

[[nodiscard]]
extern bool	engine_initialize();
extern void	engine_update();
extern void	engine_shutdown();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

#ifdef DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)
#endif // DEBUG

		set_current_directory_to_executable_path();
		if (engine_initialize()) {
			MSG msg{};
			bool is_running{ true };
			while (is_running) {
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					is_running &= (msg.message != WM_QUIT);
				}
				engine_update();
			}
		}
	engine_shutdown();
	return 0;
}

#endif // !USE_WITH_EDITOR
#endif // _WIN64
