#include "GFX/DX11/D3D11_Sampler.h"

#include "Core/CommonDefinitions.h"
#include "GFX/DX11/D3D11_Utility.h"

namespace engi::gfx
{

	D3D11Sampler::D3D11Sampler(const std::string& name, D3D11Device* device, const GpuSamplerDesc& desc)
	{
		m_name = name;
		m_device = device;
		m_desc = desc;
	}

	void* D3D11Sampler::getHandle()
	{
		return m_handle.Get();
	}

	bool D3D11Sampler::initialize()
	{
		D3D11Device* device = (D3D11Device*)m_device;

		D3D11_SAMPLER_DESC desc;
		desc.Filter = detail::d3d11Filter(m_desc.filtering);
		switch (m_desc.addressing)
		{
		case ADDRESS_WRAP: 
			desc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		case ADDRESS_MIRROR: 
			desc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_MIRROR;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_MIRROR;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_MIRROR;
			break;
		case ADDRESS_CLAMP: 
			desc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case ADDRESS_BORDER: 
			desc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_BORDER;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_BORDER;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_BORDER;
			break;
		default: ENGI_ASSERT(false);
		}
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = (m_desc.filtering == GpuSamplerFiltering::FILTER_ANISOTROPIC) ? 8 : 0;
		desc.ComparisonFunc = detail::d3d11ComparisonFunc(m_desc.comparator);
		desc.BorderColor[0] = m_desc.border[0];
		desc.BorderColor[1] = m_desc.border[1];
		desc.BorderColor[2] = m_desc.border[2];
		desc.BorderColor[3] = m_desc.border[3];
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;
		HRESULT hr = device->getHandle()->CreateSamplerState(&desc, m_handle.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// TODO: Maybe somehow incapsulate this code in D3D11_Resource?
#if !defined(_NDEBUG)
		const std::string& name = this->getName();
		if (m_handle)
			m_handle->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str());
#endif

		return true;
	}

}; // engi::gfx namespace