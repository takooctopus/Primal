#include "Geometry.h"

namespace primal::tools {
	namespace {
		using namespace math;
		using namespace DirectX;
		/// <summary>
		/// 计算三角形的Normal[法线],说白了拿出俩向量[顶点指向三角形的另外俩顶点]v1和v2，我们计算它们的叉乘n = v1xv2，这个n就是我们要找的
		/// </summary>
		/// <param name="m">The m.</param>
		void recalculate_normals(mesh& m) {
			const u32 num_indices{ (u32)m.raw_indices.size() };//三角形所有顶点数量和
			m.normals.reserve(num_indices); // 重置发现数组的大小以匹配raw_indices数量[每个三角形三个顶点都存成一个法线(三角形共顶点也要算两次的)]

			for (u32 i{ 0 }; i < num_indices; ++i) {
				// 拿到三角形的三个顶点的序号
				const u32 i0{ m.raw_indices[i] };
				const u32 i1{ m.raw_indices[++i] };
				const u32 i2{ m.raw_indices[++i] };

				// 根据序号取得顶点，并将其转化成DirectX中的vector
				XMVECTOR v0{ XMLoadFloat3(&m.positions[i0]) };
				XMVECTOR v1{ XMLoadFloat3(&m.positions[i1]) };
				XMVECTOR v2{ XMLoadFloat3(&m.positions[i2]) };

				// 两个边向量
				XMVECTOR e0{ v1 - v0 };
				XMVECTOR e1{ v2 - v0 };

				//计算叉乘并归一化，就是法线了
				XMVECTOR n{ XMVector3Normalize(XMVector3Cross(e0,e1)) };

				//将法线向量存回mesh对象m的法线数组的对应点坐标序号位置
				XMStoreFloat3(&m.normals[i], n);

				//将整个三角形三个顶点都保存成一个法线[注意现在i已经自增了2了]
				m.normals[i - 1] = m.normals[i];
				m.normals[i - 2] = m.normals[i];
			}

		}

