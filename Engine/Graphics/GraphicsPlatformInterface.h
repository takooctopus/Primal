#pragma once
#include "CommonHeaders.h"
#include "Renderer.h"

namespace primal::graphics {

	/// <summary>
	/// �ײ�ͼ��ƽ̨�ӿ�
	/// </summary>
	struct platform_interface
	{
		bool (*initialize)(void);	//����ָ�� ��ʼ��
		void (*shutdown)(void);		//����ָ�� ����
	};
}

