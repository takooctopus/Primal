#pragma once
#include "CommonHeaders.h"
#include "..\Platform\Window.h"
namespace primal::graphics {

	DEFINE_TYPED_ID(surface_id);

	class surface {
	public:
		/// <summary>
		/// Initializes a new instance of the <see cref="component"/> class.
		/// </summary>
		/// <param name="id">The identifier.</param>
		constexpr explicit surface(surface_id id) : _id{ id } {}
		constexpr surface() = default;

		/// <summary>
		/// Gets the identifier.
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		constexpr surface_id get_id() const { return _id; }

		/// <summary>
		/// �жϴ��ڵĿ�����
		/// </summary>
		/// <returns>
		///   <c>true</c> if this instance is valid; otherwise, <c>false</c>.
		/// </returns>
		[[nodiscard]]
		constexpr bool is_valid() const { return id::is_valid(_id); }

		/// <summary>
		/// �ı�ƽ���С
		/// </summary>
		/// <param name="width">The width.</param>
		/// <param name="height">The height.</param>
		void resize(u32 width, u32 height) const;

		/// <summary>
		/// ��ȡƽ�洰�ڿ��
		/// </summary>
		/// <returns></returns>
		u32 witdh() const;

		/// <summary>
		/// ��ȡƽ�洰�ڸ߶�
		/// </summary>
		/// <returns></returns>
		u32 height() const;

		/// <summary>
		/// ��Ⱦ
		/// </summary>
		/// <returns></returns>
		void render() const;

	private:
		surface_id _id{ id::invalid_id };
	};

	/// <summary>
	/// �ṹ�� ��Ⱦƽ��
	/// </summary>
	struct render_surface {
		platform::window window{};	// ����ʵ��
		surface surface{};	// ƽ��ʵ��
	};

	/// <summary>
	/// ö�� ͼ��ƽ̨
	/// </summary>
	enum class graphics_platform {
		direct3d12 = 0,
		vulkan = 1,
		open_gl = 2,
	};

	/// <summary>
	/// ��ʼ��ͼ��ƽ̨����
	/// </summary>
	/// <param name="platform"></param>
	/// <returns></returns>
	[[nodiscard]]
	bool initialize(graphics_platform platform);
	
	/// <summary>
	/// �˳�ͼ��ƽ̨����
	/// </summary>
	void shutdown();

	/// <summary>
	/// ��Ⱦ����
	/// </summary>
	void render();


	/// <summary>
	/// ͨ�����ڴ���һ������
	/// </summary>
	/// <param name="window"></param>
	/// <returns></returns>
	[[nodiscard]]
	surface create_surface(platform::window window);

	void remove_surface(surface_id id);
}