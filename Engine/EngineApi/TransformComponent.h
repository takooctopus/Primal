#pragma once
#include "..\Common\CommonHeaders.h"

namespace primal::transform {

	DEFINE_TYPED_ID(transform_id);
	
	/// <summary>
	/// �������������ע�ⲻ��������
	/// ��ʽʵ��������id
	/// Ĭ��ʵ������ʾһ���Ƿ���ʵ��
	/// </summary>
	class component final {
	public:		

		constexpr explicit component(transform_id id) : _id{ id } {}
		constexpr component() : _id{ id::invalid_id } {}

		/// <summary>
		/// ��ȡ�����Ӧ��transform_id
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		constexpr transform_id get_id() const { return _id; }

		/// <summary>
		/// �鿴���������
		/// </summary>
		/// <returns>
		///   <c>true</c> if this instance is valid; otherwise, <c>false</c>.
		/// </returns>
		[[nodiscard]]
		constexpr bool is_valid() const { return id::is_valid(_id); }


		/// <summary>
		/// ��ȡ��ά����
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		math::v3 position() const;
		
		/// <summary>
		/// ��ȡ��ת����
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		math::v4 rotation() const;

		/// <summary>
		/// ��ȡ��������
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		math::v3 scale() const;
	private:
		transform_id _id;
	};

}