
#include "Common.h"

#include "..\Engine\Components\Entity.h"
#include "..\Engine\Components\Transform.h"
#include "..\Engine\Components\Script.h"

using namespace primal;

namespace {
	
	/// <summary>
	/// 坐标变换类的结构信息
	/// to_init_info()函数可以返回坐标类的初始化信息
	/// </summary>
	struct transform_component
	{
		f32 position[3];
		f32 rotation[3];
		f32 scale[3];

		/// <summary>
		/// 将transform_component结构体中的信息转化成要生成transform的初始化信息
		/// </summary>
		/// <returns></returns>
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

	/// <summary>
	/// 脚本类的描述信息结构体
	/// 里面应该就只有一个函数指针[说白了就是个地址]
	/// </summary>
	struct script_component
	{
		script::detail::script_creator script_creator;

		/// <summary>
		/// 根据script_component里的函数指针生成script类实例的初始化信息
		/// </summary>
		/// <returns></returns>
		script::init_info to_init_info() {
			script::init_info info{};
			info.script_creator = script_creator;
			return info;
		}
	};

	/// <summary>
	/// game_entity_descriptor对游戏实体的描述结构体:
	/// 坐标变换信息
	/// 脚本信息
	/// </summary>
	struct game_entity_descriptor
	{
		transform_component transform;
		script_component script;
	};

	/// <summary>
	/// 从id来获取entity实体，当然实际上这个是返回了一个新建的entity，因为在game_entity::remove_game_entity()中需要的其实也就是一个entity_id罢了，之后就和这个entity实例没啥关系了
	/// </summary>
	/// <param name="id">The identifier.</param>
	/// <returns></returns>
	[[nodiscard]]
	game_entity::entity entity_from_id(id::id_type id) {
		return game_entity::entity{ game_entity::entity_id{id} };
	}

} // 匿名空间



EDITOR_INTERFACE 
[[nodiscard]]
/// <summary>
/// 创建游戏实体接口
/// 接收一个游戏实体的描述信息结构体
/// 返回一个生成的实体id
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
id::id_type CreateGameEntity(game_entity_descriptor* e) {
	assert(e);
	// 根据传入的game_entity_descriptor初始化游戏实体
	game_entity_descriptor& desc{ *e };

	//根据描述信息生成各个类别的组件的初始化信息
	transform::init_info transform_info{ desc.transform.to_init_info() };
	script::init_info script_info{ desc.script.to_init_info() };

	game_entity::entity_info entity_info{
		&transform_info,
		&script_info,
	};
	// 最后向外暴露出的只有一个entity._id，我们可以看到entity里面其实就一个属性_id
	return game_entity::create(entity_info).get_id();
}


EDITOR_INTERFACE
/// <summary>
/// 对外暴露的接口删除引擎中的游戏实体
/// 传入entity对应的id
/// </summary>
/// <param name="id">The identifier.</param>
void RemoveGameEntity(id::id_type id) {
	assert(id::is_valid(id));
	game_entity::remove(game_entity::entity_id{ id });
}