#pragma once
#include "ComponentsCommon.h"

namespace primal::transform {
	
	/// <summary>
	/// 坐标变换类的初始化信息结构体
	/// </summary>
	struct init_info
	{
		f32 position[3]{};
		f32 rotation[4]{};
		f32 scale[3]{ 1.f,1.f,1.f };
	};

	[[nodiscard]]
	/// <summary>
	/// 坐标变换类的构建函数 返回一个transform::component
	/// </summary>
	/// <param name="info"></param>
	/// <param name="entity"></param>
	/// <returns></returns>
	component create(init_info info, game_entity::entity entity);

	/// <summary>
	/// 移除一个transform::component
	/// </summary>
	/// <param name="c"></param>
	void remove(component c);

}