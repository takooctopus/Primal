#pragma once

#include "D3D12CommonHeaders.h"

namespace primal::graphics::d3d12::shaders {

	/// <summary>
	/// shader的所有类型
	/// </summary>
	struct shader_type {
		enum type : u32 {
			vertex = 0,	//顶点
			hull,	//外壳
			domain,	//域
			geometry,	//几何
			pixel,	//像素
			compute,	//计算
			amplification,	//放大
			mesh,	//网格

			count
		};
	};

	/// <summary>
	/// 当前引擎所拥有的shader文件【暂时这样写】
	/// </summary>
	struct engine_shader {
		enum id :u32 {
			fullscreen_triangle_vs = 0,
			fill_color_ps = 1,
			post_process_ps = 2,

			count
		};
	};

	bool initialize();
	void shutdown();

	D3D12_SHADER_BYTECODE get_engine_shader(engine_shader::id id);
}