#include "Entity.h"
#include "Transform.h"
#include "Script.h"

namespace primal::game_entity {

	/// <summary>
	/// ���������ռ䣬�����������ö��ⲻ�ɼ��ĺ�������Ȼ��Ҳ���������涨��static�����һ��Ǹ�ϲ�����
	/// </summary>
	namespace {
		
		/// <summary>
		/// ���game_entity�����飬�����ŵ��Ǵ���ֵgeneration[��id::internal::generation_bits���ƣ���id��ǰ�����ֽ�]��������Ԫ�ر����index����id�ĺ�ʮ���ֽ�
		/// </summary>
		utl::vector<id::generation_type>	generations;	

		/// <summary>
		/// һ��˫���������������Ϊremove���ճ�����generations��������Ӧ��generation+index
		/// </summary>
		utl::deque<entity_id>				free_ids;	

		utl::vector<transform::component>	transforms;
		utl::vector<script::component>		scripts;
	}


	[[nodiscard]]
	entity create(entity_info info)
	{
		assert(info.transform); // ���е�game entity������Ҫ������任������
		if (!info.transform) return entity{};

		entity_id id;

		// ��Ϊģ����ʹ��������ģ���ظ�ʹ�õ����飬�����жϿ����б��ǲ��ǹ���
		// 1. �㹻��͸��ã�������Ӧindex�е�generation��1��
		// 2. �������vector���¼���һ����ʼ��λ��(����generation��ʼ��Ϊ0)
		if (free_ids.size() > id::min_deleted_elements) {
			id = free_ids.front();
			assert(!is_alive(id));
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
		{
			assert(!transforms[index].is_valid());
			transforms[index] = transform::create(*info.transform, new_entity);
			if (!transforms[index].is_valid()) return {}; // Ĭ�Ϸ���һ����invalid_id����entity
		}

		// �����ű�ScriptComponent[��ѡ]
		{
			if (info.script && info.script->script_creator) {
				assert(!scripts[index].is_valid()); // �������entity��ʼû�ж�Ӧ�Ľű�ʵ��
				scripts[index] = script::create(*info.script, new_entity);
				assert(scripts[index].is_valid()); // �������entity��ʼû�ж�Ӧ�Ľű�ʵ��
				// TODO: ����scripts����index�ᳬ�������С��ǰ��Ӧ��һ������vector��С
			}
		}

		return new_entity;
	}

	void remove(entity_id id)
	{
		const id::id_type index{ id::index(id) };
		// Ҫ�Ƴ��Ļ����ñ�֤�Ƴ�����Ϸʵ���ǻ��
		assert(is_alive(id));
		// �Ƴ�transform
		{
			transform::remove(transforms[index]);
			transforms[index] = {}; //ͬ���ģ��ӽ�ȥһ��Ĭ�Ϲ���Ϊinvalid_id����component
		}
		free_ids.push_back(id);
	}

	[[nodiscard]]
	bool is_alive(entity_id id)
	{
		// ���ж�generationû��(-1)���ֵ�����ʵ�������Ч��
		assert(id::is_valid(id));
		// ��ȡʵ��id�Ͷ�Ӧ��index
		const id::id_type index{ id::index(id) };
		// ��֤index�������У�û�г��������������������
		assert(index < generations.size());
		assert(generations[index] == id::generation(id));
		return (generations[index] == id::generation(id) && transforms[index].is_valid());
	}


	[[nodiscard]]
	transform::component entity::transform() const {
		assert(is_alive(_id));
		const id::id_type index{ id::index(_id) };
		return transforms[index];
	}

	[[nodiscard]]
	script::component entity::script() const {
		assert(is_alive(_id));
		const id::id_type index{ id::index(_id) };
		return scripts[index];
	}
}