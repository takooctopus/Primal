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
		/// 判断窗口的可用性
		/// </summary>
		/// <returns>
		///   <c>true</c> if this instance is valid; otherwise, <c>false</c>.
		/// </returns>
		[[nodiscard]]
		constexpr bool is_valid() const { return id::is_valid(_id); }

		/// <summary>
		/// 改变平面大小
		/// </summary>
		/// <param name="width">The width.</param>
		/// <param name="height">The height.</param>
		void resize(u32 width, u32 height) const;

		/// <summary>
		/// 获取平面窗口宽度
		/// </summary>
		/// <returns></returns>
		u32 witdh() const;

		/// <summary>
		/// 获取平面窗口高度
		/// </summary>
		/// <returns></returns>
		u32 height() const;

		/// <summary>
		/// 渲染
		/// </summary>
		/// <returns></returns>
		void render() const;

	private:
		surface_id _id{ id::invalid_id };
	};

	/// <summary>
	/// 结构体 渲染平面
	/// </summary>
	struct render_surface {
		platform::window window{};	// 窗口实例
		surface surface{};	// 平面实例
	};

	/// <summary>
	/// 枚举 图形平台
	/// </summary>
	enum class graphics_platform {
		direct3d12 = 0,
		vulkan = 1,
		open_gl = 2,
	};

	/// <summary>
	/// 初始化图形平台函数
	/// </summary>
	/// <param name="platform"></param>
	/// <returns></returns>
	[[nodiscard]]
	bool initialize(graphics_platform platform);
	
	/// <summary>
	/// 退出图形平台函数
	/// </summary>
	void shutdown();

	/// <summary>
	/// 渲染场景
	/// </summary>
	void render();


	/// <summary>
	/// 通过窗口创建一个表面
	/// </summary>
	/// <param name="window"></param>
	/// <returns></returns>
	[[nodiscard]]
	surface create_surface(platform::window window);

	void remove_surface(surface_id id);
}