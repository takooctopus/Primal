#pragma once

#include "ToolsCommon.h"

namespace primal::tools {

	namespace {

	} //匿名namespace

	namespace packed_vertex {
		struct vertex_static {
			math::v3			position;
			u8					reserved[3];
			u8					t_sign;		//bit 0: tangent handedness * (tangent.z sign); bit 1: normal z sign [0=>-1, 1=>1]
			u16					normal[2];
			u16					tangent[2];
			math::v2			uv;
		};

	}//namespace packed_vertex

	struct vertex {
		math::v4	tangent{};	//切线
		math::v3	position{};	//顶点(x,y,z)坐标
		math::v3	normal{};	//法线[标准化三维向量]
		math::v2	uv{};		//uv坐标[纹理坐标]

	};

	struct mesh
	{
		//initial data
		utl::vector<math::v3>					positions;	//顶点(x,y,z)坐标【长度为顶点个数】
		utl::vector<math::v3>					normals;	//法线[标准化三维向量]【长度为3*三角形个数】
		utl::vector<math::v4>					tagents;	//切线
		utl::vector<utl::vector<math::v2>>		uv_sets;	//uv坐标[纹理坐标]

		utl::vector<u32>						raw_indices;	//所有三角形的所有顶点在positions中的序号[3*三角形个数]

		// intermediate data 中间变量数组
		utl::vector<vertex>						vertices;	//顶点的详细信息
		utl::vector<u32>						indices;	//所有三角形的所有顶点对应的vertices索引

		// output data
		std::string								name;	// mesh名称
		utl::vector<packed_vertex::vertex_static>	packed_vertices_static;
		f32										lod_threshold{ -1.f };
		u32										lod_id{ id::u32_invalid_id };
	};

	struct lod_group
	{
		std::string			name;
		utl::vector<mesh>	meshes;
	};

	struct scene {
		std::string					name;
		utl::vector<lod_group>		lod_groups;
	};

	struct geometry_import_settings
	{
		f32 smoothing_angle; // 平滑角度
		u8 calculate_normals;	// 
		u8 calculate_tangents;	//
		u8 reverse_handedness;	// 左右手互换
		u8 import_embeded_textures;
		u8 import_animations;
	};

	
	/// <summary>
	/// 场景信息
	/// 包括一个buffer地址指针，buffer的大小以及一个geometry_import_settings几何导入设置
	/// </summary>
	struct scene_data
	{
		u8* buffer;
		u32 buffer_size;
		geometry_import_settings settings;
	};

	/// <summary>
	/// 处理场景 ，传入场景引用 和几何导入设置
	/// </summary>
	/// <param name="scene"></param>
	/// <param name="settings"></param>
	void process_scene(scene& scene, const geometry_import_settings& settings);

	/// <summary>
	/// 打包场景数据
	/// </summary>
	/// <param name="scene"></param>
	/// <param name="data"></param>
	void pack_data(const scene& scene, scene_data& data);
}//namespace primal::tools