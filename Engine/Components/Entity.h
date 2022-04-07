#pragma once
#include "ComponentsCommon.h"
namespace primal {

	/// <summary>
	/// ͨ������ʵ�ְ����������ŵ���ͬ�������ռ��У��ڲ�ͬ�ļ�����ͬ�����ռ���Ҫ�Ŷ�Ӧ��
	/// </summary>
#define INIT_INFO(component) namespace component { struct init_info; }
	INIT_INFO(transform);
	INIT_INFO(script);
#undef INIT_INFO

	namespace game_entity {

		/// <summary>
		/// ��Ϸʵ���ඨ�壺
		/// <value>transform������任��ʼ����Ϣ[ÿ��ʵ�嶼����ð�������任��]</value>
		/// <value>script���ű���ʼ����Ϣ</value>
		/// </summary>
		struct entity_info
		{
			transform::init_info* transform{ nullptr };
			script::init_info* script{ nullptr };
		};

		
		/// <summary>
		/// ��Ϸʵ��Ĵ�������
		/// </summary>
		/// <param name="info"></param>
		[[nodiscard]] 
		entity create(entity_info info);

		/// <summary>
		/// �������Ƴ���Ϸʵ�壬Ҫע���Ƴ�˳��Ҫ�����������˰����BUG��Ҫ���ͱ�����
		/// </summary>
		/// <param name="id"></param>
		void remove(entity_id id);

		 
		/// <summary>
		/// �ж���Ϸʵ���Ƿ��Ծ[��ʵ�����ж�generation��������ǲ���һ���ģ���һ��˵�����Ƴ���]
		/// </summary>
		/// <param name="id"></param>
		/// <returns></returns>
		[[nodiscard]]
		bool is_alive(entity_id id);
	}

}