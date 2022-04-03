#pragma once
#include "ComponentsCommon.h"
namespace primal {
	
// ͨ������ʵ�ְ����������ŵ���ͬ�������ռ���
#define INIT_INFO(component) namespace component { struct init_info; }
	INIT_INFO(transform);
#undef INIT_INFO

namespace game_entity {
		

// ��Ϸʵ���ඨ�壺
// ÿ��ʵ�嶼����ð�������任��
struct entity_info
{
	transform::init_info* transform{ nullptr };
};

// ������Ϸʵ��
[[nodiscard]] entity
create_game_entity(const entity_info& info);

// �������Ƴ���Ϸʵ��
void 
remove_game_entity(entity e);

// �ж��Ƿ��Ծ[��ʵ�����ж�generation��������ǲ���һ���ģ���һ��˵�����Ƴ���]
[[nodiscard]] bool 
is_alive(entity e);
}
	
}