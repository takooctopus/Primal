#include "PrimitiveMesh.h"
#include "Geometry.h"

namespace primal::tools {

	namespace {

		using namespace math;
		using namespace DirectX;

		using primitive_mesh_creator = void(*)(scene& scene, const primitive_init_info& info);

		void create_plane(scene& scene, const primitive_init_info& info);
		void create_cube(scene& scene, const primitive_init_info& info);
		void create_uv_sphere(scene& scene, const primitive_init_info& info);
		void create_ico_sphere(scene& scene, const primitive_init_info& info);
		void create_cylinder(scene& scene, const primitive_init_info& info);
		void create_capsule(scene& scene, const primitive_init_info& info);

		primitive_mesh_creator creators[]{
			create_plane,
			create_cube,
			create_uv_sphere,
			create_ico_sphere,
			create_cylinder,
			create_capsule,
		};

		static_assert(_countof(creators) == primitive_mesh_type::count);

		struct axis {
			enum : u32 {
				x = 0,
				y = 1,
				z = 2,

				count
			};
		};

		/// <summary>
		/// Creates the plane.
		/// </summary>
		/// <param name="info">The information.</param>
		/// <param name="horizontal_index">Index of the horizontal.</param>
		/// <param name="vertical_index">Index of the vertical.</param>
		/// <param name="flip_winding">�������ǲ�������׼���</param>
		/// <param name="offset">The offset.</param>
		/// <param name="u_range">The u range.</param>
		/// <param name="v_range">The v range.</param>
		/// <returns></returns>
		[[nodiscard]]
		mesh create_plane(const primitive_init_info& info,
			u32 horizontal_index = axis::x, u32 vertical_index = axis::z, bool flip_winding = false,
			v3 offset = { -0.5f, 0.f, -0.5f }, v2 u_range = { 0.f, 1.f }, v2 v_range = { 0.f, 1.f }) {
			assert(horizontal_index < axis::count&& vertical_index < axis::count);
			assert(horizontal_index != vertical_index);
			const u32 horizontal_count{ clamp(info.segments[horizontal_index], 1u, 10u) };	//ÿһ���ж��ٸ�����
			const u32 vertical_count{ clamp(info.segments[vertical_index], 1u, 10u) };	//ÿһ���ж��ٸ�����
			const f32 horizontal_step{ 1.f / horizontal_count };
			const f32 vertical_step{ 1.f / vertical_count };
			const f32 u_step{ (u_range.y - u_range.x) / horizontal_count };
			const f32 v_step{ (v_range.y - v_range.x) / vertical_count };

			mesh m{};
			utl::vector<v2> uvs;

			// ��һ�����Ƿָ��n����n+1������
			for (u32 j{ 0 }; j <= vertical_count; ++j) {
				for (u32 i{ 0 }; i <= horizontal_count; ++i) {
					v3 position{ offset };
					// ��һ��ָ��ָ����offset������position����[v3],�������Ƕ�position��������ݽ��и�ֵ
					f32* const as_array{ &position.x };
					// �����γ�grid���㣬������ָ�ľŸ����������
					as_array[horizontal_index] += i * horizontal_step;
					as_array[vertical_index] += j * vertical_step;
					// ��ÿ������λ�ø���primitive_init_info.size���v3�����x,y,z���зŴ�����ˣ�Ȼ���ӽ�mesh�Ķ��㼯����
					m.positions.emplace_back(position.x * info.size.x, position.y * info.size.y, position.z * info.size.z);

					v2 uv{ u_range.x, 1.f - v_range.x };
					uv.x += i * u_step;
					uv.y -= j * v_step;
					uvs.emplace_back(uv);
				}
			}

			assert(m.positions.size() == ((u64)horizontal_count + 1) * ((u64)vertical_count + 1));
			const u32 row_length{ horizontal_count + 1 }; // �ܹ�����ж�����
			for (u32 j{ 0 }; j < vertical_count; ++j) {
				u32 k{ 0 };
				for (u32 i{ k }; i < horizontal_count; ++i) {
					const u32 index[4]{
						i + j * row_length, //j��i�еĸ�����
						i + (j + 1) * row_length,
						(i + 1) + j * row_length,
						(i + 1) + (j + 1) * row_length,
					};
					// ��ӽ�ȥһ��������[�ĸ���(quat)�����Ͻ�������][Ҫ��flip_winding����˳��Ҫ��һ�£�˳������˷������ķ���]
					m.raw_indices.emplace_back(index[0]);
					m.raw_indices.emplace_back(index[flip_winding ? 2 : 1]);
					m.raw_indices.emplace_back(index[flip_winding ? 1 : 2]);

					// ��ӽ�ȥһ��������[�ĸ���(quat)�����½�������][Ҫ��flip_winding����˳��Ҫ��һ�£�˳������˷������ķ���]
					m.raw_indices.emplace_back(index[2]);
					m.raw_indices.emplace_back(index[flip_winding ? 3 : 1]);
					m.raw_indices.emplace_back(index[flip_winding ? 1 : 3]);
				}
				++k;
			}

			const u32 num_indices{ 3 * 2 * horizontal_count * vertical_count };	//�ܹ��ж��ٸ������ε�(�ܸ���grid�� * һ��quat2�������� * һ��triangle3������)
			assert(m.raw_indices.size() == num_indices);

			// uv��
			m.uv_sets.resize(1);
			for (u32 i{ 0 }; i < num_indices; ++i) {
				m.uv_sets[0].emplace_back(uvs[m.raw_indices[i]]);
			}

			return m;
		}


