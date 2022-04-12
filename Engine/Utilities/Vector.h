#pragma once
#include "CommonHeaders.h"

namespace primal::utl {
	template<typename T, bool destruct = true>
	class vector {
	public:
		vector() = default;	//默认构造函数，不分配内存
		constexpr explicit vector(u64 count) {
			resize(count);
		}
		constexpr explicit vector(u64 count, const T& value) {
			resize(count, value);
		}

		template <typename it, typename = std::enable_if_t<std::_Is_iterator_v<it>>>
		constexpr explicit vector(it first, it last) {
			for (; first != last; ++first) {
				emplace_back(*first);
			}
		}

		constexpr vector(const vector& o) {
			*this = o;
		}
		constexpr vector(const vector&& o)
			: _capacity(o._capacity), _size(o._size), _data{ o._data }{
			o.reset();
		}

		/// <summary>
		/// 右侧是左值的=操作
		/// </summary>
		/// <param name="o"></param>
		/// <returns></returns>
		constexpr vector& operator=(const vector& o) {
			assert(this != std::addressof(o));
			if (this != std::addressof(o)) {
				clear();
				reserve(o._size);
				for (auto&& item : o) {
					emplace_back(item);
				}
				assert(_size == o._size);
			}
			return *this;
		}

		/// <summary>
		/// 右侧是右值的=操作
		/// </summary>
		/// <param name="o"></param>
		/// <returns></returns>
		constexpr vector& operator=(vector&& o) {
			assert(this != std::addressof(o));
			if (this != std::addressof(o)) {
				destroy();
				move(o);
			}
			return *this;
		}

		~vector() {
			destroy();
		}


		/// <summary>
		/// 左值插入
		/// </summary>
		/// <param name="value"></param>
		constexpr void push_back(const T& value) {
			emplace_back(value);
		}

		/// <summary>
		/// 右值插入【要用转发】
		/// </summary>
		/// <param name="value"></param>
		constexpr void push_back(const T&& value) {
			emplace_back(std::move(value));
		}

		/// <summary>
		/// 拷贝或者移动构造都行
		/// </summary>
		/// <typeparam name="...params"></typeparam>
		/// <param name="...p"></param>
		/// <returns></returns>
		template<class... params>
		constexpr decltype(auto) emplace_back(params&&... p) {
			if (_size == _capacity) {
				reserve((_capacity + 1) * 3 >> 1);	// reserve分配1.5倍大小的空间
			}
			assert(_size < _capacity);

			T* const item{ new (std::addressof(_data[_size])) T(std::forward<params>(p)...) };
			++_size;
			return *item;
		}

		/// <summary>
		/// 重新分配空间，并在其中初始化一个默认实例
		/// </summary>
		/// <param name="new_size"></param>
		constexpr void resize(u64 new_size) {
			static_assert(std::is_default_constructible_v<T>, "Type must be default constructible.");	//就像std::vector一样，resize是要实际分配空间的
			if (new_size > _size) {
				reserve(new_size);
				while (_size < new_size) {
					emplace_back();
				}
			}
			else if (new_size < _size) {
				if constexpr (destruct) {
					destruct_range(new_size, _size);
				}
				_size = new_size;
			}
			assert(new_size == _size);
		}

		/// <summary>
		/// 重新分配空间，并在其中初始化一个value实例
		/// </summary>
		/// <param name="new_size"></param>
		constexpr void resize(u64 new_size, const T& value) {
			static_assert(std::is_copy_constructible_v<T>, "Type must be copy constructible.");	//就像std::vector一样，resize是要实际分配空间的
			if (new_size > _size) {
				reserve(new_size);
				while (_size < new_size) {
					emplace_back(value);
				}
			}
			else if (new_size < _size) {
				if constexpr (destruct) {
					destruct_range(new_size, _size);
				}
				_size = new_size;
			}
			assert(new_size == _size);
		}

		/// <summary>
		/// 重新分配内存
		/// </summary>
		/// <param name="new_capacity"></param>
		constexpr void reserve(u64 new_capacity) {
			if (new_capacity > _capacity) {
				void* new_buffer{ realloc(_data, new_capacity * sizeof(T)) };
				assert(new_buffer);
				if (new_buffer) {
					_data = static_cast<T*>(new_buffer);
					_capacity = new_capacity;
				}
			}
		}

		/// <summary>
		/// 删除一个item并返回下一个指针
		/// </summary>
		/// <param name="index"></param>
		/// <returns></returns>
		constexpr T* const erase(u64 index) {
			assert(_data && index < _size);
			return erase(std::addressof(_data[index]));
		}

