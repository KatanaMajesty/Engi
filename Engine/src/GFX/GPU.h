#pragma once

#include "Utility/Memory.h"
#include "GFX/GPUDevice.h"
#include "GFX/ImGui.h"

namespace engi::gfx
{
	UniqueHandle<IGpuDevice> createDevice();

	UniqueHandle<IImGuiContext> createImGuiContext(void* handle, IGpuDevice* device);
}; 