		/// <summary>
		/// ����uv��
		/// </summary>
		/// <param name="info"></param>
		/// <returns></returns>
		[[nodiscard]]
		mesh create_uv_sphere(const primitive_init_info& info) {
			const u32 theta_count{ clamp(info.segments[axis::y], 2u, 64u) }; //���϶��㵽�µ��߶�����[(������������)]��������-1��
			const u32 phi_count{ clamp(info.segments[axis::x],3u,64u) }; // ˮƽһȦ���߶����������ڵ�������
			const f32 theta_step{ pi / theta_count };	// ��ֱ����ļн�
			const f32 phi_step{ two_pi / phi_count };	// ˮƽ����н�

			const u32 num_vertices{ 2 + (theta_count - 1) * phi_count };	//�ܶ�����
			const u32 num_indices{ 2 * 3 * phi_count + 2 * 3 * phi_count * (theta_count - 2) }; // �������ζ����������ϲ�һȦ3n����������һȦ3n�����м���2*3n(m-2)����

			mesh m{};
			m.name = "uv_sphere";
			m.positions.resize(num_vertices);

			// ����϶���
			u32 c{ 0 };
			m.positions[c++] = { 0.f, info.size.y, 0.f };	// �϶���ֻ��y����ֵ

			// �м�Ķ���
			// �ӵ�һ�ſ�ʼ �� = �� => 2�� => 3�� => ... => pi-�� 
			for (u32 j{ 1 }; j <= (theta_count - 1); ++j) {
				const f32 theta{ j * theta_step };
				//ÿһ�Ŷ�Ҫ�� 0 => 2pi - ��
				for (u32 i{ 0 }; i < phi_count; ++i) {
					const f32 phi{ i * phi_step };
					m.positions[c++] = {
						info.size.x * XMScalarSin(theta) * XMScalarCos(phi),
						info.size.y * XMScalarCos(theta),
						-info.size.z * XMScalarSin(theta) * XMScalarSin(phi)
					}; // x, z, y ��������ϵ
				}
			}

			// �ײ�����
			m.positions[c++] = { 0.f, -info.size.z, 0.f };
			assert(c == num_vertices);

			//������ţ�������ռ�
			c = 0;
			m.raw_indices.resize(num_indices);
			utl::vector<v2> uvs(num_indices);
			const f32 inv_theta_count{ 1.f / theta_count };
			const f32 inv_phi_count{ 1.f / phi_count };

			// ��һ�������ε��������㡾��һ���Ǳ����㣬���������μӡ�
			for (u32 i{ 0 }; i < phi_count - 1; ++i) {
				uvs[c] = { (2 * i + 1) * 0.5f * inv_phi_count, 1.f };	//�������uv����(2i+1)
				m.raw_indices[c++] = 0;
				uvs[c] = { i * inv_phi_count, 1.f - inv_theta_count };
				m.raw_indices[c++] = i + 1;
				uvs[c] = { (i + 1) * inv_phi_count, 1.f - inv_theta_count };
				m.raw_indices[c++] = i + 2;
			}
			{
				uvs[c] = { 1.f - 0.5f * inv_phi_count, 1.f };	//�������uv����(2i+1)
				m.raw_indices[c++] = 0;
				uvs[c] = { 1.f - inv_phi_count, 1.f - inv_theta_count };
				m.raw_indices[c++] = phi_count;
				uvs[c] = { 1.f, 1.f - inv_theta_count };
				m.raw_indices[c++] = 1;
			}
			// �м伸��Ķ���
			for (u32 j{ 0 }; j < (theta_count - 2); ++j) {	//γ��
				for (u32 i{ 0 }; i < (phi_count - 1); ++i) {	//����
					const u32 index[4]{
						1 + i + j * phi_count,	// ���Ͻǵ�
						1 + i + (j + 1) * phi_count,	//���½ǵ�
						1 + (i + 1) + (j + 1) * phi_count, //���½ǵ�
						1 + (i + 1) + j * phi_count	//���Ͻǵ�
					};	//�Է�ʱ��˳����֯
					uvs[c] = { i * inv_phi_count, 1.f - (j + 1) * inv_theta_count };
					m.raw_indices[c++] = index[0];
					uvs[c] = { i * inv_phi_count, 1.f - (j + 2) * inv_theta_count };
					m.raw_indices[c++] = index[1];
					uvs[c] = { (i + 1) * inv_phi_count, 1.f - (j + 2) * inv_theta_count };
					m.raw_indices[c++] = index[2];

					uvs[c] = { i * inv_phi_count, 1.f - (j + 1) * inv_theta_count };
					m.raw_indices[c++] = index[0];
					uvs[c] = { (i + 1) * inv_phi_count, 1.f - (j + 2) * inv_theta_count };
					m.raw_indices[c++] = index[2];
					uvs[c] = { (i + 1) * inv_phi_count, 1.f - (j + 1) * inv_theta_count };
					m.raw_indices[c++] = index[3];
				}
				{
					const u32 index[4]{
						phi_count + j * phi_count,
						phi_count + (j + 1) * phi_count,
						1 + (j + 1) * phi_count,
						1 + j * phi_count
					};
					uvs[c] = { 1.f - inv_phi_count, 1.f - (j + 1) * inv_theta_count };
					m.raw_indices[c++] = index[0];
					uvs[c] = { 1.f - inv_phi_count, 1.f - (j + 2) * inv_theta_count };
					m.raw_indices[c++] = index[1];
					uvs[c] = { 1.f , 1.f - (j + 2) * inv_theta_count };
					m.raw_indices[c++] = index[2];

					uvs[c] = { 1.f - inv_phi_count, 1.f - (j + 1) * inv_theta_count };
					m.raw_indices[c++] = index[0];
					uvs[c] = { 1.f , 1.f - (j + 2) * inv_theta_count };
					m.raw_indices[c++] = index[2];
					uvs[c] = { 1.f , 1.f - (j + 1) * inv_theta_count };
					m.raw_indices[c++] = index[3];
				}
			}

			// �ϼ���
			const u32 south_pole_index{ (u32)m.positions.size() - 1 };
			for (u32 i{ 0 }; i < (phi_count - 1); ++i) {
				uvs[c] = { (2 * i + 1) * 0.5f * inv_phi_count, 0.f };
				m.raw_indices[c++] = south_pole_index;
				uvs[c] = { (i + 1) * inv_phi_count, inv_theta_count };
				m.raw_indices[c++] = south_pole_index - phi_count + i + 1;
				uvs[c] = { i * inv_phi_count, inv_theta_count };
				m.raw_indices[c++] = south_pole_index - phi_count + i;
			}
			{
				uvs[c] = { 1.f - 0.5f * inv_phi_count, 0.f };
				m.raw_indices[c++] = south_pole_index;
				uvs[c] = { 1.f , inv_theta_count };
				m.raw_indices[c++] = south_pole_index - phi_count;
				uvs[c] = { 1.f - inv_phi_count , inv_theta_count };
				m.raw_indices[c++] = south_pole_index - 1;
			}

			assert(c == num_indices);

			m.uv_sets.emplace_back(uvs);

			return m;
		}








