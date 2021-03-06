#pragma comment(lib, "engine.lib")
#include "Test.h"

#if TEST_ENTITY_COMPONENTS
#include "TestEntityComponents.h"
#elif TEST_WINDOW
#include "TestWindow.h"
#elif TEST_RENDERER
#include "TestRenderer.h"
#else
#error One of the tests need to be enabled
#endif // TEST_ENTITY_COMPONENTS




#ifdef _WIN64
#include <Windows.h>
#include <filesystem>

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

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG

	set_current_directory_to_executable_path();
	engine_test test {};
	if (test.initialize()) {
		MSG msg{};
		bool is_running{ true };
		while (is_running) {
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				is_running &= (msg.message != WM_QUIT);
			}
			test.run();
		}
	}
	test.shutdown();
	return 0;
}
#else
int main() {
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	engine_test test{};
	if (test.initialize()) {
		test.run();
	}
	test.shutdown();
}
#endif // _WIN64

