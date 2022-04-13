#include "D3D12Shaders.h"
#include "..\Content\ContentLoader.h"

namespace primal::graphics::d3d12::shaders {
	namespace {

		/// <summary>
		/// 编译好的着色器
		/// </summary>
		typedef struct compiled_shader
		{
			u64			size;	//着色器大小
			const u8*	byte_code;	//二进制文件的指针
		}const * compiled_shader_ptr;

		compiled_shader_ptr engine_shaders[engine_shader::count]{};	// compiled_shader指针数组

		std::unique_ptr<u8[]> shaders_blob{};	//包含所有编译好的引擎着色器二进制文件数组，用的时候当指针就好，里面的组织类型是u64的size和后面数据文件，最后我们是要把这个原始数据转化成compiled_shader指针的

		/// <summary>
		/// 串行读取shader.bin
		/// </summary>
		/// <returns></returns>
		bool load_engine_shaders() {
			assert(!shaders_blob);
			u64 size{ 0 };
			bool result{ content::load_engine_shaders(shaders_blob, size) };

			assert(shaders_blob && size);

			u64 offset{ 0 };
			u32 index{ 0 };
			while (offset < size && result) {
				assert(index < engine_shader::count);
				compiled_shader_ptr& shader{ engine_shaders[index] };	//获取数组元素引用，方便修改
				assert(!shader);
				result &= index < engine_shader::count && !shader;
				if (!result) break;
				shader = reinterpret_cast<const compiled_shader_ptr>(&shaders_blob[offset]);	//将原始blob转化成对应指针
				offset += sizeof(u64) + shader->size;	//指针地址偏移量修改
				++index;
			}
			assert(offset == size && index == engine_shader::count);

			return result;
		}


	}//anonymous namespace






	bool initialize()
	{
		return load_engine_shaders();
	}

	void shutdown()
	{
		for (u32 i{ 0 }; i < engine_shader::count; ++i) {
			engine_shaders[i] = {};
		}
		shaders_blob.reset();
	}

	D3D12_SHADER_BYTECODE get_engine_shader(engine_shader::id id)
	{
		assert(id < engine_shader::count);
		const compiled_shader_ptr shader{ engine_shaders[id] };
		assert(shader && shader->size);
		return { &shader->byte_code, shader->size };
	}

}

