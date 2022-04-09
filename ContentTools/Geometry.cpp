#include "Geometry.h"

namespace primal::tools {
	namespace {
		using namespace math;
		using namespace DirectX;
		/// <summary>
		/// ���������ε�Normal[����],˵�����ó�������[����ָ�������ε�����������]v1��v2�����Ǽ������ǵĲ��n = v1xv2�����n��������Ҫ�ҵ�
		/// </summary>
		/// <param name="m">The m.</param>
		void recalculate_normals(mesh& m) {
			const u32 num_indices{ (u32)m.raw_indices.size() };//���������ж���������
			m.normals.reserve(num_indices); // ���÷�������Ĵ�С��ƥ��raw_indices����[ÿ���������������㶼���һ������(�����ι�����ҲҪ�����ε�)]

			for (u32 i{ 0 }; i < num_indices; ++i) {
				// �õ������ε�������������
				const u32 i0{ m.raw_indices[i] };
				const u32 i1{ m.raw_indices[++i] };
				const u32 i2{ m.raw_indices[++i] };

				// �������ȡ�ö��㣬������ת����DirectX�е�vector
				XMVECTOR v0{ XMLoadFloat3(&m.positions[i0]) };
				XMVECTOR v1{ XMLoadFloat3(&m.positions[i1]) };
				XMVECTOR v2{ XMLoadFloat3(&m.positions[i2]) };

				// ����������
				XMVECTOR e0{ v1 - v0 };
				XMVECTOR e1{ v2 - v0 };

				//�����˲���һ�������Ƿ�����
				XMVECTOR n{ XMVector3Normalize(XMVector3Cross(e0,e1)) };

				//�������������mesh����m�ķ�������Ķ�Ӧ���������λ��
				XMStoreFloat3(&m.normals[i], n);

				//�������������������㶼�����һ������[ע������i�Ѿ�������2��]
				m.normals[i - 1] = m.normals[i];
				m.normals[i - 2] = m.normals[i];
			}

		}

