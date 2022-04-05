#pragma once

#define USE_STL_VECTOR 1
#define USE_STL_DEQUE 1

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
	void erase_unordered(utl::vector<T>& v, size_t index) {
		if (v.size() > 1) {
			std::iter_swap(v.begin() + index, v.end() - 1);
			v.pop_back();
		}
		else {
			v.clear();
		}
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

namespace primal::utl {
	// TODO: implement our own containers

}