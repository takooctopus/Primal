
#ifndef EDITOR_INTERFACE
#define EDITOR_INTERFACE extern "C" __declspec(dllexport)
#endif // !EDITOR_INTERFACE

#include "CommonHeaders.h"
#include "Id.h"
#include "..\Engine\Components\Entity.h"
#include "..\Engine\Components\Transform.h"

using namespace primal;

namespace {


struct transform_component
{
	f32 position[3];
	f32 rotation[3];
	f32 scale[3];

	transform::init_info to_init_info() {
		using namespace DirectX;
		transform::init_info info{};
		memcpy(&info.position[0], &position[0], sizeof(f32) * _countof(position));
		memcpy(&info.scale[0], &scale[0], sizeof(f32) * _countof(scale));
		XMFLOAT3A rot{ &rotation[0] };
		XMVECTOR quat{ XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&rot)) };
		XMFLOAT4A rot_quat{};
		XMStoreFloat4A(&rot_quat, quat);
		memcpy(&info.rotation[0], &rot_quat.x, sizeof(f32) * _countof(info.rotation));

		return info;
	}
};


struct game_entity_descriptor
{
	transform_component transform;
};

//��id����ȡentityʵ�壬��Ȼʵ��������Ƿ�����һ���½���entity����Ϊ��game_entity::remove_game_entity()����Ҫ����ʵҲ����һ��entity_id���ˣ�֮��ͺ����entityʵ��ûɶ��ϵ��
[[nodiscard]]
game_entity::entity entity_from_id(id::id_type id) {
	return game_entity::entity{ game_entity::entity_id{id} };
}

} // �����ռ�

EDITOR_INTERFACE [[nodiscard]]
id::id_type CreateGameEntity(game_entity_descriptor* e) {
	assert(e);
	game_entity_descriptor& desc{ *e };
	transform::init_info transform_info{ desc.transform.to_init_info() };
	game_entity::entity_info entity_info{
		&transform_info
	};
	// ������Ⱪ¶����ֻ��һ��entity._id�����ǿ��Կ���entity������ʵ��һ������_id
	return game_entity::create_game_entity(entity_info).get_id();
}

EDITOR_INTERFACE
void RemoveGameEntity(id::id_type id) {
	assert(id::is_valid(id));
	game_entity::remove_game_entity(entity_from_id(id));
}