#pragma once

#include "ToolsCommon.h"

namespace primal::tools {

	namespace {

	} //����namespace

	namespace packed_vertex {
		struct vertex_static {	// ��С����Ķ���ṹ�����մ����Ķ������ݡ�
			math::v3			position;
			u8					reserved[3];
			u8					t_sign;		//bit 0: tangent handedness * (tangent.z sign); bit 1: normal z sign [0=>-1, 1=>1]
			u16					normal[2];
			u16					tangent[2];
			math::v2			uv;
		};

	}//namespace packed_vertex

	struct vertex {	// ����
		math::v4	tangent{};	//����
		math::v3	position{};	//����(x,y,z)����
		math::v3	normal{};	//����[��׼����ά����]
		math::v2	uv{};		//uv����[��������]

	};

	struct mesh
	{
		//initial data
		utl::vector<math::v3>					positions;	//����(x,y,z)���꡾����Ϊ���������
		utl::vector<math::v3>					normals;	//����[��׼����ά����]������Ϊ3*�����θ�����
		utl::vector<math::v4>					tagents;	//����
		utl::vector<utl::vector<math::v2>>		uv_sets;	//uv����[��������]

		utl::vector<u32>						raw_indices;	//���������ε����ж�����positions�е����[3*�����θ���]

		// intermediate data �м��������
		utl::vector<vertex>						vertices;	//�������ϸ��Ϣ
		utl::vector<u32>						indices;	//���������ε����ж����Ӧ��vertices����

		// output data
		std::string								name;	// mesh����
		utl::vector<packed_vertex::vertex_static>	packed_vertices_static;
		f32										lod_threshold{ -1.f };
		u32										lod_id{ id::u32_invalid_id };
	};

	struct lod_group	//���ϸ��
	{
		std::string			name;	//���ϸ�ڵ�����
		utl::vector<mesh>	meshes;	//��������״��������
	};

	struct scene {	//����
		std::string					name;	//��������
		utl::vector<lod_group>		lod_groups;	//���ϸ������
	};

	struct geometry_import_settings	//���ε�������
	{
		f32 smoothing_angle; // ƽ���Ƕ�
		u8 calculate_normals;	// ���㷨��?(bool)
		u8 calculate_tangents;	//��������?(bool)
		u8 reverse_handedness;	// �����ֻ�����(bool)
		u8 import_embeded_textures; // ����Ƕ������?(bool)
		u8 import_animations;	// ���붯��?(bool)
	};

	
	/// <summary>
	/// ������Ϣ
	/// ����һ��buffer��ַָ�룬buffer�Ĵ�С�Լ�һ��geometry_import_settings���ε�������
	/// </summary>  
	struct scene_data
	{
		u8* buffer;	//ָ��buffer�׵�ַ��ָ��
		u32 buffer_size;	//buffer��С
		geometry_import_settings settings;	//���ε�������
	};

	/// <summary>
	/// ������ �����볡������ �ͼ��ε�������
	/// </summary>
	/// <param name="scene"></param>
	/// <param name="settings"></param>
	void process_scene(scene& scene, const geometry_import_settings& settings);

	/// <summary>
	/// �����������
	/// </summary>
	/// <param name="scene"></param>
	/// <param name="data"></param>
	void pack_data(const scene& scene, scene_data& data);
}//namespace primal::tools