		/// <summary>
		/// 处理法线向量数组，看看我们该用软边[soft edge]还是硬边缘[hard edge]
		/// </summary>
		/// <param name="m">The m.</param>
		/// <param name="smoothing_angle">The smoothing angle.</param>
		void process_normals(mesh& m, f32 smoothing_angle) {
			// 使用cos来进行平滑分区，因为我们知道cos函数是从1到-1再到1，其相对于Pi(180°)是对称的，只要cos(alpha) < cos(theta[cos_alpha])，我们就认为这个是一个硬边【可以看到180°即pi左右的cos值最小，我们可以直接使用余弦值比较】
			const f32 cos_alpha{ XMScalarACos(pi - smoothing_angle * pi / 180.f) };	//因为余弦是轴对称的，将角度(0-2Pi)转化到[-Pi~Pi]之间
			const bool is_hard_edge{ XMScalarNearEqual(smoothing_angle, 180.f, epsilon) }; //180°为硬边
			const bool is_soft_edge{ XMScalarNearEqual(smoothing_angle, 0.f, epsilon) };	// 0°为软边
			const u32 num_indices{ (u32)m.raw_indices.size() }; // 三角形所有顶点对应序号
			const u32 num_vertices{ (u32)m.positions.size() };	// 顶点数量
			assert(num_indices && num_vertices);
			assert(num_indices == num_vertices * 3);

			m.indices.resize(num_indices);
			utl::vector<utl::vector<u32>> idx_ref(num_vertices); //为每个顶点都创建一个引用数组
			for (u32 i{ 0 }; i < num_indices; ++i) {
				idx_ref[m.raw_indices[i]].emplace_back(i); //遍历图，将三角形顶点在raw_indices中的索引添加到引用数组中
			}
			for (u32 i{ 0 }; i < num_vertices; ++i) {
				auto& refs{ idx_ref[i] };	//每个顶点的引用数组vector<u32>，装的是这个顶点出现在raw_indices中的索引【每在三角形中出现一次就有一个】
				u32 num_refs{ (u32)refs.size() };	//这个顶点出现在多少个三角形中
				//对每个顶点的引用数组进行遍历
				for (u32 j{ 0 }; j < num_refs; ++j) {
					m.indices[refs[j]] = (u32)m.vertices.size();	//refs[j]=>在raw_indices数组中的索引；m.indices数组保存的是三角形上顶点在vertices数组上的索引【这里存放的是新idx】
					vertex& v{ m.vertices.emplace_back() }; //m的vertices[顶点]数组添加一个空位置，返回一个指向新加入的那个空格位置的引用
					v.position = m.positions[m.raw_indices[refs[j]]]; //raw_indices中的索引 => positions的索引 => 点的具体参数

					XMVECTOR n1{ XMLoadFloat3(&m.normals[refs[j]]) }; //顶点 => 对应三角形 => 法线
					if (!is_hard_edge) {
						// 如果不是平滑角度不是180°【相当于不平滑】
						for (u32 k{ j + 1 }; k < num_refs; ++k) {
							// 新开一个循环，将当前法线和之后的法线，判断其中夹角
							f32 cos_theta{ 0.f };
							XMVECTOR n2{ XMLoadFloat3(&m.normals[refs[k]]) };
							if (!is_soft_edge) {
								// 如果平滑角度不是设置为0°，有余弦值
								// 计算 n = n1·n2 / (||n1|| * ||n2||)
								XMStoreFloat(&cos_theta, XMVector3Dot(n1, n2) * XMVector3ReciprocalLength(n1));
							}
							if (is_soft_edge || cos_theta >= cos_alpha) {
								// 如果平滑角度设为0° 或者两个法向向量夹角过大【你在图片上看到的像是锐角了】【法向夹角越接近于0°，说明其面越平行】
								// 这时我们就要进行平滑处理【取平均值】，这里相加就算一样的，因为之后也要进行归一化
								n1 += n2;
								// 因为进行了平滑，将后面三角形顶点对应的vertexs数组索引设置成前面的
								m.indices[refs[k]] = m.indices[refs[j]];
								refs.erase(refs.begin() + k);
								--num_refs;
								--k;
							}
						}
					}
					//最后保存对应点平滑后的法线回v，因为v是vertices数组元素的引用，相当于直接更新了数组
					XMStoreFloat3(&v.normal, XMVector3Normalize(n1));
				}
			}
		}

		/// <summary>
		/// 处理纹理矩阵
		/// </summary>
		/// <param name="m">The m.</param>
		void process_uvs(mesh& m) {
			utl::vector<vertex> old_vertices;
			old_vertices.swap(m.vertices);
			utl::vector<u32> old_indices(m.indices.size());
			old_indices.swap(m.indices);
			const u32 num_vertices{ (u32)old_vertices.size() };
			const u32 num_indices{ (u32)old_indices.size() };
			assert(num_vertices && num_indices);

			utl::vector<utl::vector<u32>> idx_ref(num_vertices);
			for (u32 i{ 0 }; i < num_indices; ++i) {
				idx_ref[old_indices[i]].emplace_back(i);
			}
			for (u32 i{ 0 }; i < num_indices; ++i) {
				auto& refs{ idx_ref[i] };	// &数组 每个顶点对应的raw_indice序号
				u32 num_refs{ (u32)refs.size() };
				for (u32 j{ 0 }; j < num_refs; ++j) {
					// 每遍历一个顶点对应的三角形顶点，就要在vertices数组中添加一个位置
					m.indices[refs[j]] = (u32)m.vertices.size();	//保存这个 indices[raw_indices的索引] 保存 vertices数组的索引 
					vertex& v{ old_vertices[old_indices[refs[j]]] }; //找到这个vertices数组的索引 的旧数据【vetex】
					v.uv = m.uv_sets[0][refs[j]];	//改变旧数据vetex中的uv
					m.vertices.emplace_back(v);

					for (u32 k{ j + 1 }; k < num_refs; ++k) {
						v2& uv1{ m.uv_sets[0][refs[k]] };	//每个顶点的 所有uv依次比较，要是相同就合并，并将indices中指向的位置合并【意味着vertices有很多多于数据】
						if (XMScalarNearEqual(v.uv.x, uv1.x, epsilon) &&
							XMScalarNearEqual(v.uv.y, uv1.y, epsilon)) {
							m.indices[refs[k]] = m.indices[refs[j]];
							refs.erase(refs.begin() + k);
							--num_refs;
							--k;
						}
					}
				}
			}
		}


