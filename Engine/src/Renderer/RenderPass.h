#pragma once

#include "GFX/Definitions.h"
#include "Renderer/Texture2D.h"
#include "Renderer/TextureCube.h"

namespace engi
{

	namespace gfx 
	{ 
		class IGpuTexture; 
	}

	class Buffer;

	struct RenderPass
	{
		RenderPass() = default;

		inline constexpr static uint32_t getNumSimultaneousRtvs() noexcept { return 8; }
		inline constexpr static uint32_t getNumSimultaneousUavs() noexcept { return 8; }

		void setDepthStencilBufferCube(TextureCube* depthStencilBuffer, uint32_t mipslice, uint32_t arrayslice, bool clearDepth, bool clearStencil) noexcept;
		void setDepthStencilBuffer2D(Texture2D* depthStencilBuffer, uint32_t mipslice, uint32_t arrayslice, bool clearDepth, bool clearStencil) noexcept;

		// slot - is the slot in 8-element array of render targets. When the RenderPass object is passed to the Renderer, actual RTV slots might be changed
		// For example, it the RTV array of the RenderPass is { rtv0, nullptr, rtv2, rtv3, nulltr } the final RTV array will be { rtv0, rtv2, rtv3 }
		// Thus, the slot of rtv2 will actually be 1, not 2.
		void setRenderTarget(uint32_t slot, gfx::IGpuTexture* buffer, uint32_t mipslice, uint32_t arrayslice, bool clearBuffer) noexcept;

		// slot - is the slot in 8-element array of unordered access views. When the RenderPass object is passed to the Renderer, actual UAV slots WILL be changed
		// For example, in PS if the RTV array contains 3 VALID rtvs and the UAV array looks like { uav0, nullptr, uav2 } the final slots of uav0 and uav2 will be 3 and 4 respectively
		// Thus the UAV slot in PS might be calculated as numValidRtvs + actualUavSlot
		void setUnorderedAccessView(uint32_t slot, Buffer* buffer) noexcept;

		void setViewport(uint32_t width, uint32_t height) noexcept;

		uint32_t viewportWidth = 0;
		uint32_t viewportHeight = 0;
		gfx::GpuRenderPassDesc desc{};
	};

};