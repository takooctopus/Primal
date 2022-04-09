#pragma once
#include "ToolsCommon.h"

namespace primal::tools {

	enum primitive_mesh_type : u32 {
		plane,
		cube,
		uv_sphere,
		ico_sphere,
		cylinder,
		capsule,

		count
	};

	struct primitive_init_info	//初始化原始数据信息结构体
	{
		primitive_mesh_type			type;	//初始化信息数据类型
		u32							segments[3]{ 1,1,1 };	
		math::v3					size{ 1,1,1 };	
		u32							lod{ 0 };	//lod的指针
	};



}//namespace primal::tools