#include "RenderPass.h"

#include "Core/CommonDefinitions.h"
#include "GFX/GPUSwapchain.h"
#include "GFX/GPUTexture.h"
#include "Buffer.h"

namespace engi
{

	void RenderPass::setDepthStencilBufferCube(TextureCube* depthStencilBuffer, uint32_t mipslice, uint32_t arrayslice, bool clearDepth, bool clearStencil) noexcept
	{
		desc.depthStencilBuffer = depthStencilBuffer ? depthStencilBuffer->getHandle() : nullptr;
		desc.depthMip = mipslice;
		desc.depthSlice = arrayslice;
		desc.useClearDepth = clearDepth;
		desc.useClearStencil = clearStencil;
	}

	void RenderPass::setDepthStencilBuffer2D(Texture2D* depthStencilBuffer, uint32_t mipslice, uint32_t arrayslice, bool clearDepth, bool clearStencil) noexcept
	{
		desc.depthStencilBuffer = depthStencilBuffer ? depthStencilBuffer->getHandle() : nullptr;
		desc.depthMip = mipslice;
		desc.depthSlice = arrayslice;
		desc.useClearDepth = clearDepth;
		desc.useClearStencil = clearStencil;
	}

	void RenderPass::setRenderTarget(uint32_t slot, gfx::IGpuTexture* buffer, uint32_t mipslice, uint32_t arrayslice, bool clearBuffer) noexcept
	{
		ENGI_ASSERT(buffer);
		ENGI_ASSERT(slot < RenderPass::getNumSimultaneousRtvs());

		gfx::GpuRenderTargetDesc& rtvDesc = desc.rtvs[slot];
		rtvDesc.rtv = buffer;
		rtvDesc.mipSlice = mipslice;
		rtvDesc.arraySlice = arrayslice;
		rtvDesc.useClearcolor = clearBuffer;
	}

	void RenderPass::setUnorderedAccessView(uint32_t slot, Buffer* buffer) noexcept
	{
		ENGI_ASSERT(buffer);
		ENGI_ASSERT(slot < RenderPass::getNumSimultaneousUavs());

		desc.uavs[slot] = buffer->getUav();
	}

	void RenderPass::setViewport(uint32_t width, uint32_t height) noexcept
	{
		ENGI_ASSERT(width > 0 && height > 0);

		this->viewportWidth = width;
		this->viewportHeight = height;
	}

}; // engi namespace