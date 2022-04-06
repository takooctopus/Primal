
#include "Common.h"

#ifndef WIN_MEAN_AND_LEAN
#define WIN_MEAN_AND_LEAN
#endif // WIN_MEAN_AND_LEAN
#include <Windows.h>

using namespace primal;

namespace {
	HMODULE game_code_dll{ nullptr };
}

EDITOR_INTERFACE
u32 LoadGameCodeDll(const char* dll_path) {
	if (game_code_dll) return false;
	game_code_dll = LoadLibraryA(dll_path);
	assert(game_code_dll);
	return game_code_dll ? true : false;
}

EDITOR_INTERFACE
u32 UnloadGameCodeDll() {
	if (!game_code_dll) return false;
	assert(game_code_dll);
	int result{ FreeLibrary(game_code_dll) };
	assert(result);
	game_code_dll = nullptr;
	return true;
}