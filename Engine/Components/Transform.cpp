#include "Transform.h"
#include "Entity.h"

namespace primal::transform {
	
namespace {
	util::vector<math::v3> positions;
	util::vector<math::v4> rotations;
	util::vector<math::v3> scales;
}//���������ռ�

[[nodiscard]]
component create_transform(const init_info& info, game_entity::entity entity)
{
	assert(entity.is_valid());
	const id::id_type entity_index{ id::index(entity.get_id()) };
	if (positions.size() > entity_index) {
		// TODO: �ж�����ò�����std::move�������õ�stl�����������Լ������ݽṹ��std::move���ܻ�ʧЧ����������ع���
		positions[entity_index] = math::v3(info.position);
		rotations[entity_index] = math::v4(info.rotation);
		scales[entity_index] = math::v3(info.scale);
	}else{
		assert(positions.size() == entity_index);
		positions.emplace_back(info.position);
		rotations.emplace_back(info.rotation);
		scales.emplace_back(info.scale);
	}
	return component(transform_id{ static_cast<id::id_type>(positions.size() - 1) });
}

void remove_transform(component c)
{
	assert(c.is_valid());
}


[[nodiscard]]
math::v3 component::position() const {
	assert(is_valid());
	return positions[id::index(_id)];
};
[[nodiscard]]
math::v4 component::rotation() const {
	assert(is_valid());
	return rotations[id::index(_id)];
};
[[nodiscard]]
math::v3 component::scale() const {
	assert(is_valid());
	return scales[id::index(_id)];
};


}