#include "Entity.h"
#include "Transform.h"

namespace primal::game_entity {

namespace {
	//匿名命名空间，方便我们设置对外不可见的函数，当然你也可以在外面定义static，但我还是更喜欢这个
	util::vector<id::generation_type>	generations;
	util::deque<entity_id>				free_ids;

	util::vector<transform::component>	transforms;
} 


[[nodiscard]] 
entity create_game_entity(const entity_info& info)
{
	assert(info.transform); // 所有的game entity都必须要有坐标变换这个组件
	if (!info.transform) return entity{};
	
	entity_id id;
	
	// 因为模型是使用了线性模型重复使用的数组，首先判断空闲列表是不是够大
	// 1. 足够大就复用，并将对应index中的generation加1，
	// 2. 否则就往vector中新加入一个初始化位置(其中generation初始化为0)
	if (free_ids.size() > id::min_deleted_elements) {
		id = free_ids.front();
		assert(!is_alive(entity{ id }));
		free_ids.pop_front();
		id = entity_id{ id::new_generation(id) };
		++generations[id::index(id)];
	}
	else {
		// 使用generations.size()即vector大小进行初始化，对应下一个index，由于新的entity默认generation肯定为0[generation在id_type的前面几个字节]，所以可以直接使用index来初始化entity_id
		id = entity_id{ static_cast<id::id_type>(generations.size()) };
		generations.push_back(0);
		transforms.emplace_back();
	}

	// 使用对应生成的id进行初始化生成entity
	const entity new_entity{ id };
	// 使用对应生成的id获取后面字节的index
	const id::id_type index{ id::index(id) };

	// 创建transformComponent，毕竟每个实体都得有坐标变换类
	assert(!transforms[index].is_valid());
	transforms[index] = transform::create_transform(*info.transform, new_entity);
	if (!transforms[index].is_valid()) return {}; // 默认返回一个有invalid_id的新entity

	return new_entity;
}

void remove_game_entity(entity e)
{
	const entity_id id{ e.get_id() };
	const id::id_type index{ id::index(id) };
	// 要移除的话，得保证移除的游戏实体是活动的
	assert(is_alive(e));
	if (is_alive(e)) {
		transform::remove_transform(transforms[index]);
		transforms[index] = {}; //同样的，扔进去一个默认构造为invalid_id的新component
		free_ids.push_back(id);
	}
}

[[nodiscard]] 
bool is_alive(entity e)
{
	// 先判断generation没到(-1)这个值，这个实体得是有效的
	assert(e.is_valid());
	// 获取实体id和对应的index
	const entity_id id{ e.get_id() };
	const id::id_type index{ id::index(id) };
	// 保证index在数组中，没有出现溢出到数组外面的情况
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