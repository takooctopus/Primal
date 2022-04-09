#pragma once

#include "CommonHeaders.h"
#include "MathTypes.h"


namespace primal::math {
	/// <summary>
	/// Clamps the specified value. Ҫ��С����Сֵ�ͷ�����Сֵ��Ҫ�Ǵ������ֵ�ͷ������ֵ
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
	/// ����bits��Ŀ��С���а�װ����������f => [u32](f * (2^bits-1) + 0.5)��һ����Χ��0-1�ĸ������Ŵ�bitsλ������
	/// </summary>
	/// <param name="f">The f.</param>
	/// <returns></returns>
	template<u32 bits>
	constexpr u32 pack_unit_float(f32 f)
	{
		static_assert(bits <= sizeof(u32) * 8);
		assert(f >= 0.f && f <= 1.f);
		constexpr f32 intervals{ (f32)((1ui32 << bits) - 1) }; //���intervals����С�Ļ�����2^bits-1
		return (u32)(intervals * f + 0.5f); //��󷵻�һ��u32���͵���[��0.5��Ϊ�˷��ر߽�������0=>0.5, 1=>max+0.5]
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