#pragma once
#include "D3D12CommonHeaders.h"
namespace primal::graphics::d3d12 {
	struct d3d12_frame_info;
} // primal::graphics::d3d12

namespace primal::graphics::d3d12::gpass {
	bool initialize();
	void shutdown();

	/// <summary>
	/// 更改大小【每一帧都要调用这个方法】
	/// </summary>
	/// <param name="size"></param>
	void set_size(math::u32v2 size);

	void depth_prepass(id3d12_graphics_command_list* cmd_list, const d3d12_frame_info& info);
	void render(id3d12_graphics_command_list* cmd_list, const d3d12_frame_info& info);

	void add_transitions_for_depth_prepass(d3dx::d3d12_resource_barrier& barriers);
	void add_transitions_for_gpass(d3dx::d3d12_resource_barrier& barriers);
	void add_transitions_for_post_process(d3dx::d3d12_resource_barrier& barriers);
	
	void set_render_targets_for_depth_prepass(id3d12_graphics_command_list* cmd_list);
	void set_render_targets_for_gpass(id3d12_graphics_command_list* cmd_list);

}// primal::graphics::d3d12::gpass