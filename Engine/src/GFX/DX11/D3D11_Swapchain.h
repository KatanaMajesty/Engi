#pragma once

#include "GFX/DX11/D3D11_API.h"
#include "GFX/GPUSwapchain.h"

namespace engi::gfx
{

	class D3D11Device;

	class D3D11Swapchain : public IGpuSwapchain
	{
	public:
		D3D11Swapchain(const std::string& name, D3D11Device* device, const GpuSwapchainDesc& desc);
		virtual ~D3D11Swapchain();

		virtual void* getHandle() override;
		virtual void present() override;
		virtual bool resize(uint32_t width, uint32_t height) override;
		virtual IGpuTexture* getBackbuffer() override;

		bool initialize();

	private:
		bool createBackbuffer();

	private:
		ComPtr<IDXGISwapChain1> m_handle = nullptr;
		bool m_hasVsync = false; // currently no support for vsync
		IGpuTexture* m_backbuffer = nullptr;
	}; // D3D11Swapchain class

}; // engi::gfx namespace