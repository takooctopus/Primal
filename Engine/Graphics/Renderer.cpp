#include "Renderer.h"
#include "GraphicsPlatformInterface.h"
#include "Direct3D12\D3D12Interface.h"

namespace primal::graphics {
	
	namespace {
		platform_interface gfx{};	//底层图形平台接口

		/// <summary>
		/// static 初始化图形平台
		/// </summary>
		/// <param name="platform"></param>
		/// <returns></returns>
		[[nodiscard]]
		bool set_platform_interface(graphics_platform platform) {
			switch (platform)
			{
			case graphics_platform::direct3d12:
				d3d12::get_platform_interface(gfx);
				break;
			default:
				return false;
				break;
			}
			return true;
		}


	}//anonymous namespace

	[[nodiscard]]
	bool initialize(graphics_platform platform) {
		return set_platform_interface(platform) && gfx.initialize();
	}

	void shutdown() {
		gfx.shutdown();
	}


	surface create_surface(platform::window window)
	{
		return gfx.surface.create(window);
	}

	void remove_surface(surface_id id)
	{
		assert(id::is_valid(id));
		gfx.surface.remove(id);
	}




	void surface::resize(u32 width, u32 height) const
	{
		assert(id::is_valid(_id));
		gfx.surface.resize(_id, width, height);
	}

	u32 surface::witdh() const
	{
		assert(id::is_valid(_id));
		return gfx.surface.width(_id);
	}

	u32 surface::height() const
	{
		assert(id::is_valid(_id));
		return gfx.surface.height(_id);
	}

	void surface::render() const
	{
		assert(id::is_valid(_id));
		gfx.surface.render(_id);
	}


}// primal::graphics