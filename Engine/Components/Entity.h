#pragma once
#include "ComponentsCommon.h"
namespace primal {

	/// <summary>
	/// 通过宏来实现把组件派生类放到相同的命名空间中，在不同文件的相同命名空间中要放对应的
	/// </summary>
#define INIT_INFO(component) namespace component { struct init_info; }
	INIT_INFO(transform);
	INIT_INFO(script);
#undef INIT_INFO

	namespace game_entity {

		/// <summary>
		/// 游戏实体类定义：
		/// <value>transform：坐标变换初始化信息[每个实体都必须得包含坐标变换类]</value>
		/// <value>script：脚本初始化信息</value>
		/// </summary>
		struct entity_info
		{
			transform::init_info* transform{ nullptr };
			script::init_info* script{ nullptr };
		};

		
		/// <summary>
		/// 游戏实体的创建函数
		/// </summary>
		/// <param name="info"></param>
		[[nodiscard]] 
		entity create(entity_info info);

		/// <summary>
		/// 从数组移除游戏实体，要注意移除顺序要反向【让我找了半天的BUG】要不就崩溃了
		/// </summary>
		/// <param name="id"></param>
		void remove(entity_id id);

		 
		/// <summary>
		/// 判断游戏实体是否活跃[其实就是判断generation这个属性是不是一样的，不一样说明被移除了]
		/// </summary>
		/// <param name="id"></param>
		/// <returns></returns>
		[[nodiscard]]
		bool is_alive(entity_id id);
	}

}