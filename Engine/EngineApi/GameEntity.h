#pragma once
#include "..\Components\ComponentsCommon.h"
#include "TransformComponent.h"
#include "ScriptComponent.h"

namespace primal {

	namespace game_entity {
		DEFINE_TYPED_ID(entity_id);

		/// <summary>
		/// ����ӿڵ�ʵ���࣬��Ķ�����������ͬʱҪ�ŵ������ռ��У����Կ�����ͬ�ļ���ͬ�����ռ��жԴ�������ķ���
		/// </summary>
		class entity {
		public:
			constexpr explicit entity(entity_id id) : _id{ id } {}
			constexpr entity() : _id{ id::invalid_id } {}
					
			/// <summary>
			/// ������Ϸʵ���id
			/// </summary>
			/// <returns></returns>
			[[nodiscard]]
			constexpr entity_id get_id() const { return _id; }
			
			/// <summary>
			/// �ж���Ϸʵ���Ƿ����
			/// </summary>
			/// <returns>
			///   <c>true</c> if this instance is valid; otherwise, <c>false</c>.
			/// </returns>
			[[nodiscard]]
			constexpr bool is_valid() const { return id::is_valid(_id); }

			/// <summary>
			/// ����ʵ������������任ʵ��
			/// </summary>
			/// <returns></returns>
			[[nodiscard]]
			transform::component transform() const;

			/// <summary>
			/// ����ʵ�������Ľű�ʵ��
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
		/// �û��ű��ĳ����࣬�����û�����ֻ��Ҫ�̳�����ӿھ���
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
			/// ָ��ű�������ָ��
			/// </summary>
			using script_ptr = std::unique_ptr<entity_script>;

			/// <summary>
			/// �ű���������ָ��
			/// ���ָ�봫��һ��entity 
			/// ��󷵻�һ��script_ptr����ָ��
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
				// ����һ���ű�ʵ��������ָ���script��ָ��
				return std::make_unique<script_class>(entity);
			}
		}// namespace detail
	}//namespace script 
}