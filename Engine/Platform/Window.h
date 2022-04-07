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

		/// <summary>
		/// 切换窗口全屏
		/// </summary>
		/// <param name="is_fullscreen">if set to <c>true</c> [is fullscreen].</param>
		void set_fullscreen(bool is_fullscreen) const;
		
		/// <summary>
		/// 看看是不是全屏
		/// </summary>
		/// <returns>
		///   <c>true</c> if this instance is fullscreen; otherwise, <c>false</c>.
		/// </returns>
		[[nodiscard]]
		bool is_fullscreen() const;
		
		/// <summary>
		/// 返回window Handles， 因为得考虑平台， 所以返回void*
		/// </summary>
		/// <returns></returns>
		void* handle() const;


		/// <summary>
		/// 设置窗口标题
		/// </summary>
		/// <param name="caption">The caption.</param>
		void set_caption(const wchar_t* caption) const;

		/// <summary>
		/// 获取窗口大小
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		const math::u32v4 size() const;
		
		/// <summary>
		/// 根据宽高重新伸缩窗口
		/// </summary>
		/// <param name="width">The width.</param>
		/// <param name="height">The height.</param>
		void resize(u32 width, u32 height) const;

		/// <summary>
		/// 获取窗口宽度
		/// </summary>
		/// <returns></returns>
		const u32 witdh() const;

		/// <summary>
		/// 获取窗口高度
		/// </summary>
		/// <returns></returns>
		const u32 height() const;
		
		/// <summary>
		/// 窗口现在是否关闭
		/// </summary>
		/// <returns></returns>
		const bool is_closed() const;

	private:
		window_id _id{ id::invalid_id };
	};

} // namespace primal::platform