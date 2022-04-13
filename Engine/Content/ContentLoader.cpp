#include "ContentLoader.h"
#include "..\Components\Entity.h"
#include "..\Components\Transform.h"
#include "..\Components\Script.h"
#include "..\Graphics\Renderer.h"

#if !defined(SHIPPING)

#include <fstream>
#include <filesystem>
#include <Windows.h>

namespace primal::content {

	namespace {
		enum component_type {
			transform,
			script,

			count,
		};

		utl::vector<game_entity::entity> entities;

		transform::init_info transform_info{};
		script::init_info script_info{};


		/// <summary>
		/// 读取坐标变换，输入一个u8的指针地址，和一个init_info初始化信息结构体
		/// </summary>
		/// <param name="data">The data.</param>
		/// <param name="info">The information.</param>
		/// <returns></returns>
		[[nodiscard]]
		bool read_transform(const u8*& data, game_entity::entity_info& info) {
			using namespace DirectX;
			// 要注意在引擎中rotation是vector[4]，外部是[vector3]所以要中转一下
			f32 rotation[3];

			assert(!info.transform);
			memcpy(&transform_info.position[0], data, sizeof(transform_info.position)); data += sizeof(transform_info.position);
			memcpy(&rotation[0], data, sizeof(rotation)); data += sizeof(rotation);
			memcpy(&transform_info.scale[0], data, sizeof(transform_info.scale)); data += sizeof(transform_info.scale);

			XMFLOAT3A rot{ &rotation[0] };
			XMVECTOR quat{ XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&rot)) };
			XMFLOAT4A rot_quat{};
			XMStoreFloat4A(&rot_quat, quat);
			memcpy(&transform_info.rotation[0], &rot_quat.x, sizeof(transform_info.rotation));

			info.transform = &transform_info;
			return true;
		}

		/// <summary>
		/// 从buffer中读取脚本名称数据并扔给entity_info
		/// </summary>
		/// <param name="data">The data.</param>
		/// <param name="info">The information.</param>
		/// <returns></returns>
		[[nodiscard]]
		bool read_script(const u8*& data, game_entity::entity_info& info) {
			assert(!info.script);
			const u32 name_length{ *data }; data += sizeof(u32);
			if (!name_length) return false;
			// 脚本名字不可能超过255(8个bit)
			assert(name_length < 256);
			char script_name[256];
			memcpy(&script_name[0], data, name_length); data += name_length;
			// 让c-string最后一个为0来结束
			script_name[name_length] = 0;
			script_info.script_creator = script::detail::get_script_creator(script::detail::string_hash()(script_name));
			info.script = &script_info;
			return script_info.script_creator != nullptr;
		}


		/// <summary>
		/// 函数指针，(外部数据地址，游戏实体初始化结构体)
		/// </summary>
		using component_reader = bool(*)(const u8*&, game_entity::entity_info&);

		component_reader component_readers[]{
			read_transform,
			read_script
		};

		static_assert(_countof(component_readers) == component_type::count);

		bool read_file(std::filesystem::path path, std::unique_ptr<u8[]>& data, u64& size) {
			if (!std::filesystem::exists(path)) return false;
			size = std::filesystem::file_size(path);
			assert(size);
			if (!size) return false;
			data = std::make_unique<u8[]>(size);
			std::ifstream file{ path, std::ios::in | std::ios::binary };
			if (!file || !file.read((char*)data.get(), size)) {
				file.close();
				return false;
			}
			file.close();
			return true;
		}


	}//匿名namespace


	/// <summary>
	/// 读取C#中生成的game.bin 并创建对应实体
	/// </summary>
	/// <returns></returns>
	bool load_game()
	{

		//std::ifstream game{ "game.bin", std::ios::in | std::ios::binary };
		//utl::vector<u8> buffer{ std::istreambuf_iterator<char>(game),{} };
		//assert(buffer.size());
		//const u8* at{ buffer.data() };
		//const u32 su32{ sizeof(u32) };
		//const u32 num_entities{ *at }; at += su32;
		//if (!num_entities) return false;

		std::unique_ptr<u8[]> game_data{};
		u64 size{ 0 };
		if (!read_file("game.bin", game_data, size)) return false;
		assert(game_data.get());
		const u8* at{ game_data.get() };
		const u32 su32{ sizeof(u32) };
		const u32 num_entities{ *at }; at += su32;
		if (!num_entities) return false;

		for (u32 entity_index{ 0 }; entity_index < num_entities; ++entity_index) {
			game_entity::entity_info info{};
			const u32 entity_type{ *at }; at += su32;
			const u32 num_components{ *at }; at += su32;
			if (!num_components) return false;
			for (u32 component_index{ 0 }; component_index < num_components; ++component_index) {
				const u32 component_type{ *at }; at += su32;
				assert(component_type < component_type::count);
				if (!component_readers[component_type](at, info)) return false;
			}

			// 每个游戏实体都有一个坐标变换类
			assert(info.transform);
			game_entity::entity entity{ game_entity::create(info) };
			if (!entity.is_valid()) return false;

			entities.emplace_back(entity);
		}
		// 最后断言我们刚好读到所有字节
		assert(at == game_data.get() + size);
		return true;
	}

	void unload_game() {
		for (auto entity : entities) {
			game_entity::remove(entity.get_id());
		}
	}


	bool load_engine_shaders(std::unique_ptr<u8[]>& shaders, u64& size) {
		auto path = graphics::get_engine_shaders_path();
		return read_file(path, shaders, size);
	}
}

#endif // !defined(SHIPPING)