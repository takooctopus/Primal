#pragma once
#include "..\Components\ComponentsCommon.h"
#include "TransformComponent.h"
#include "ScriptComponent.h"

namespace primal {

	namespace game_entity {
		DEFINE_TYPED_ID(entity_id);

		/// <summary>
		/// 对外接口的实体类，类的定义在这里，与此同时要放到命名空间中，可以看到不同文件相同命名空间有对此类操作的方法
		/// </summary>
		class entity {
		public:
			constexpr explicit entity(entity_id id) : _id{ id } {}
			constexpr entity() : _id{ id::invalid_id } {}
					
			/// <summary>
			/// 返回游戏实体的id
			/// </summary>
			/// <returns></returns>
			[[nodiscard]]
			constexpr entity_id get_id() const { return _id; }
			
			/// <summary>
			/// 判断游戏实体是否可用
			/// </summary>
			/// <returns>
			///   <c>true</c> if this instance is valid; otherwise, <c>false</c>.
			/// </returns>
			[[nodiscard]]
			constexpr bool is_valid() const { return id::is_valid(_id); }

			/// <summary>
			/// 返回实体下属的坐标变换实例
			/// </summary>
			/// <returns></returns>
			[[nodiscard]]
			transform::component transform() const;

			/// <summary>
			/// 返回实体下属的脚本实例
			/// </summary>
			/// <returns></returns>
			[[nodiscard]]
			script::component script() const;

		private:
			entity_id _id;
		};

	}// namespace game_entity


	namespace script {

		/// <summary>
		/// 用户脚本的抽象类，对于用户我们只需要继承这个接口就行
		/// </summary>
		/// <code>
		///	class my_player_character : public entity_script {
		///	public:
		///		void update(float dt)override {
		///			do player chracter update
		///		}
		///	};
		/// </code>
		class entity_script : public game_entity::entity {
		public:
			virtual ~entity_script() = default;
			virtual void begin_play() {}
			virtual void update(float) {}
		protected:
			constexpr explicit entity_script(game_entity::entity entity) : game_entity::entity(entity.get_id()) {}
		};

		namespace detail {
			/// <summary>
			/// 指向脚本的智能指针
			/// </summary>
			using script_ptr = std::unique_ptr<entity_script>;

			/// <summary>
			/// 脚本创建函数指针
			/// 这个指针传入一个entity 
			/// 最后返回一个script_ptr智能指针
			/// </summary>
			using script_creator = script_ptr(*)(game_entity::entity entity);

			/// <summary>
			/// 
			/// </summary>
			/// <typeparam name="script_class"></typeparam>
			/// <param name="entity"></param>
			/// <returns></returns>
			template<class script_class>
			script_ptr create_script(game_entity::entity entity) {
				assert(entity.is_valid());
				// 创建一个脚本实例并返回指向此script的指针
				return std::make_unique<script_class>(entity);
			}
		}// namespace detail
	}//namespace script 
}