		/// <summary>
		/// 对中间数组vertices进行打包，对每个顶点的数据，将其法线范围改到u16范围中，并得到其符号
		/// </summary>
		/// <param name="m">The m.</param>
		void pack_vertices_static(mesh& m) {
			const u32 num_vertices{ (u32)m.vertices.size() };
			assert(num_vertices);
			m.packed_vertices_static.reserve(num_vertices);

			for (u32 i{ 0 }; i < num_vertices; ++i) {
				vertex& v{ m.vertices[i] };
				const u8 signs{ (u8)((v.normal.z > 0.f) << 1) };	// 根据法线的z值确定其是向上还是向下的[signs正数为2，负数或者0为0]
				const u16 normal_x{ (u16)pack_float<16>(v.normal.x,-1.f,1.f) };
				const u16 normal_y{ (u16)pack_float<16>(v.normal.y,-1.f,1.f) };
				// TODO: pack tangents in sign and in x/y components
				m.packed_vertices_static
					.emplace_back(packed_vertex::vertex_static{
							v.position,
							{0,0,0},
							signs,
							{normal_x,normal_y},
							{},
							v.uv
						});
			}
		}

		/// <summary>
		/// 处理网图(mesh)三角形顶点
		/// </summary>
		/// <param name="m">The m.</param>
		/// <param name="settings">The settings.</param>
		void process_vertices(mesh& m, const geometry_import_settings& settings) {
			assert((m.raw_indices.size() % 3) == 0);
			// 如果导入设置中我们要计算法线或者法线数组为空，我们就要去计算法线
			if (settings.calculate_normals || m.normals.empty()) {
				recalculate_normals(m);
			}
			// 计算后根据平滑角平湖处理法线角度【硬角转软角】
			process_normals(m, settings.smoothing_angle);
			// 如果uv图为空，就去处理uv图
			if (!m.uv_sets.empty()) {
				process_uvs(m);
			}
			// 最后将其打包成稳定的vertex_static结构
			pack_vertices_static(m);
		}

		/// <summary>
/// 获取mesh的大小
/// </summary>
/// <param name="m">The m.</param>
/// <returns></returns>
		u64 get_mesh_size(const mesh& m) {
			const u64 num_vertices{ m.vertices.size() };
			const u64 vertex_buffer_size{ sizeof(packed_vertex::vertex_static) * num_vertices };
			const u64 index_size{ (num_vertices < (1 << 16)) ? sizeof(u16) : sizeof(u32) };	// 序号要的数据结构大小 2或者4[ushort 或者 uint]
			const u64 index_buffer_size{ index_size * m.indices.size() };	//保存序号的缓冲区大小[序号要的数据结构大小 * 序号数量]
			constexpr u64 su32{ sizeof(u32) };
			const u64 size{
				su32 + m.name.size()	// mesh name
				+ su32				// lod id
				+ su32				// vertex size
				+ su32				// number of vertices
				+ su32				// index size (16bit /32bit)
				+ su32				// number of indices
				+ sizeof(f32)			// lod threshold
				+ vertex_buffer_size	// room for vertex
				+ index_buffer_size	// room for indices
			};
			return size;
		}

		/// <summary>
		/// 获取场景实例的内存占用大小
		/// </summary>
		/// <param name="scene">The scene.</param>
		/// <returns></returns>
		u64 get_scene_size(const scene& scene) {
			constexpr u64 su32{ sizeof(u32) };	//u32数据大小
			u64 size{
				su32	//name size
				+ scene.name.size()	// scene name 具体占的空间
				+ su32				// lod数量
			};
			for (auto& lod : scene.lod_groups) {
				u64 lod_size{
					su32					// lod 名称的长度【4字节(即为4)】
					+ lod.name.size()		// lod name 具体占的空间
					+ su32				// lod 中 mesh 的数量
				};
				for (auto& m : lod.meshes) {
					lod_size += get_mesh_size(m);
				}
				size += lod_size;
			}
			return size;
		}