		/// <summary>
		/// �������������飬�������Ǹ������[soft edge]����Ӳ��Ե[hard edge]
		/// </summary>
		/// <param name="m">The m.</param>
		/// <param name="smoothing_angle">The smoothing angle.</param>
		void process_normals(mesh& m, f32 smoothing_angle) {
			// ʹ��cos������ƽ����������Ϊ����֪��cos�����Ǵ�1��-1�ٵ�1���������Pi(180��)�ǶԳƵģ�ֻҪcos(alpha) < cos(theta[cos_alpha])�����Ǿ���Ϊ�����һ��Ӳ�ߡ����Կ���180�㼴pi���ҵ�cosֵ��С�����ǿ���ֱ��ʹ������ֵ�Ƚϡ�
			const f32 cos_alpha{ XMScalarACos(pi - smoothing_angle * pi / 180.f) };	//��Ϊ��������ԳƵģ����Ƕ�(0-2Pi)ת����[-Pi~Pi]֮��
			const bool is_hard_edge{ XMScalarNearEqual(smoothing_angle, 180.f, epsilon) }; //180��ΪӲ��
			const bool is_soft_edge{ XMScalarNearEqual(smoothing_angle, 0.f, epsilon) };	// 0��Ϊ���
			const u32 num_indices{ (u32)m.raw_indices.size() }; // ���������ж����Ӧ���
			const u32 num_vertices{ (u32)m.positions.size() };	// ��������
			assert(num_indices && num_vertices);
			assert(num_indices == num_vertices * 3);

			m.indices.resize(num_indices);
			utl::vector<utl::vector<u32>> idx_ref(num_vertices); //Ϊÿ�����㶼����һ����������
			for (u32 i{ 0 }; i < num_indices; ++i) {
				idx_ref[m.raw_indices[i]].emplace_back(i); //����ͼ���������ζ�����raw_indices�е�������ӵ�����������
			}
			for (u32 i{ 0 }; i < num_vertices; ++i) {
				auto& refs{ idx_ref[i] };	//ÿ���������������vector<u32>��װ����������������raw_indices�е�������ÿ���������г���һ�ξ���һ����
				u32 num_refs{ (u32)refs.size() };	//�����������ڶ��ٸ���������
				//��ÿ�����������������б���
				for (u32 j{ 0 }; j < num_refs; ++j) {
					m.indices[refs[j]] = (u32)m.vertices.size();	//refs[j]=>��raw_indices�����е�������m.indices���鱣������������϶�����vertices�����ϵ������������ŵ�����idx��
					vertex& v{ m.vertices.emplace_back() }; //m��vertices[����]�������һ����λ�ã�����һ��ָ���¼�����Ǹ��ո�λ�õ�����
					v.position = m.positions[m.raw_indices[refs[j]]]; //raw_indices�е����� => positions������ => ��ľ������

					XMVECTOR n1{ XMLoadFloat3(&m.normals[refs[j]]) }; //���� => ��Ӧ������ => ����
					if (!is_hard_edge) {
						// �������ƽ���ǶȲ���180�㡾�൱�ڲ�ƽ����
						for (u32 k{ j + 1 }; k < num_refs; ++k) {
							// �¿�һ��ѭ��������ǰ���ߺ�֮��ķ��ߣ��ж����мн�
							f32 cos_theta{ 0.f };
							XMVECTOR n2{ XMLoadFloat3(&m.normals[refs[k]]) };
							if (!is_soft_edge) {
								// ���ƽ���ǶȲ�������Ϊ0�㣬������ֵ
								// ���� n = n1��n2 / (||n1|| * ||n2||)
								XMStoreFloat(&cos_theta, XMVector3Dot(n1, n2) * XMVector3ReciprocalLength(n1));
							}
							if (is_soft_edge || cos_theta >= cos_alpha) {
								// ���ƽ���Ƕ���Ϊ0�� �����������������нǹ�������ͼƬ�Ͽ�������������ˡ�������н�Խ�ӽ���0�㣬˵������Խƽ�С�
								// ��ʱ���Ǿ�Ҫ����ƽ������ȡƽ��ֵ����������Ӿ���һ���ģ���Ϊ֮��ҲҪ���й�һ��
								n1 += n2;
								// ��Ϊ������ƽ���������������ζ����Ӧ��vertexs�����������ó�ǰ���
								m.indices[refs[k]] = m.indices[refs[j]];
								refs.erase(refs.begin() + k);
								--num_refs;
								--k;
							}
						}
					}
					//��󱣴��Ӧ��ƽ����ķ��߻�v����Ϊv��vertices����Ԫ�ص����ã��൱��ֱ�Ӹ���������
					XMStoreFloat3(&v.normal, XMVector3Normalize(n1));
				}
			}
		}

