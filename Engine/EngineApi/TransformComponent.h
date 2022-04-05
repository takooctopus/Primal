#pragma once
#include "..\Common\CommonHeaders.h"

namespace primal::transform {

	DEFINE_TYPED_ID(transform_id);
	
	/// <summary>
	/// 坐标类型组件，注意不可再派生
	/// 显式实例化传入id
	/// 默认实例化表示一个非法的实例
	/// </summary>
	class component final {
	public:		

		constexpr explicit component(transform_id id) : _id{ id } {}
		constexpr component() : _id{ id::invalid_id } {}

		/// <summary>
		/// 获取组件对应的transform_id
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		constexpr transform_id get_id() const { return _id; }

		/// <summary>
		/// 查看组件可用性
		/// </summary>
		/// <returns>
		///   <c>true</c> if this instance is valid; otherwise, <c>false</c>.
		/// </returns>
		[[nodiscard]]
		constexpr bool is_valid() const { return id::is_valid(_id); }


		/// <summary>
		/// 获取三维坐标
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		math::v3 position() const;
		
		/// <summary>
		/// 获取旋转属性
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		math::v4 rotation() const;

		/// <summary>
		/// 获取缩放属性
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		math::v3 scale() const;
	private:
		transform_id _id;
	};

}