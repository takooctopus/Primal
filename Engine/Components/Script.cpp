#include "Script.h"
#include "Entity.h"

namespace primal::script {
	namespace {

		/// <summary>
		/// The generations�������飬ͨ��id_mapping��ָ�������entity_scripts�еĽű�ʵ��
		/// </summary>
		utl::vector<id::generation_type>		generations;

		/// <summary>
		/// ����scripts_id����
		/// </summary>
		utl::deque<script_id>					free_ids;

		/// <summary>
		/// ����ָ�����飬Ԫ��ָ��ű�ʵ��
		/// �ܼ����Ľű�ʵ������
		/// </summary>
		utl::vector<detail::script_ptr>			entity_scripts;


		/// <summary>
		/// The identifier mapping [generations �� entity_scripts ��ӳ��]
		/// </summary>
		utl::vector<id::id_type>				id_mapping;

		/// <summary>
		/// һ�����ͣ�ӳ�䣬��tag=>script_creator
		/// </summary>
		using script_registry = utl::unordered_map<size_t, detail::script_creator>;

		/// <summary>
		/// ע���û��ű��õĺ�����������ʼ���ű�ӳ�䣬��Ϊ����֪��static�����������ȳ�ʼ���ı�������������Ҫȷ������ʹ����֮ǰ�Ѿ�����ʼ��
		/// </summary>
		/// <returns></returns>
		script_registry& registery() {
			/// <summary>
			/// �û��ű�ע��ӳ��unordered_map
			/// </summary>
			static script_registry reg;
			return reg;
		}

#ifdef USE_WITH_EDITOR

		/// <summary>
		/// ���û�У���ʼ��һ��static������[string]������������ʵ��
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		utl::vector<std::string>& script_names() {
			static utl::vector<std::string> names;
			return names;
		}
#endif // USE_WITH_EDITOR


		/// <summary>
		/// �жϽű�id�Ƿ���ڣ�
		/// ��Ϊ�ű�idʵ������entity_idͬ���ģ�
		/// ����ʹ��ӳ������script_id���������ڵĽű�ʵ����
		/// </summary>
		/// <param name="id">The identifier.</param>
		/// <returns></returns>
		[[nodiscard]]
		bool exist(script_id id) {
			assert(id::is_valid(id));
			const id::id_type index{ id::index(id) };
			// �������idӦ����generations�����С�����ޣ��Լ���Ӧ��ӳ���ȥ��entity_scripts��idҪС��entity_scripts����Ĵ�С
			assert(index < generations.size() && id_mapping[index] < entity_scripts.size());
			assert(generations[index] == id::generation(id));
			// �жϵķ�����������: 1.��֤����Ӧ����ͬ(Ӳ��Ҫ��) 2. �Ƿ���ӳ���entity_script 3. ӳ�䵽��entity_script�Ƿ����
			return (generations[index] == id::generation(id) && entity_scripts[id_mapping[index]] && entity_scripts[id_mapping[index]]->is_valid());
		}

	}//�����ռ�

	namespace detail {

		[[nodiscard]]
		u8 register_script(size_t tag, script_creator func) {
			bool result{ registery().insert(script_registry::value_type{tag, func}).second };
			assert(result);
			return result;
		}


		/// <summary>
		/// �����������ɵ�hash�ҵ���Ӧ�Ľű�creator
		/// </summary>
		/// <param name="tag">The tag.</param>
		/// <returns></returns>
		[[nodiscard]]
		script_creator get_script_creator(size_t tag) {
			// ��һ�����staticӳ��umap�����Ƿ�������tag
			auto script = primal::script::registery().find(tag);
			assert(script != primal::script::registery().end() && script->first == tag);
			return script->second;
		}

#ifdef USE_WITH_EDITOR

		/// <summary>
		/// ��static�����������������һ���ű�����
		/// </summary>
		/// <param name="name">The name.</param>
		/// <returns></returns>
		[[nodiscard]]
		u8 add_script_name(const char* name) {
			script_names().emplace_back(name);
			return true;
		}
#endif // USE_WITH_EDITOR

	}//detail�����ռ�

	[[nodiscard]]
	component create(init_info info, game_entity::entity entity) {
		assert(entity.is_valid());
		assert(info.script_creator);
		script_id id{};
		if (free_ids.size() > id::min_deleted_elements) {
			id = free_ids.front();
			//����id���ڲ�����
			//assert();
			free_ids.pop_front();
			id = script_id{ id::new_generation(id) };
			++generations[id::index(id)];
		}
		else {
			// �������ǵ�script_id���͵�id��Ϊ�µ�id_mapping�Ĵ�С[��Ϊfree_idsΪ���ˣ���id_mapping�����Ѿ���generations�Ķ�Ӧ���ˣ����������ڴ�СӦ��һ���ɣ���generations.size()Ӧ��Ҳ���]
			// ͬ���ģ���Ϊ���µģ�����generation������0����id_type����ǰ�漸��bitΪ0�����ǿ���ֱ�Ӵ���index�ͳ�ʼ����
			id = script_id{ static_cast<id::id_type>(id_mapping.size()) };
			id_mapping.emplace_back();
			generations.push_back(0);
		}

		// ���ǵ���script_idӦ���ǿ��õ�
		assert(id::is_valid(id));
		// ��ű�ʵ�����������һ���µ�ʵ��
		entity_scripts.emplace_back(info.script_creator(entity));
		assert(entity_scripts.back()->get_id() == entity.get_id());
		// entity_scripts���һ��Ԫ�ص�λ�ã�����ǰ���0��չ��id_type
		const id::id_type index{ static_cast<id::id_type>(entity_scripts.size()) - 1 };
		// ������Ӧӳ��� generation[i] => entity_scripts[j]
		id_mapping[id::index(id)] = index;
		return component{ id };
	}


	void remove(component c) {
		// ���Խű�ʵ�������ҿ���
		assert(c.is_valid() && exist(c.get_id()));
		const script_id id{ c.get_id() };
		// ��ȡentity_scripts���һ��Ԫ��ʵ����script_id
		const script_id last_id{ entity_scripts.back()->script().get_id() };
		// ��ȡscript_idָ����entity_scripts�ж�Ӧ��index
		const id::id_type index{ id_mapping[id::index(id)] };
		utl::erase_unordered(entity_scripts, index);
		// ��Ҫ����ԭ��entity_scriptsĩβԪ�صĶ�Ӧ��ϵ
		id_mapping[id::index(last_id)] = index;
		// ɾ�������ǽ���ɾ����script_id��Ӧ��ָ��Ƿ�
		id_mapping[id::index(id)] = id::invalid_id;
	}

	//
	void update(float dt) {
		for (auto& ptr : entity_scripts) {
			ptr->update(dt);
		}
	}

} // namespace primal::script


#ifdef USE_WITH_EDITOR
#include <atlsafe.h>

extern "C" __declspec(dllexport)
/// <summary>
/// ��ȡ�ű��������Ⱪ¶�Ľӿڣ�����ֵ����һ�ֽӿ�����ð���<atlsafe.h>
/// </summary>
/// <returns></returns>
[[nodiscard]]
LPSAFEARRAY get_script_names() {
	const u32 size{ (u32)primal::script::script_names().size() };
	if (!size) return nullptr;
	CComSafeArray<BSTR> names(size);
	for (u32 i{ 0 }; i < size; ++i) {
		names.SetAt(i, A2BSTR(primal::script::script_names()[i].c_str()), false);
	}
	return names.Detach();
}

#endif // USE_WITH_EDITOR
