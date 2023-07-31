#include "GFX/GPU.h"

#include "GFX/DX11/D3D11_Device.h"
#include "GFX/DX11/DX11_ImGui.h"

namespace engi::gfx
{

	UniqueHandle<IGpuDevice> createDevice()
	{
		IGpuDevice* device = new D3D11Device();
		if (!((D3D11Device*)device)->initialize())
		{
			delete device;
			device = nullptr;
		}
		return makeUnique<IGpuDevice>(device);
	}

	UniqueHandle<IImGuiContext> createImGuiContext(void* handle, IGpuDevice* device)
	{
		IImGuiContext* context = new DX11ImGuiContext();
		if (!context->initialize(handle, device))
		{
			delete context;
			context = nullptr;
		}
		return UniqueHandle<IImGuiContext>(context);
	}

}; // engi::gfx namespace

