#pragma once

#include <vector>
#include "GFX/DX11/D3D11_API.h"
#include "GFX/GPUTexture.h"

namespace engi::gfx
{

	class D3D11Device;

	class D3D11Texture : public IGpuTexture
	{
	public:
		D3D11Texture(const std::string& name, D3D11Device* device, const GpuTextureDesc& desc);
		virtual ~D3D11Texture() = default;

		bool init(const void* initialData);
		virtual void* getHandle() override;
		virtual void* getRTV(uint32_t mipSlice, uint32_t arraySlice) override;
		virtual void* getDSV(uint32_t mipSlice, uint32_t arraySlice) override;

	private:
		friend class D3D11Swapchain;
		friend class TextureLoader;

		ComPtr<ID3D11Resource> m_handle;
		std::vector<ComPtr<ID3D11RenderTargetView>> m_rtvs;
		std::vector<ComPtr<ID3D11DepthStencilView>> m_dsvs;
	}; // D3D11Texture class

}; // engi::gfx