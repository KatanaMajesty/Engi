#pragma once

#include "GFX/GPUDevice.h"

namespace engi::gfx
{

	class IImGuiContext
	{
	public:
		virtual ~IImGuiContext() = default;

		virtual bool initialize(void* handle, IGpuDevice* device) = 0;
		virtual void beginFrame() = 0;
		virtual void endFrame() = 0;
		virtual void deinitialize() = 0;
	};

}; // engi::gfx namespace