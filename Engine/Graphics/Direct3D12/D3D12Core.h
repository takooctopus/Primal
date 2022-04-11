#pragma once
#include "D3D12CommonHeaders.h"
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
	/// ������Ⱦ����
	/// </summary>
	void render();


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
	ID3D12Device* const device();

	/// <summary>
	/// ��ǰframe��index
	/// </summary>
	[[nodiscard]]
	u32 current_frame_index();

	/// <summary>
	/// �趨��ǰframe���Ƴ�
	/// </summary>
	void set_deferred_releases_flag();
	
}