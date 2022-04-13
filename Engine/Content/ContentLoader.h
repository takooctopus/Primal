#pragma once
#include "CommonHeaders.h"

#if !defined(SHIPPING)

namespace primal::content {

	[[nodiscard]]
	bool load_game();

	void unload_game();

	[[nodiscard]]
	bool load_engine_shaders(std::unique_ptr<u8[]>& shaders, u64& size);
}//namespace primal::content

#endif // !defined(SHIPPING)