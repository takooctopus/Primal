#pragma once

#include "CommonHeaders.h"
#include "MathTypes.h"


namespace primal::math {
	/// <summary>
	/// Clamps the specified value. 要是小于最小值就返回最小值，要是大于最大值就返回最大值
	/// </summary>
	/// <param name="value">The value.</param>
	/// <param name="min">The minimum.</param>
	/// <param name="max">The maximum.</param>
	/// <returns></returns>
	template<typename T>
	constexpr T clamp(T value, T min, T max) {
		return (value < min) ? min : (value > max) ? max : value;
	}

	/// <summary>
	/// 根据bits数目大小进行包装，将浮点数f => [u32](f * (2^bits-1) + 0.5)【一个范围是0-1的浮点数放大到bits位整数】
	/// </summary>
	/// <param name="f">The f.</param>
	/// <returns></returns>
	template<u32 bits>
	constexpr u32 pack_unit_float(f32 f)
	{
		static_assert(bits <= sizeof(u32) * 8);
		assert(f >= 0.f && f <= 1.f);
		constexpr f32 intervals{ (f32)((1ui32 << bits) - 1) }; //间隔intervals，大小的话算是2^bits-1
		return (u32)(intervals * f + 0.5f); //最后返回一个u32类型的数[加0.5是为了返回边界条件，0=>0.5, 1=>max+0.5]
	}

	template <u32 bits>
	constexpr f32 unpack_to_unit_float(u32 i) {
		static_assert(bits <= sizeof(u32) * 8);
		assert(i < (1ui32 << bits));
		constexpr f32 intervals{ (f32)((1ui32 << bits) - 1) };
		return (f32)i / intervals;
	}

	template <u32 bits>
	constexpr u32 pack_float(f32 f, f32 min, f32 max) {
		assert(min < max);
		assert(f <= max && f >= min);
		const f32 distance{ (f - min) / (max - min) };
		return pack_unit_float<bits>(distance);
	}

	template <u32 bits>
	constexpr f32 unpack_to_float(u32 i, f32 min, f32 max) {
		assert(min < max);
		return unpack_to_unit_float<bits>(i) * (max - min) + min;
	}
}