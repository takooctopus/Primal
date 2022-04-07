#pragma once
#include "ComponentsCommon.h"

namespace primal::script {


	/// <summary>
	/// 创建脚本实例的初始化信息结构体
	/// </summary>
	struct init_info
	{

		/// <summary>
		/// 需要的就是一个脚本创建函数指针
		/// </summary>
		detail::script_creator script_creator;
	};

	[[nodiscard]]
	/// <summary>
	/// 根据初始化信息和所属entity来创建组件
	/// </summary>
	/// <param name="info"></param>
	/// <param name="entity"></param>
	/// <returns></returns>
	component create(init_info info, game_entity::entity entity);

	/// <summary>
	/// 删除这个组件
	/// </summary>
	/// <param name="c"></param>
	void remove(component c);

	/// <summary>
	/// DEV ONLY:
	/// 暂时性的更新函数，里面暂时是遍历所有的entity_scripts，全部执行一遍里面的更新函数
	/// </summary>
	/// <param name="dt"></param>
	void update(float dt);
}