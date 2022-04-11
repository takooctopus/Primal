#pragma once
#include "CommonHeaders.h"
#include "..\Platform\Window.h"
namespace primal::graphics {

	class surface {

	};

	/// <summary>
	/// �ṹ�� ��Ⱦƽ��
	/// </summary>
	struct render_surface {
		platform::window window{};	// ����ʵ��
		surface surface{};	// ƽ��ʵ��
	};

	/// <summary>
	/// ö�� ͼ��ƽ̨
	/// </summary>
	enum class graphics_platform {
		direct3d12 = 0,
		vulkan = 1,
		open_gl = 2,
	};

	/// <summary>
	/// ��ʼ��ͼ��ƽ̨����
	/// </summary>
	/// <param name="platform"></param>
	/// <returns></returns>
	[[nodiscard]]
	bool initialize(graphics_platform platform);
	
	/// <summary>
	/// �˳�ͼ��ƽ̨����
	/// </summary>
	void shutdown();
}