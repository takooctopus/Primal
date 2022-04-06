
#include "Common.h"
#include "..\Engine\Components\Script.h"

#ifndef WIN_MEAN_AND_LEAN
#define WIN_MEAN_AND_LEAN
#endif // WIN_MEAN_AND_LEAN
#include <Windows.h>

using namespace primal;

namespace {

	/// <summary>
	/// The game code DLL �б���Ȼ���Ͳ����б���Ҳ���԰������һ���б�
	/// </summary>
	HMODULE game_code_dll{ nullptr };

	using _get_script_creator = primal::script::detail::script_creator(*)(size_t);
	/// <summary>
	/// ��ȡ�ű�����creator�ĺ���ָ��
	/// </summary>
	_get_script_creator get_script_creator{ nullptr };

	using _get_script_names = LPSAFEARRAY(*)(void);
	/// <summary>
	/// ��ȡ�ű���������ĺ���ָ��
	/// </summary>
	_get_script_names get_script_names{ nullptr };

} //����namespace


/// <summary>
/// Loads the game code DLL.
/// </summary>
/// <param name="dll_path">The DLL path.</param>
/// <returns></returns>
EDITOR_INTERFACE
u32 LoadGameCodeDll(const char* dll_path) {
	if (game_code_dll) return false;
	game_code_dll = LoadLibraryA(dll_path);
	assert(game_code_dll);

	// ͨ��GetProcAddress��ȡ����game_code_dll��������еĺ���ָ�뼯�ϣ�������ת���ɶ�Ӧ�������еĺ���ָ��
	get_script_names = (_get_script_names)GetProcAddress(game_code_dll, "get_script_names");
	get_script_creator = (_get_script_creator)GetProcAddress(game_code_dll, "get_script_creator");

	return (game_code_dll && get_script_names && get_script_creator) ? true : false;
}

/// <summary>
/// Unloads the game code DLL.
/// </summary>
/// <returns></returns>
EDITOR_INTERFACE
u32 UnloadGameCodeDll() {
	if (!game_code_dll) return false;
	assert(game_code_dll);
	int result{ FreeLibrary(game_code_dll) };
	assert(result);
	game_code_dll = nullptr;
	return true;
}

EDITOR_INTERFACE
script::detail::script_creator GetScriptCreator(const char* name) {
	return (game_code_dll && get_script_creator) ? get_script_creator(script::detail::string_hash()(name)) : nullptr;
}

EDITOR_INTERFACE
LPSAFEARRAY GetScriptNames() {
	return (game_code_dll && get_script_names) ? get_script_names() : nullptr;
}