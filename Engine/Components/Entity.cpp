#include "Entity.h"
#include "Transform.h"

namespace primal::game_entity {

namespace {
	//���������ռ䣬�����������ö��ⲻ�ɼ��ĺ�������Ȼ��Ҳ���������涨��static�����һ��Ǹ�ϲ�����
	util::vector<id::generation_type>	generations;
	util::deque<entity_id>				free_ids;

	util::vector<transform::component>	transforms;
} 


[[nodiscard]] 
entity create_game_entity(const entity_info& info)
{
	assert(info.transform); // ���е�game entity������Ҫ������任������
	if (!info.transform) return entity{};
	
	entity_id id;
	
	// ��Ϊģ����ʹ��������ģ���ظ�ʹ�õ����飬�����жϿ����б��ǲ��ǹ���
	// 1. �㹻��͸��ã�������Ӧindex�е�generation��1��
	// 2. �������vector���¼���һ����ʼ��λ��(����generation��ʼ��Ϊ0)
	if (free_ids.size() > id::min_deleted_elements) {
		id = free_ids.front();
		assert(!is_alive(entity{ id }));
		free_ids.pop_front();
		id = entity_id{ id::new_generation(id) };
		++generations[id::index(id)];
	}
	else {
		// ʹ��generations.size()��vector��С���г�ʼ������Ӧ��һ��index�������µ�entityĬ��generation�϶�Ϊ0[generation��id_type��ǰ�漸���ֽ�]�����Կ���ֱ��ʹ��index����ʼ��entity_id
		id = entity_id{ static_cast<id::id_type>(generations.size()) };
		generations.push_back(0);
		transforms.emplace_back();
	}

	// ʹ�ö�Ӧ���ɵ�id���г�ʼ������entity
	const entity new_entity{ id };
	// ʹ�ö�Ӧ���ɵ�id��ȡ�����ֽڵ�index
	const id::id_type index{ id::index(id) };

	// ����transformComponent���Ͼ�ÿ��ʵ�嶼��������任��
	assert(!transforms[index].is_valid());
	transforms[index] = transform::create_transform(*info.transform, new_entity);
	if (!transforms[index].is_valid()) return {}; // Ĭ�Ϸ���һ����invalid_id����entity

	return new_entity;
}

void remove_game_entity(entity e)
{
	const entity_id id{ e.get_id() };
	const id::id_type index{ id::index(id) };
	// Ҫ�Ƴ��Ļ����ñ�֤�Ƴ�����Ϸʵ���ǻ��
	assert(is_alive(e));
	if (is_alive(e)) {
		transform::remove_transform(transforms[index]);
		transforms[index] = {}; //ͬ���ģ��ӽ�ȥһ��Ĭ�Ϲ���Ϊinvalid_id����component
		free_ids.push_back(id);
	}
}

[[nodiscard]] 
bool is_alive(entity e)
{
	// ���ж�generationû��(-1)���ֵ�����ʵ�������Ч��
	assert(e.is_valid());
	// ��ȡʵ��id�Ͷ�Ӧ��index
	const entity_id id{ e.get_id() };
	const id::id_type index{ id::index(id) };
	// ��֤index�������У�û�г��������������������
	assert(index < generations.size());
	assert(generations[index] == id::generation(id));
	return (generations[index] == id::generation(id) && transforms[index].is_valid());
}


[[nodiscard]] 
transform::component entity::transform() const {
	assert(is_alive(*this));
	const id::id_type index{ id::index(_id) };
	return transforms[index];
}

}