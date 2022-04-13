#include "D3D12Shaders.h"
#include "..\Content\ContentLoader.h"

namespace primal::graphics::d3d12::shaders {
	namespace {

		/// <summary>
		/// ����õ���ɫ��
		/// </summary>
		typedef struct compiled_shader
		{
			u64			size;	//��ɫ����С
			const u8*	byte_code;	//�������ļ���ָ��
		}const * compiled_shader_ptr;

		compiled_shader_ptr engine_shaders[engine_shader::count]{};	// compiled_shaderָ������

		std::unique_ptr<u8[]> shaders_blob{};	//�������б���õ�������ɫ���������ļ����飬�õ�ʱ��ָ��ͺã��������֯������u64��size�ͺ��������ļ������������Ҫ�����ԭʼ����ת����compiled_shaderָ���

		/// <summary>
		/// ���ж�ȡshader.bin
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
				compiled_shader_ptr& shader{ engine_shaders[index] };	//��ȡ����Ԫ�����ã������޸�
				assert(!shader);
				result &= index < engine_shader::count && !shader;
				if (!result) break;
				shader = reinterpret_cast<const compiled_shader_ptr>(&shaders_blob[offset]);	//��ԭʼblobת���ɶ�Ӧָ��
				offset += sizeof(u64) + shader->size;	//ָ���ַƫ�����޸�
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

