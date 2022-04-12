#pragma once
#include "D3D12CommonHeaders.h"

namespace primal::graphics::d3d12 {
	class descriptor_heap;
}

namespace primal::graphics::d3d12::core {

	/// <summary>
	/// ���ĳ�ʼ������
	/// </summary>
	[[nodiscard]]
	bool initialize();

	/// <summary>
	/// ���Ĺرպ���
	/// </summary>
	void shutdown();

	/// <summary>
	/// ģ���� �ж�null����Դ�ͷ�
	/// </summary>
	template <typename T>
	constexpr void release(T*& resource) {
		if (resource) {
			resource->Release();
			resource = nullptr;
		}
	}

	namespace detail {
		/// <summary>
		/// �Ƴ���Դ�ͷš��൱�ڼ�����С�
		/// </summary>
		void deferred_release(IUnknown* resource);
	}

	/// <summary>
	/// ģ���� �Ƴ���Դ�ͷš����ø��ײ�ĺ�����
	/// </summary>
	template <typename T>
	constexpr void deferred_release(T*& resource) {
		if (resource) {
			detail::deferred_release(resource);
			resource = nullptr;
		}
	}


	/// <summary>
	/// ����device
	/// </summary>
	[[nodiscard]]
	ID3D12Device8* const device();
	
	[[nodiscard]]
	descriptor_heap& rtv_heap();

	[[nodiscard]]
	descriptor_heap& dsv_heap();

	[[nodiscard]]
	descriptor_heap& srv_heap();

	[[nodiscard]]
	descriptor_heap& uav_heap();

	[[nodiscard]]
	DXGI_FORMAT	default_render_target_format();

	/// <summary>
	/// ��ǰframe��index
	/// </summary>
	[[nodiscard]]
	u32 current_frame_index();

	/// <summary>
	/// �趨��ǰframe���Ƴ�
	/// </summary>
	void set_deferred_releases_flag();

	surface create_surface(platform::window window);
	void remove_surface(surface_id id);
	void resize_surface(surface_id id, u32 width, u32 height);
	u32 surface_width(surface_id id);
	u32 surface_height(surface_id id);
	void render_surface(surface_id id);
	
}