		/// <summary>
		/// ����ƽ��
		/// </summary>
		/// <param name="scene"></param>
		/// <param name="info"></param>
		void create_plane(scene& scene, const primitive_init_info& info) {
			lod_group lod{};
			lod.name = "plane";
			lod.meshes.emplace_back(create_plane(info));
			scene.lod_groups.emplace_back(lod);
		}
		void create_cube(scene& scene, const primitive_init_info& info) {

		}

		/// <summary>
		/// ��������
		/// </summary>
		/// <param name="scene"></param>
		/// <param name="info"></param>
		void create_uv_sphere(scene& scene, const primitive_init_info& info) {
			lod_group lod{};
			lod.name = "uv_sphere";
			lod.meshes.emplace_back(create_uv_sphere(info));
			scene.lod_groups.emplace_back(lod);
		}
		void create_ico_sphere(scene& scene, const primitive_init_info& info) {

		}
		void create_cylinder(scene& scene, const primitive_init_info& info) {

		}
		void create_capsule(scene& scene, const primitive_init_info& info) {

		}


	}// ����namspace


	/// <summary>
	/// ����dll�ӿڣ����ݴ���ĳ�����Ϣ��ԭʼ��Ϣ���� ��״����(mesh)
	/// </summary>
	/// <param name="data">��������ָ��</param>
	/// <param name="info">The information.</param>
	EDITOR_INTERFACE
		void CreatePrimitiveMesh(scene_data* data, primitive_init_info* info) {
		assert(data && info);
		assert(info->type < primitive_mesh_type::count);
		scene scene{};

		creators[info->type](scene, *info);

		//TODO: ��ȡ������pack
		data->settings.calculate_normals = 1;
		process_scene(scene, data->settings);
		pack_data(scene, *data);
	}

} // namespace primal::tools