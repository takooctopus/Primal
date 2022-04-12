#pragma once
#include "CommonHeaders.h"
#include "Renderer.h"
#include "..\Platform\Window.h"

namespace primal::graphics {

	/// <summary>
	/// �ײ�ͼ��ƽ̨�ӿ�
	/// </summary>
	struct platform_interface
	{
		bool (*initialize)(void);	//����ָ�� ��ʼ��
		void (*shutdown)(void);		//����ָ�� ����
		void (*render)(void);	//����ָ�� ��Ⱦ

		struct 
		{
			surface(*create)(platform::window);
			void (*remove)(surface_id);
			void (*resize)(surface_id, u32, u32);
			u32 (*width)(surface_id);
			u32 (*height)(surface_id);
			void (*render)(surface_id);
		} surface;
	};
}

