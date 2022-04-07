#pragma once
#include "CommonHeaders.h"

namespace primal::platform {

	DEFINE_TYPED_ID(window_id);

	/// <summary>
	/// 窗口类
	/// </summary>
	class window {
	public:
		/// <summary>
		/// Initializes a new instance of the <see cref="component"/> class.
		/// </summary>
		/// <param name="id">The identifier.</param>
		constexpr explicit window(window_id id) : _id{ id } {}
		constexpr window() : _id{ id::invalid_id } {}

		/// <summary>
		/// Gets the identifier.
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		constexpr window_id get_id() const { return _id; }

		/// <summary>
		/// 判断窗口的可用性
		/// </summary>
		/// <returns>
		///   <c>true</c> if this instance is valid; otherwise, <c>false</c>.
		/// </returns>
		[[nodiscard]]
		constexpr bool is_valid() const { return id::is_valid(_id); }

		void set_fullscreen(bool is_fullscreen) const;
		bool is_fullscreen() const;
		void* handle() const;
		void set_caption(const char* caption) const;
		const math::u32v4 size() const;
		void resize(u32 width, u32 height) const;
		const u32 witdh() const;
		const u32 height() const;
		const bool is_closed() const;

	private:
		window_id _id{ id::invalid_id };
	};

} // namespace primal::platform