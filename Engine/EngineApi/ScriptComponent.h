#pragma once
#include "..\Common\CommonHeaders.h"


namespace primal::script {

	DEFINE_TYPED_ID(script_id);
	
	/// <summary>
	/// 脚本类型的组件
	/// </summary>
	class component final {
	public:		
		/// <summary>
		/// Initializes a new instance of the <see cref="component"/> class.
		/// </summary>
		/// <param name="id">The identifier.</param>
		constexpr explicit component(script_id id) : _id{ id } {}
		constexpr component() : _id{ id::invalid_id } {}

		/// <summary>
		/// Gets the identifier.
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		constexpr script_id get_id() const { return _id; }

		/// <summary>
		/// Determines whether this instance is valid.
		/// </summary>
		/// <returns>
		///   <c>true</c> if this instance is valid; otherwise, <c>false</c>.
		/// </returns>
		[[nodiscard]]
		constexpr bool is_valid() const { return id::is_valid(_id); }

	private:
		script_id _id;
	};

}