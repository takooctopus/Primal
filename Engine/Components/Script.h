#pragma once
#include "ComponentsCommon.h"

namespace primal::script {


	/// <summary>
	/// �����ű�ʵ���ĳ�ʼ����Ϣ�ṹ��
	/// </summary>
	struct init_info
	{

		/// <summary>
		/// ��Ҫ�ľ���һ���ű���������ָ��
		/// </summary>
		detail::script_creator script_creator;
	};

	[[nodiscard]]
	/// <summary>
	/// ���ݳ�ʼ����Ϣ������entity���������
	/// </summary>
	/// <param name="info"></param>
	/// <param name="entity"></param>
	/// <returns></returns>
	component create(init_info info, game_entity::entity entity);

	/// <summary>
	/// ɾ��������
	/// </summary>
	/// <param name="c"></param>
	void remove(component c);

	/// <summary>
	/// DEV ONLY:
	/// ��ʱ�Եĸ��º�����������ʱ�Ǳ������е�entity_scripts��ȫ��ִ��һ������ĸ��º���
	/// </summary>
	/// <param name="dt"></param>
	void update(float dt);
}