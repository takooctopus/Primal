#pragma once


#define USE_STL_VECTOR 0
#define USE_STL_DEQUE 1
#define USE_STL_UNORDERED_MAP 1

#if USE_STL_VECTOR
#include <vector>
namespace primal::utl {
	template<typename T>
	using vector = std::vector<T>;

	/// <summary>
	/// 删除数组中的元素，将末尾元素交换到删除位置【先交换，再删除末尾元素】
	/// </summary>
	/// <param name="v">The v.</param>
	/// <param name="index">The index.</param>
	template <typename T>
	void erase_unordered(T& v, size_t index) {
		if (v.size() > 1) {
			std::iter_swap(v.begin() + index, v.end() - 1);
			v.pop_back();
		}
		else {
			v.clear();
		}
	}
}
#else
#include "Vector.h"
namespace primal::utl {
	template <typename T>
	void erase_unordered(T& v, size_t index) {
		v.erase_unordered(index);
	}
}
#endif

#if USE_STL_DEQUE
#include <deque>
namespace primal::utl {
	template<typename T>
	using deque = std::deque<T>;
}
#endif

#if USE_STL_UNORDERED_MAP
#include <unordered_map>
namespace primal::utl {
	template<typename T1, typename T2>
	using unordered_map = std::unordered_map<T1, T2>;
}
#endif

namespace primal::utl {
	// TODO: implement our own containers

}

#include "..\Utilities\FreeList.h"