		/// <summary>
		/// 根据item地址删除item返回当前地址【因为删除后就要往前走一格】
		/// </summary>
		/// <param name="item"></param>
		/// <returns></returns>
		constexpr T* const erase(T* const item) {
			assert(_data && item >= std::addressof(_data[0]) && item < std::addressof(_data[_size]));
			if constexpr (destruct) item->~T();
			--_size;
			if (item < std::addressof(_data[_size])) {
				memcpy(item, item + 1, (std::addressof(_data[_size]) - item) * sizeof(T));
			}
			return item;
		}

		/// <summary>
		/// 乱序删除item
		/// </summary>
		/// <param name="index"></param>
		/// <returns></returns>
		constexpr T* const erase_unordered(u64 index) {
			assert(_data && index < _size);
			return erase_unordered(std::addressof(_data[index]));
		}

		/// <summary>
		/// 乱序根据item地址删除item返回当前地址【因为删除后就要往前走一格】
		/// </summary>
		/// <param name="item"></param>
		/// <returns></returns>
		constexpr T* const erase_unordered(T* const item) {
			assert(_data && item >= std::addressof(_data[0]) && item < std::addressof(_data[_size]));
			if constexpr (destruct) item->~T();
			--_size;
			if (item < std::addressof(_data[_size])) {
				memcpy(item, std::addressof(_data[_size]), sizeof(T));
			}
			return item;
		}

		/// <summary>
		/// 判断一下清除数据时销不销毁里面item的实例【释放内存↗↘↗↘】
		/// </summary>
		constexpr void clear() {
			if constexpr (destruct) {
				destruct_range(0, _size);
			}
			_size = 0;
		}

		/// <summary>
		/// 交换两个vector
		/// </summary>
		/// <param name="o"></param>
		constexpr void swap(vector& o) {
			if (this != std::addressof(o)) {
				auto temp(std::move(o));
				o.move(* this);
				move(temp);
			}
		}

		/// <summary>
		/// 返回一个指针指向data的开始，可能为null
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		constexpr T* data() {
			return _data;
		}

		/// <summary>
		/// 返回一个指针指向data的开始，可能为null
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		constexpr T* const data() const {
			return _data;
		}

		[[nodiscard]]
		constexpr bool empty() const {
			return _size == 0;
		}

		[[nodiscard]]
		constexpr u64 size() const {
			return _size;
		}

		[[nodiscard]]
		constexpr u64 capacity() const {
			return _capacity;
		}

		[[nodiscard]]
		constexpr T& operator[](u64 index) {
			assert(_data && index < _size);
			return _data[index];
		}

		[[nodiscard]]
		constexpr T& const operator[](u64 index) const {
			assert(_data && index < _size);
			return _data[index];
		}

		[[nodiscard]]
		constexpr T& front() {
			assert(_data && _size);
			return _data[0];
		}

		[[nodiscard]]
		constexpr T& const front() const {
			assert(_data && _size);
			return _data[0];
		}

		[[nodiscard]]
		constexpr T& back() {
			assert(_data && _size);
			return _data[0];
		}

		[[nodiscard]]
		constexpr T& const back() const {
			assert(_data && _size);
			return _data[_size - 1];
		}

		[[nodiscard]]
		constexpr T* begin() {
			assert(_data);
			return std::addressof(_data[0]);
		}

		[[nodiscard]]
		constexpr T* const begin() const {
			assert(_data);
			return std::addressof(_data[0]);
		}

		[[nodiscard]]
		constexpr T* end() {
			assert(_data);
			return std::addressof(_data[_size]);
		}

		[[nodiscard]]
		constexpr T* const end() const {
			assert(_data);
			return std::addressof(_data[_size]);
		}

	private:

		/// <summary>
		/// 移动函数
		/// </summary>
		/// <param name="o"></param>
		/// <returns></returns>
		const void move(vector& o) {
			_capacity = o._capacity;
			_size = o._size;
			_data = o._data;
			o.reset();
		}

		/// <summary>
		/// 重新初始化
		/// </summary>
		constexpr void reset() {
			_capacity = 0;
			_size = 0;
			_data = nullptr;
		}

		/// <summary>
		/// 调用范围内的实例的析构函数
		/// </summary>
		/// <param name="first"></param>
		/// <param name="last"></param>
		constexpr void destruct_range(u64 first, u64 last) {
			assert(destruct);
			assert(first <= _size && last <= _size && first <= last);
			if (_data) {
				for (; first != last; ++first) {
					_data[first].~T();
				}
			}
		}

		/// <summary>
		/// 销毁所有数据
		/// </summary>
		constexpr void destroy() {
			assert([&] {return _capacity ? _data != nullptr : _data == nullptr; }()); //用lambda函数
			clear();
			_capacity = 0;
			if (_data) free(_data);	//释放数据指针
			_data = nullptr;
		}

		u64 _capacity{ 0 };
		u64 _size{ 0 };
		T*	_data{ nullptr };
	};
}