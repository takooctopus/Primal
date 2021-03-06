#include "Transform.h"
#include "Entity.h"

namespace primal::transform {

	namespace {
		utl::vector<math::v3> positions;
		utl::vector<math::v4> rotations;
		utl::vector<math::v3> scales;
	}//匿名命名空间

	[[nodiscard]]
	component create(init_info info, game_entity::entity entity)
	{
		assert(entity.is_valid());
		const id::id_type entity_index{ id::index(entity.get_id()) };
		if (positions.size() > entity_index) {
			// TODO: 判断这里该不该用std::move，现在用的stl，或许用了自己的数据结构后，std::move可能会失效，这个留给重构吧
			positions[entity_index] = math::v3(info.position);
			rotations[entity_index] = math::v4(info.rotation);
			scales[entity_index] = math::v3(info.scale);
		}
		else {
			assert(positions.size() == entity_index);
			positions.emplace_back(info.position);
			rotations.emplace_back(info.rotation);
			scales.emplace_back(info.scale);
		}
		return component{ transform_id{ entity.get_id() } };	// 坐标变换类和实体是一一对应的
	}

	void remove([[maybe_unused]] component c)
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