		/// <summary>
		/// 根据mesh和初始指针位置【buffer[at]】进行打包
		/// </summary>
		/// <param name="m">The m.</param>
		/// <param name="buffer">The buffer.</param>
		/// <param name="at">At.</param>
		void pack_mesh_data(const mesh& m, u8* const buffer, u64& at) {
			constexpr u64 su32{ sizeof(u32) };
			u32 s{ 0 };

			// mesh name
			s = (u32)m.name.size();
			memcpy(&buffer[at], &s, su32); at += su32;
			memcpy(&buffer[at], m.name.c_str(), s); at += s;

			// lod_id
			s = m.lod_id;
			memcpy(&buffer[at], &s, su32); at += su32;

			//vertex size
			constexpr u32 vertex_size{ sizeof(packed_vertex::vertex_static) };
			s = vertex_size;
			memcpy(&buffer[at], &s, su32); at += su32;

			// number of vertices
			const u32 num_vertices{ (u32)m.vertices.size() };
			s = num_vertices;
			memcpy(&buffer[at], &s, su32); at += su32;

			// index size (16bit / 32bit)
			const u32 index_size{ (num_vertices < (1 << 16)) ? sizeof(u16) : sizeof(u32) };
			s = index_size;
			memcpy(&buffer[at], &s, su32); at += su32;

			// number of indices
			const u32 num_indices{ (u32)m.indices.size() };
			s = num_indices;
			memcpy(&buffer[at], &s, su32); at += su32;

			// lod threshhold
			memcpy(&buffer[at], &m.lod_threshold, sizeof(f32)); at += sizeof(f32);

			// vertex data
			s = vertex_size * num_vertices;
			memcpy(&buffer[at], m.packed_vertices_static.data(), s); at += s;

			// indice data
			s = index_size * num_indices;
			void* data{ (void*)m.indices.data() };
			utl::vector<u16> indices;
			if (index_size == sizeof(u16)) {
				indices.resize(num_indices);
				for (u32 i{ 0 }; i < num_indices; ++i) {
					indices[i] = (u16)m.indices[i];
					data = (void*)indices.data();
				}
			}
			memcpy(&buffer[at], data, s); at += s;
		}
	}// 匿名 namespace

	void process_scene(scene& scene, const geometry_import_settings& settings)
	{
		for (auto&& lod : scene.lod_groups) {
			for (auto&& m : lod.meshes) {
				process_vertices(m, settings);
			}
		}
	}

	void pack_data(const scene& scene, scene_data& data)
	{
		constexpr u64 su32{ sizeof(u32) };	//uint32一共4Byte
		const u64 scene_size{ get_scene_size(scene) };	//拿到整个场景的大小
		data.buffer_size = (u32)scene_size;
		data.buffer = (u8*)CoTaskMemAlloc(scene_size);
		assert(data.buffer);

		u8* const buffer{ data.buffer }; //出一个指针指向固定区域 data.buffer
		u64 at{ 0 };
		u32 s{ 0 };

		//	scene name
		s = (u32)scene.name.size();
		memcpy(&buffer[at], &s, su32);	at += su32;
		memcpy(&buffer[at], scene.name.c_str(), s); at += s;

		// number of lods
		s = (u32)scene.lod_groups.size();
		memcpy(&buffer[at], &s, su32); at += su32;

		for (auto& lod : scene.lod_groups) {
			// lod name
			s = (u32)lod.name.size();
			memcpy(&buffer[at], &s, su32); at += su32;
			memcpy(&buffer[at], lod.name.c_str(), s); at += s;

			//number of meshes in this lod
			s = (u32)lod.meshes.size();
			memcpy(&buffer[at], &s, su32); at += su32;

			for (auto& m : lod.meshes) {
				pack_mesh_data(m, buffer, at);
			}
		}

	}
}//namespace primal::tools
