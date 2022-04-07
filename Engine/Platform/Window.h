#pragma once
#include "CommonHeaders.h"

namespace primal::platform {

	DEFINE_TYPED_ID(window_id);

	/// <summary>
	/// ������
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
		/// �жϴ��ڵĿ�����
		/// </summary>
		/// <returns>
		///   <c>true</c> if this instance is valid; otherwise, <c>false</c>.
		/// </returns>
		[[nodiscard]]
		constexpr bool is_valid() const { return id::is_valid(_id); }

		/// <summary>
		/// �л�����ȫ��
		/// </summary>
		/// <param name="is_fullscreen">if set to <c>true</c> [is fullscreen].</param>
		void set_fullscreen(bool is_fullscreen) const;
		
		/// <summary>
		/// �����ǲ���ȫ��
		/// </summary>
		/// <returns>
		///   <c>true</c> if this instance is fullscreen; otherwise, <c>false</c>.
		/// </returns>
		[[nodiscard]]
		bool is_fullscreen() const;
		
		/// <summary>
		/// ����window Handles�� ��Ϊ�ÿ���ƽ̨�� ���Է���void*
		/// </summary>
		/// <returns></returns>
		void* handle() const;


		/// <summary>
		/// ���ô��ڱ���
		/// </summary>
		/// <param name="caption">The caption.</param>
		void set_caption(const wchar_t* caption) const;

		/// <summary>
		/// ��ȡ���ڴ�С
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		const math::u32v4 size() const;
		
		/// <summary>
		/// ���ݿ��������������
		/// </summary>
		/// <param name="width">The width.</param>
		/// <param name="height">The height.</param>
		void resize(u32 width, u32 height) const;

		/// <summary>
		/// ��ȡ���ڿ��
		/// </summary>
		/// <returns></returns>
		const u32 witdh() const;

		/// <summary>
		/// ��ȡ���ڸ߶�
		/// </summary>
		/// <returns></returns>
		const u32 height() const;
		
		/// <summary>
		/// ���������Ƿ�ر�
		/// </summary>
		/// <returns></returns>
		const bool is_closed() const;

	private:
		window_id _id{ id::invalid_id };
	};

} // namespace primal::platform