#include "Script.h"
#include "Entity.h"

namespace primal::script {
	namespace {

		/// <summary>
		/// The generations代际数组，通过id_mapping来指向下面的entity_scripts中的脚本实例
		/// </summary>
		utl::vector<id::generation_type>		generations;

		/// <summary>
		/// 空闲scripts_id链表
		/// </summary>
		utl::deque<script_id>					free_ids;

		/// <summary>
		/// 智能指针数组，元素指向脚本实例
		/// 密集化的脚本实例数组
		/// </summary>
		utl::vector<detail::script_ptr>			entity_scripts;


		/// <summary>
		/// The identifier mapping [generations 到 entity_scripts 的映射]
		/// </summary>
		utl::vector<id::id_type>				id_mapping;

		/// <summary>
		/// 一个类型，映射，从tag=>script_creator
		/// </summary>
		using script_registry = utl::unordered_map<size_t, detail::script_creator>;

		/// <summary>
		/// 注册用户脚本用的函数，用来初始化脚本映射，因为我们知道static变量算是最先初始化的变量，所以我们要确定我们使用其之前已经被初始化
		/// </summary>
		/// <returns></returns>
		script_registry& registery() {
			/// <summary>
			/// 用户脚本注册映射unordered_map
			/// </summary>
			static script_registry reg;
			return reg;
		}

#ifdef USE_WITH_EDITOR

		/// <summary>
		/// 如果没有，初始化一个static的名字[string]向量，并返回实例
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		utl::vector<std::string>& script_names() {
			static utl::vector<std::string> names;
			return names;
		}
#endif // USE_WITH_EDITOR


		/// <summary>
		/// 判断脚本id是否存在，
		/// 因为脚本id实际上与entity_id同步的，
		/// 我们使用映射来让script_id与真正存在的脚本实例绑定
		/// </summary>
		/// <param name="id">The identifier.</param>
		/// <returns></returns>
		[[nodiscard]]
		bool exist(script_id id) {
			assert(id::is_valid(id));
			const id::id_type index{ id::index(id) };
			// 断言这个id应该在generations数组大小，别超限，以及对应的映射出去的entity_scripts的id要小于entity_scripts数组的大小
			assert(index < generations.size() && id_mapping[index] < entity_scripts.size());
			assert(generations[index] == id::generation(id));
			// 判断的返回条件有三: 1.保证代际应该相同(硬性要求) 2. 是否有映射的entity_script 3. 映射到的entity_script是否可用
			return (generations[index] == id::generation(id) && entity_scripts[id_mapping[index]] && entity_scripts[id_mapping[index]]->is_valid());
		}

	}//匿名空间

	namespace detail {

		[[nodiscard]]
		u8 register_script(size_t tag, script_creator func) {
			bool result{ registery().insert(script_registry::value_type{tag, func}).second };
			assert(result);
			return result;
		}


		/// <summary>
		/// 根据名称生成的hash找到对应的脚本creator
		/// </summary>
		/// <param name="tag">The tag.</param>
		/// <returns></returns>
		[[nodiscard]]
		script_creator get_script_creator(size_t tag) {
			// 看一看这个static映射umap里面是否包含这个tag
			auto script = primal::script::registery().find(tag);
			assert(script != primal::script::registery().end() && script->first == tag);
			return script->second;
		}

#ifdef USE_WITH_EDITOR

		/// <summary>
		/// 向static变量名称数组中添加一个脚本名称
		/// </summary>
		/// <param name="name">The name.</param>
		/// <returns></returns>
		[[nodiscard]]
		u8 add_script_name(const char* name) {
			script_names().emplace_back(name);
			return true;
		}
#endif // USE_WITH_EDITOR

	}//detail命名空间

	[[nodiscard]]
	component create(init_info info, game_entity::entity entity) {
		assert(entity.is_valid());
		assert(info.script_creator);
		script_id id{};
		if (free_ids.size() > id::min_deleted_elements) {
			id = free_ids.front();
			//断言id现在不存在
			//assert();
			free_ids.pop_front();
			id = script_id{ id::new_generation(id) };
			++generations[id::index(id)];
		}
		else {
			// 否则，我们的script_id类型的id就为新的id_mapping的大小[因为free_ids为空了，即id_mapping现在已经将generations的对应完了，他们俩现在大小应该一样吧，用generations.size()应该也差不多]
			// 同样的，因为是新的，所以generation现在是0，即id_type数据前面几个bit为0，我们可以直接传入index就初始化了
			id = script_id{ static_cast<id::id_type>(id_mapping.size()) };
			id_mapping.emplace_back();
			generations.push_back(0);
		}

		// 我们的新script_id应该是可用的
		assert(id::is_valid(id));
		// 向脚本实例数组中添加一个新的实例
		entity_scripts.emplace_back(info.script_creator(entity));
		assert(entity_scripts.back()->get_id() == entity.get_id());
		// entity_scripts最后一个元素的位置，将其前面加0拓展到id_type
		const id::id_type index{ static_cast<id::id_type>(entity_scripts.size()) - 1 };
		// 建立对应映射从 generation[i] => entity_scripts[j]
		id_mapping[id::index(id)] = index;
		return component{ id };
	}


	void remove(component c) {
		// 断言脚本实例存在且可用
		assert(c.is_valid() && exist(c.get_id()));
		const script_id id{ c.get_id() };
		// 获取entity_scripts最后一个元素实例的script_id
		const script_id last_id{ entity_scripts.back()->script().get_id() };
		// 获取script_id指向在entity_scripts中对应的index
		const id::id_type index{ id_mapping[id::index(id)] };
		utl::erase_unordered(entity_scripts, index);
		// 还要更新原来entity_scripts末尾元素的对应关系
		id_mapping[id::index(last_id)] = index;
		// 删除后我们将被删除的script_id对应的指向非法
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
/// 获取脚本名称向外暴露的接口，返回值得是一种接口数组得包含<atlsafe.h>
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
