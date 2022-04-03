#pragma once
#include "ComponentsCommon.h"
namespace primal {
	
// 通过宏来实现把组件派生类放到相同的命名空间中
#define INIT_INFO(component) namespace component { struct init_info; }
	INIT_INFO(transform);
#undef INIT_INFO

namespace game_entity {
		

// 游戏实体类定义：
// 每个实体都必须得包含坐标变换类
struct entity_info
{
	transform::init_info* transform{ nullptr };
};

// 创建游戏实体
[[nodiscard]] entity
create_game_entity(const entity_info& info);

// 从数组移除游戏实体
void 
remove_game_entity(entity e);

// 判断是否活跃[其实就是判断generation这个属性是不是一样的，不一样说明被移除了]
[[nodiscard]] bool 
is_alive(entity e);
}
	
}