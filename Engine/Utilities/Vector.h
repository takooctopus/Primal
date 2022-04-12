#pragma once
#include "CommonHeaders.h"

namespace primal::utl {
	template<typename T, bool destruct = true>
	class vector {
	public:
		vector() = default;	//Ĭ�Ϲ��캯�����������ڴ�
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
		/// �Ҳ�����ֵ��=����
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
		/// �Ҳ�����ֵ��=����
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
		/// ��ֵ����
		/// </summary>
		/// <param name="value"></param>
		constexpr void push_back(const T& value) {
			emplace_back(value);
		}

		/// <summary>
		/// ��ֵ���롾Ҫ��ת����
		/// </summary>
		/// <param name="value"></param>
		constexpr void push_back(const T&& value) {
			emplace_back(std::move(value));
		}

		/// <summary>
		/// ���������ƶ����춼��
		/// </summary>
		/// <typeparam name="...params"></typeparam>
		/// <param name="...p"></param>
		/// <returns></returns>
		template<class... params>
		constexpr decltype(auto) emplace_back(params&&... p) {
			if (_size == _capacity) {
				reserve((_capacity + 1) * 3 >> 1);	// reserve����1.5����С�Ŀռ�
			}
			assert(_size < _capacity);

			T* const item{ new (std::addressof(_data[_size])) T(std::forward<params>(p)...) };
			++_size;
			return *item;
		}

		/// <summary>
		/// ���·���ռ䣬�������г�ʼ��һ��Ĭ��ʵ��
		/// </summary>
		/// <param name="new_size"></param>
		constexpr void resize(u64 new_size) {
			static_assert(std::is_default_constructible_v<T>, "Type must be default constructible.");	//����std::vectorһ����resize��Ҫʵ�ʷ���ռ��
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
		/// ���·���ռ䣬�������г�ʼ��һ��valueʵ��
		/// </summary>
		/// <param name="new_size"></param>
		constexpr void resize(u64 new_size, const T& value) {
			static_assert(std::is_copy_constructible_v<T>, "Type must be copy constructible.");	//����std::vectorһ����resize��Ҫʵ�ʷ���ռ��
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
		/// ���·����ڴ�
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
		/// ɾ��һ��item��������һ��ָ��
		/// </summary>
		/// <param name="index"></param>
		/// <returns></returns>
		constexpr T* const erase(u64 index) {
			assert(_data && index < _size);
			return erase(std::addressof(_data[index]));
		}

		/// <summary>
		/// ����item��ַɾ��item���ص�ǰ��ַ����Ϊɾ�����Ҫ��ǰ��һ��
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
		/// ����ɾ��item
		/// </summary>
		/// <param name="index"></param>
		/// <returns></returns>
		constexpr T* const erase_unordered(u64 index) {
			assert(_data && index < _size);
			return erase_unordered(std::addressof(_data[index]));
		}

		/// <summary>
		/// �������item��ַɾ��item���ص�ǰ��ַ����Ϊɾ�����Ҫ��ǰ��һ��
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
		/// �ж�һ���������ʱ������������item��ʵ�����ͷ��ڴ�J�K�J�K��
		/// </summary>
		constexpr void clear() {
			if constexpr (destruct) {
				destruct_range(0, _size);
			}
			_size = 0;
		}

		/// <summary>
		/// ��������vector
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
		/// ����һ��ָ��ָ��data�Ŀ�ʼ������Ϊnull
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		constexpr T* data() {
			return _data;
		}

		/// <summary>
		/// ����һ��ָ��ָ��data�Ŀ�ʼ������Ϊnull
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
		/// �ƶ�����
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
		/// ���³�ʼ��
		/// </summary>
		constexpr void reset() {
			_capacity = 0;
			_size = 0;
			_data = nullptr;
		}

		/// <summary>
		/// ���÷�Χ�ڵ�ʵ������������
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
		/// ������������
		/// </summary>
		constexpr void destroy() {
			assert([&] {return _capacity ? _data != nullptr : _data == nullptr; }()); //��lambda����
			clear();
			_capacity = 0;
			if (_data) free(_data);	//�ͷ�����ָ��
			_data = nullptr;
		}

		u64 _capacity{ 0 };
		u64 _size{ 0 };
		T*	_data{ nullptr };
	};
}