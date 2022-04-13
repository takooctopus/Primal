#include "ShaderCompilation.h"

#include "..\packages\DirectXShaderCompiler\inc\d3d12shader.h"
#include "..\packages\DirectXShaderCompiler\inc\dxcapi.h"

#pragma comment(lib, "../packages/DirectXShaderCompiler/lib/x64/dxcompiler.lib")


#include "..\Graphics\Direct3D12\D3D12Core.h"
#include "..\Graphics\Direct3D12\D3D12Shaders.h"
#include <filesystem>
#include <fstream>

using namespace primal;
using namespace primal::graphics::d3d12::shaders;
using namespace Microsoft::WRL;
namespace {
	/// <summary>
	/// 着色器文件的文件信息
	/// </summary>
	struct shader_file_info {
		const char*                 file;	//文件名
		const char*                 function;	//文件里面函数名称
		engine_shader::id           id;	//shader在d3d12shaders.h里定义文件枚举中的顺序
		shader_type::type           type;	// shader的类型【比方说顶点啊、像素啊】
	};

	constexpr shader_file_info shader_files[]{
		{"FullScreenTriangle.hlsl", "FullScreenTriangleVS", engine_shader::fullscreen_triangle_vs, shader_type::vertex},
		{"FillColor.hlsl", "FillColorPS", engine_shader::fill_color_ps, shader_type::pixel},
	};	// 要编译的着色器文件信息数组

	static_assert(_countof(shader_files) == engine_shader::count);

	constexpr const char* shader_source_path{ "../../Engine/Graphics/Direct3D12/Shaders/" };	//要编译的着色器文件的目录

	/// <summary>
	/// 转换到宽字符
	/// </summary>
	/// <param name="c"></param>
	/// <returns></returns>
	std::wstring to_wstring(const char* c) {
		std::string s{ c };
		return { s.begin(), s.end() };
	}

