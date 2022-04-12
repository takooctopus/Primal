#pragma once

#include "CommonHeaders.h"

namespace primal::utl {
	template<typename T>
	class free_list {
		static_assert(sizeof(T) >= sizeof(u32));
	public:
		free_list() = default;
		explicit free_list(u32 count) {
			_array.reserve(count);
		}
		~free_list() { 
			assert(!_size); 
		}

		template <class... params>
		constexpr u32 add(params&&... p) {
			u32 id{ u32_invalid_id };
			if (_next_free_index == u32_invalid_id) {
				// 要是没有多余的空位，就加一个【这里用了完美转发】
				id = (u32)_array.size();
				_array.emplace_back(std::forward<params>(p)...);
			}
			else {
				id = _next_free_index;
				assert(id < _array.size() && already_removed(id));
				_next_free_index = *(const u32* const)std::addressof(_array[id]);	// 先取_array对应idx的地址， 将其转成一个u32【4字节】的常指针，取得前4个字节，就是我们下一个空闲节点
				new (std::addressof(_array[id])) T(std::forward<params>(p)...);	// 在_array[id]这个地址上使用我们的T参数创建一个实例
			}
			++_size;
			return id;
		}

		constexpr void remove(u32 id) {
			assert(id < _array.size() && !already_removed(id));
			T& item{ _array[id] };
			item.~T();
			DEBUG_OP(memset(std::addressof(_array[id]), 0xcc, sizeof(T)));
			*(u32* const)std::addressof(_array[id]) = _next_free_index;	//将当前数组位置的前4个字节设置成下一个序号【做成一个链表】
			_next_free_index = id; 
			--_size;
		}

		[[nodiscard]]
		constexpr u32 size() const {
			return _size;
		}

		[[nodiscard]]
		constexpr u32 capacity() const {
			return _array.size();
		}

		[[nodiscard]]
		constexpr bool empty() const {
			return _size == 0;
		}

		[[nodiscard]]
		constexpr T& operator[](u32 id) {
			assert(id < _array.size() && !already_removed(id));
			return _array[id];
		}

		[[nodiscard]]
		constexpr const T& operator[](u32 id) const {
			assert(id < _array.size() && !already_removed(id));
			return _array[id];
		}

	private:
		constexpr bool already_removed(u32 id) {
			if constexpr (sizeof(T) > sizeof(u32)) {
				u32 i{ sizeof(u32) };	//跳过开头四个字节
				const u8* const p{ (const u8* const)std::addressof(_array[id]) };
				while ((p[i] == 0xcc) && (i < sizeof(T)))++i;
				return i == sizeof(T);
			}
			else {
				return true;
			}
		}

		utl::vector<T>		_array;
		u32					_next_free_index{ u32_invalid_id };
		u32					_size{0};
	};
}