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
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG
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

