#pragma once

#include "GFX/DX11/D3D11_API.h"
#include "GFX/DX11/D3D11_Device.h"
#include "GFX/GPUSampler.h"

namespace engi::gfx
{

	class D3D11Sampler : public IGpuSampler
	{
	public:
		D3D11Sampler(const std::string& name, D3D11Device* device, const GpuSamplerDesc& desc);
		virtual ~D3D11Sampler() = default;

		virtual void* getHandle() override;

		bool initialize();

	private:
		ComPtr<ID3D11SamplerState> m_handle;
	};

}; // engi::gfx namespace