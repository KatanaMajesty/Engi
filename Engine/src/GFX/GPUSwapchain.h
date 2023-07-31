#pragma once

#include "GFX/Definitions.h"
#include "GFX/GPUResource.h"

namespace engi::gfx
{

	class IGpuTexture;

	class IGpuSwapchain : public IGpuResource
	{
	public:
		IGpuSwapchain() = default;
		virtual ~IGpuSwapchain() = default;

		const GpuSwapchainDesc& getDesc() const { return m_desc; }

		virtual void present() = 0;
		virtual bool resize(uint32_t width, uint32_t height) = 0;
		virtual IGpuTexture* getBackbuffer() = 0;

	protected:
		GpuSwapchainDesc m_desc{};
	}; // IGpuSwapchain class

}; // engi::gfx namespace