	/// <summary>
	/// 着色器编译类
	/// </summary>
	class shader_compiler {
	public:
		shader_compiler() {
			HRESULT hr{ S_OK };
			DXCall(hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler)));
			if (FAILED(hr)) return;
			DXCall(hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils)));
			if (FAILED(hr)) return;
			DXCall(hr = _utils->CreateDefaultIncludeHandler(&_include_handler));
			if (FAILED(hr)) return;
		}

		DISABLE_COPY_AND_MOVE(shader_compiler);

		/// <summary>
		/// 编译函数
		/// </summary>
		/// <param name="info"></param>
		/// <param name="full_path"></param>
		/// <returns></returns>
		IDxcBlob* compile(shader_file_info info, std::filesystem::path full_path) {
			assert(_compiler && _utils && _include_handler);
			HRESULT hr{ S_OK };

			ComPtr<IDxcBlobEncoding> source_blob{};	//使用util将文件装入source_blob
			DXCall(hr = _utils->LoadFile(full_path.c_str(), nullptr, &source_blob));
			if (FAILED(hr)) return nullptr;
			assert(source_blob && source_blob->GetBufferSize());

			std::wstring file{ to_wstring(info.file) };
			std::wstring func{ to_wstring(info.function) };
			std::wstring prof{ to_wstring(_profile_strings[(u32)info.type]) };

			// 填写调用DXC函数的参数
			LPCWSTR args[]{
				file.c_str(),				// 可选 shader source file name for error reporting
				L"-E", func.c_str(),		// 进入方法
				L"-T", prof.c_str(),		// Target profile
				DXC_ARG_ALL_RESOURCES_BOUND,
#if _DEBUG
				DXC_ARG_DEBUG,
				DXC_ARG_SKIP_OPTIMIZATIONS,
#else
				DXC_ARG_OPTIMIZATION_LEVEL3,
#endif // _DEBUG
				DXC_ARG_WARNINGS_ARE_ERRORS,
				L"-Qstrip_reflect",			// strip reflection into a seperate blob
				L"-Qstrip_debug"			// strip debug information into a seperate blob
			};

			OutputDebugStringA("Compiling ");
			OutputDebugStringA(info.file);
			OutputDebugStringA("\n");

			return compile(source_blob.Get(), args, _countof(args));
		}

		/// <summary>
		/// 根据源数据blob和调用参数进行编译
		/// </summary>
		/// <param name="source_blob"></param>
		/// <param name="args"></param>
		/// <param name="num_args"></param>
		/// <returns></returns>
		IDxcBlob* compile(IDxcBlobEncoding* source_blob, LPCWSTR* args, u32 num_args) {
			DxcBuffer buffer{};	//buffer
			buffer.Encoding = DXC_CP_ACP;	// 自动检测编码格式
			buffer.Ptr = source_blob->GetBufferPointer();
			buffer.Size = source_blob->GetBufferSize();

			HRESULT hr{ S_OK };
			ComPtr<IDxcResult> results{ nullptr };
			DXCall(_compiler->Compile(&buffer, args, num_args, _include_handler.Get(), IID_PPV_ARGS(&results)));	//获取编译结果
			if (FAILED(hr)) return nullptr;

			ComPtr<IDxcBlobUtf8> errors{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));	//获取输出的错误信息
			if (FAILED(hr)) return nullptr;

			if (errors && errors->GetStringLength()) {	//有错误输出错误
				OutputDebugStringA("Shader compilation error: \n");
				OutputDebugStringA(errors->GetStringPointer());
			}
			else {
				OutputDebugStringA(" [ Succeded ] \n");	//没有就成功
			}
			OutputDebugStringA("\n");

			HRESULT status{ S_OK };
			DXCall(hr = results->GetStatus(&status));
			if (FAILED(hr) || FAILED(status)) return nullptr;

			ComPtr<IDxcBlob> shader{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));
			if (FAILED(hr)) return nullptr;

			return shader.Detach();	//最后返回原始指针
		}
	private:
		constexpr static const char*		_profile_strings[]{ "vs_6_5",  "hs_6_5",  "ds_6_5",  "gs_6_5",  "ps_6_5",  "cs_6_5", "as_6_5",  "ms_6_5", };	//描述信息
		static_assert(_countof(_profile_strings) == shader_type::count);
		ComPtr<IDxcCompiler3>				_compiler{ nullptr };
		ComPtr<IDxcUtils>					_utils{ nullptr };
		ComPtr<IDxcIncludeHandler>			_include_handler{ nullptr };

	};

	/// <summary>
	/// 获取最终shaders编译好的bin文件输出位置
	/// </summary>
	/// <returns></returns>
	decltype(auto) get_engine_shaders_path() {
		return std::filesystem::path{ graphics::get_engine_shaders_path(graphics::graphics_platform::direct3d12) };
	}

	/// <summary>
	/// 判断是否最新，用last_write_time去比较
	/// </summary>
	/// <returns></returns>
	bool compile_shaders_are_up_to_date() {
		auto engine_shaders_path = get_engine_shaders_path();
		if (!std::filesystem::exists(engine_shaders_path))return false;
		auto shaders_compilation_time = std::filesystem::last_write_time(engine_shaders_path);
		std::filesystem::path path{};
		std::filesystem::path fullpath{};

		// 检查数组里有没有新的，只要有一个，就去编译
		for (u32 i{ 0 }; i < engine_shader::count; ++i) {
			auto& info = shader_files[i];
			path = shader_source_path;
			path += info.file;
			fullpath = path;
			if (!std::filesystem::exists(fullpath)) return false;

			auto shader_file_time = std::filesystem::last_write_time(fullpath);
			if (shader_file_time > shaders_compilation_time) return false;
		}
		return true;
	}
	/// <summary>
	/// 保存编译好的文件
	/// </summary>
	/// <param name="shaders"></param>
	/// <returns></returns>
	bool save_compiled_shaders(utl::vector<ComPtr<IDxcBlob>>& shaders) {
		auto engine_shaders_path = get_engine_shaders_path();
		std::filesystem::create_directories(engine_shaders_path.parent_path());	//创建文件夹
		std::ofstream file{ engine_shaders_path, std::ios::out | std::ios::binary };
		if (!file || !std::filesystem::exists(engine_shaders_path)) {
			file.close();
			return false;
		}
		for (auto& shader : shaders) {
			const D3D12_SHADER_BYTECODE byte_code{ shader->GetBufferPointer(), shader->GetBufferSize() };
			file.write((char*)&byte_code.BytecodeLength, sizeof(byte_code.BytecodeLength));
			file.write((char*)byte_code.pShaderBytecode, byte_code.BytecodeLength);
		}
		file.close();
		return true;
	}
}

/// <summary>
/// 【public】编译着色器
/// </summary>
/// <returns></returns>
bool compile_shaders()
{
	if (compile_shaders_are_up_to_date()) return true;
	utl::vector<ComPtr<IDxcBlob>> shaders;

	std::filesystem::path path{};
	std::filesystem::path full_path{};

	shader_compiler compiler{};
	// compile shaders
	// put all shaders in same buffer in the same order
	for (u32 i{ 0 }; i < engine_shader::count; ++i) {
		auto& info = shader_files[i];
		path = shader_source_path;
		path += info.file;
		full_path = path;
		if (!std::filesystem::exists(full_path)) return false;
		ComPtr<IDxcBlob> compiled_shader{ compiler.compile(info, full_path) };
		if (compiled_shader && compiled_shader->GetBufferPointer() && compiled_shader->GetBufferSize()) {
			shaders.emplace_back(std::move(compiled_shader));
		}
		else {
			return false;
		}
	}
	return save_compiled_shaders(shaders);
}
