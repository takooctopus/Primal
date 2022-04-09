#include "PrimitiveMesh.h"
#include "Geometry.h"

namespace primal::tools {

	namespace {

		using namespace math;
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
					uv.y += i * v_step;
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

		void create_plane(scene& scene, const primitive_init_info& info) {
			lod_group lod{};
			lod.name = "plane";
			lod.meshes.emplace_back(create_plane(info));
			scene.lod_groups.emplace_back(lod);
		}
		void create_cube(scene& scene, const primitive_init_info& info) {

		}
		void create_uv_sphere(scene& scene, const primitive_init_info& info) {

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