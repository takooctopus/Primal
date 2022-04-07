#pragma once
#include "CommonHeaders.h"

#if !defined(SHIPPING)

namespace primal::content {

	[[nodiscard]]
	bool load_game();

	void unload_game();
}//namespace primal::content

#endif // !defined(SHIPPING)