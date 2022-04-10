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
		/// <param name="flip_winding">法向量是不是右手准则的</param>
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
			const u32 horizontal_count{ clamp(info.segments[horizontal_index], 1u, 10u) };	//每一行有多少个格子
			const u32 vertical_count{ clamp(info.segments[vertical_index], 1u, 10u) };	//每一列有多少个格子
			const f32 horizontal_step{ 1.f / horizontal_count };
			const f32 vertical_step{ 1.f / vertical_count };
			const f32 u_step{ (u_range.y - u_range.x) / horizontal_count };
			const f32 v_step{ (v_range.y - v_range.x) / vertical_count };

			mesh m{};
			utl::vector<v2> uvs;

			// 想一下我们分割成n段有n+1个顶点
			for (u32 j{ 0 }; j <= vertical_count; ++j) {
				for (u32 i{ 0 }; i <= horizontal_count; ++i) {
					v3 position{ offset };
					// 拿一个指针指向由offset创建的position数组[v3],方便我们对position里面的数据进行赋值
					f32* const as_array{ &position.x };
					// 依次形成grid顶点，想成田字格的九个顶点就是了
					as_array[horizontal_index] += i * horizontal_step;
					as_array[vertical_index] += j * vertical_step;
					// 将每个顶点位置根据primitive_init_info.size这个v3里面的x,y,z进行放大就是了，然后扔进mesh的顶点集合中
					m.positions.emplace_back(position.x * info.size.x, position.y * info.size.y, position.z * info.size.z);

					v2 uv{ u_range.x, 1.f - v_range.x };
					uv.x += i * u_step;
					uv.y -= j * v_step;
					uvs.emplace_back(uv);
				}
			}

			assert(m.positions.size() == ((u64)horizontal_count + 1) * ((u64)vertical_count + 1));
			const u32 row_length{ horizontal_count + 1 }; // 总共格点有多少列
			for (u32 j{ 0 }; j < vertical_count; ++j) {
				u32 k{ 0 };
				for (u32 i{ k }; i < horizontal_count; ++i) {
					const u32 index[4]{
						i + j * row_length, //j行i列的格点序号
						i + (j + 1) * row_length,
						(i + 1) + j * row_length,
						(i + 1) + (j + 1) * row_length,
					};
					// 添加进去一个三角形[四格格点(quat)的左上角三角形][要是flip_winding我们顺序要变一下，顺序决定了法向量的方向]
					m.raw_indices.emplace_back(index[0]);
					m.raw_indices.emplace_back(index[flip_winding ? 2 : 1]);
					m.raw_indices.emplace_back(index[flip_winding ? 1 : 2]);

					// 添加进去一个三角形[四格格点(quat)的右下角三角形][要是flip_winding我们顺序要变一下，顺序决定了法向量的方向]
					m.raw_indices.emplace_back(index[2]);
					m.raw_indices.emplace_back(index[flip_winding ? 3 : 1]);
					m.raw_indices.emplace_back(index[flip_winding ? 1 : 3]);
				}
				++k;
			}

			const u32 num_indices{ 3 * 2 * horizontal_count * vertical_count };	//总共有多少个三角形点(总格子grid数 * 一个quat2个三角形 * 一个triangle3个顶点)
			assert(m.raw_indices.size() == num_indices);

			// uv点
			m.uv_sets.resize(1);
			for (u32 i{ 0 }; i < num_indices; ++i) {
				m.uv_sets[0].emplace_back(uvs[m.raw_indices[i]]);
			}

			return m;
		}


		/// <summary>
		/// 创建uv球
		/// </summary>
		/// <param name="info"></param>
		/// <returns></returns>
		[[nodiscard]]
		mesh create_uv_sphere(const primitive_init_info& info) {
			const u32 theta_count{ clamp(info.segments[axis::y], 2u, 64u) }; //从上顶点到下的线段数量[(不包含两顶点)]【点数量-1】
			const u32 phi_count{ clamp(info.segments[axis::x],3u,64u) }; // 水平一圈的线段数量【等于点数量】
			const f32 theta_step{ pi / theta_count };	// 竖直两点的夹角
			const f32 phi_step{ two_pi / phi_count };	// 水平两点夹角

			const u32 num_vertices{ 2 + (theta_count - 1) * phi_count };	//总顶点数
			const u32 num_indices{ 2 * 3 * phi_count + 2 * 3 * phi_count * (theta_count - 2) }; // 总三角形顶点数【最上层一圈3n个，最下面一圈3n个，中间有2*3n(m-2)个】

			mesh m{};
			m.name = "uv_sphere";
			m.positions.resize(num_vertices);

			// 添加上顶点
			u32 c{ 0 };
			m.positions[c++] = { 0.f, info.size.y, 0.f };	// 上顶点只有y轴有值

			// 中间的顶点
			// 从第一排开始 θ = α => 2α => 3α => ... => pi-α 
			for (u32 j{ 1 }; j <= (theta_count - 1); ++j) {
				const f32 theta{ j * theta_step };
				//每一排都要从 0 => 2pi - β
				for (u32 i{ 0 }; i < phi_count; ++i) {
					const f32 phi{ i * phi_step };
					m.positions[c++] = {
						info.size.x * XMScalarSin(theta) * XMScalarCos(phi),
						info.size.y * XMScalarCos(theta),
						-info.size.z * XMScalarSin(theta) * XMScalarSin(phi)
					}; // x, z, y 右手坐标系
				}
			}

			// 底部顶点
			m.positions[c++] = { 0.f, -info.size.z, 0.f };
			assert(c == num_vertices);

			//重置序号，并分配空间
			c = 0;
			m.raw_indices.resize(num_indices);
			utl::vector<v2> uvs(num_indices);
			const f32 inv_theta_count{ 1.f / theta_count };
			const f32 inv_phi_count{ 1.f / phi_count };

			// 第一层三角形的三个顶点【第一个是北极点，后面俩依次加】
			for (u32 i{ 0 }; i < phi_count - 1; ++i) {
				uvs[c] = { (2 * i + 1) * 0.5f * inv_phi_count, 1.f };	//北极点的uv坐标(2i+1)
				m.raw_indices[c++] = 0;
				uvs[c] = { i * inv_phi_count, 1.f - inv_theta_count };
				m.raw_indices[c++] = i + 1;
				uvs[c] = { (i + 1) * inv_phi_count, 1.f - inv_theta_count };
				m.raw_indices[c++] = i + 2;
			}
			{
				uvs[c] = { 1.f - 0.5f * inv_phi_count, 1.f };	//北极点的uv坐标(2i+1)
				m.raw_indices[c++] = 0;
				uvs[c] = { 1.f - inv_phi_count, 1.f - inv_theta_count };
				m.raw_indices[c++] = phi_count;
				uvs[c] = { 1.f, 1.f - inv_theta_count };
				m.raw_indices[c++] = 1;
			}
			// 中间几层的顶点
			for (u32 j{ 0 }; j < (theta_count - 2); ++j) {	//纬度
				for (u32 i{ 0 }; i < (phi_count - 1); ++i) {	//经度
					const u32 index[4]{
						1 + i + j * phi_count,	// 左上角点
						1 + i + (j + 1) * phi_count,	//左下角点
						1 + (i + 1) + (j + 1) * phi_count, //右下角点
						1 + (i + 1) + j * phi_count	//右上角点
					};	//以反时钟顺序组织
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

			// 南极点
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
		/// 创建平面
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
		/// 创建纹理
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


	}// 匿名namspace


	/// <summary>
	/// 对外dll接口：根据传入的场景信息和原始信息创建 网状数据(mesh)
	/// </summary>
	/// <param name="data">场景数据指针</param>
	/// <param name="info">The information.</param>
	EDITOR_INTERFACE
		void CreatePrimitiveMesh(scene_data* data, primitive_init_info* info) {
		assert(data && info);
		assert(info->type < primitive_mesh_type::count);
		scene scene{};

		creators[info->type](scene, *info);

		//TODO: 获取场景和pack
		data->settings.calculate_normals = 1;
		process_scene(scene, data->settings);
		pack_data(scene, *data);
	}

} // namespace primal::tools