		/// <summary>
		/// �����������
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
				auto& refs{ idx_ref[i] };	// &���� ÿ�������Ӧ��raw_indice���
				u32 num_refs{ (u32)refs.size() };
				for (u32 j{ 0 }; j < num_refs; ++j) {
					// ÿ����һ�������Ӧ�������ζ��㣬��Ҫ��vertices���������һ��λ��
					m.indices[refs[j]] = (u32)m.vertices.size();	//������� indices[raw_indices������] ���� vertices��������� 
					vertex& v{ old_vertices[old_indices[refs[j]]] }; //�ҵ����vertices��������� �ľ����ݡ�vetex��
					v.uv = m.uv_sets[0][refs[j]];	//�ı������vetex�е�uv
					m.vertices.emplace_back(v);

					for (u32 k{ j + 1 }; k < num_refs; ++k) {
						v2& uv1{ m.uv_sets[0][refs[k]] };	//ÿ������� ����uv���αȽϣ�Ҫ����ͬ�ͺϲ�������indices��ָ���λ�úϲ�����ζ��vertices�кܶ�������ݡ�
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
		/// ���м�����vertices���д������ÿ����������ݣ����䷨�߷�Χ�ĵ�u16��Χ�У����õ������
		/// </summary>
		/// <param name="m">The m.</param>
		void pack_vertices_static(mesh& m) {
			const u32 num_vertices{ (u32)m.vertices.size() };
			assert(num_vertices);
			m.packed_vertices_static.reserve(num_vertices);

			for (u32 i{ 0 }; i < num_vertices; ++i) {
				vertex& v{ m.vertices[i] };
				const u8 signs{ (u8)((v.normal.z > 0.f) << 1) };	// ���ݷ��ߵ�zֵȷ���������ϻ������µ�[signs����Ϊ2����������0Ϊ0]
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
		/// ������ͼ(mesh)�����ζ���
		/// </summary>
		/// <param name="m">The m.</param>
		/// <param name="settings">The settings.</param>
		void process_vertices(mesh& m, const geometry_import_settings& settings) {
			assert((m.raw_indices.size() % 3) == 0);
			// �����������������Ҫ���㷨�߻��߷�������Ϊ�գ����Ǿ�Ҫȥ���㷨��
			if (settings.calculate_normals || m.normals.empty()) {
				recalculate_normals(m);
			}
			// ��������ƽ����ƽ�������߽Ƕȡ�Ӳ��ת��ǡ�
			process_normals(m, settings.smoothing_angle);
			// ���uvͼΪ�գ���ȥ����uvͼ
			if (!m.uv_sets.empty()) {
				process_uvs(m);
			}
			// ����������ȶ���vertex_static�ṹ
			pack_vertices_static(m);
		}

		/// <summary>
/// ��ȡmesh�Ĵ�С
/// </summary>
/// <param name="m">The m.</param>
/// <returns></returns>
		u64 get_mesh_size(const mesh& m) {
			const u64 num_vertices{ m.vertices.size() };
			const u64 vertex_buffer_size{ sizeof(packed_vertex::vertex_static) * num_vertices };
			const u64 index_size{ (num_vertices < (1 << 16)) ? sizeof(u16) : sizeof(u32) };	// ���Ҫ�����ݽṹ��С 2����4[ushort ���� uint]
			const u64 index_buffer_size{ index_size * m.indices.size() };	//������ŵĻ�������С[���Ҫ�����ݽṹ��С * �������]
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
		/// ��ȡ����ʵ�����ڴ�ռ�ô�С
		/// </summary>
		/// <param name="scene">The scene.</param>
		/// <returns></returns>
		u64 get_scene_size(const scene& scene) {
			constexpr u64 su32{ sizeof(u32) };	//u32���ݴ�С
			u64 size{
				su32	//name size
				+ scene.name.size()	// scene name ����ռ�Ŀռ�
				+ su32				// lod����
			};
			for (auto& lod : scene.lod_groups) {
				u64 lod_size{
					su32					// lod ���Ƶĳ��ȡ�4�ֽ�(��Ϊ4)��
					+ lod.name.size()		// lod name ����ռ�Ŀռ�
					+ su32				// lod �� mesh ������
				};
				for (auto& m : lod.meshes) {
					lod_size += get_mesh_size(m);
				}
				size += lod_size;
			}
			return size;
		}

		/// <summary>
		/// ����mesh�ͳ�ʼָ��λ�á�buffer[at]�����д��
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
	}// ���� namespace

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
		constexpr u64 su32{ sizeof(u32) };	//uint32һ��4Byte
		const u64 scene_size{ get_scene_size(scene) };	//�õ����������Ĵ�С
		data.buffer_size = (u32)scene_size;
		data.buffer = (u8*)CoTaskMemAlloc(scene_size);
		assert(data.buffer);

		u8* const buffer{ data.buffer }; //��һ��ָ��ָ��̶����� data.buffer
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
