#pragma once

#include "GFX/DX11/D3D11_API.h"
#include "GFX/GPUDescriptor.h"

namespace engi::gfx
{

	class D3D11Device;

	class D3D11ShaderResourceView : public IGpuDescriptor 
	{
	public:
		D3D11ShaderResourceView(const std::string& name, D3D11Device* device, const GpuSrvDesc& desc);
		virtual ~D3D11ShaderResourceView() = default;
	
		bool init(IGpuResource* resource);
		virtual void* getHandle() override;

	private:
		friend class TextureLoader;

		GpuSrvDesc m_desc;
		ComPtr<ID3D11ShaderResourceView> m_handle = nullptr;
	};

	class D3D11UnorderedAccessView : public IGpuDescriptor
	{
	public:
		D3D11UnorderedAccessView(const std::string& name, D3D11Device* device, const GpuUavDesc& desc);
		virtual ~D3D11UnorderedAccessView() = default;

		bool init(IGpuResource* resource);
		virtual void* getHandle() override;

	private:
		GpuUavDesc m_desc;
		ComPtr<ID3D11UnorderedAccessView> m_handle = nullptr;
	};

}; // engi::gfx namespace