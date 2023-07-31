#include "GFX/DX11/D3D11_Descriptor.h"

#include "Core/CommonDefinitions.h"
#include "GFX/DX11/D3D11_Utility.h"
#include "GFX/DX11/D3D11_Device.h"
#include "GFX/DX11/D3D11_Buffer.h"
#include "GFX/DX11/D3D11_Texture.h"

namespace engi::gfx
{

	D3D11ShaderResourceView::D3D11ShaderResourceView(const std::string& name, D3D11Device* device, const GpuSrvDesc& desc)
	{
		m_name = name;
		m_device = device;
		m_desc = desc;
	}

	bool D3D11ShaderResourceView::init(IGpuResource* resource)
	{
		if (!resource)
			return false;

		D3D11Device* device = (D3D11Device*)m_device;
		ID3D11Device* d3dDevice = device->getHandle();
		
		if (m_desc.type == GpuResourceType::RESOURCE_UNKNOWN)
		{
			HRESULT hr = d3dDevice->CreateShaderResourceView((ID3D11Resource*)resource->getHandle(), nullptr, m_handle.ReleaseAndGetAddressOf());
			return SUCCEEDED(hr);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ID3D11Resource* d3dResource = nullptr;
		switch (m_desc.type)
		{
		case GpuResourceType::RESOURCE_BUFFER:
		{
			D3D11Buffer* r = static_cast<D3D11Buffer*>(resource);
			if (!r->getHandle())
				return false;

			const GpuBufferDesc& rDesc = r->getDesc();
			if ((rDesc.otherFlags & GpuMisc::MISC_STRUCTURED_BUFFER) != 0)
			{
				// This is only true if the buffer is StructuredBuffer
				ENGI_ASSERT(rDesc.byteStride != 0);
				ENGI_ASSERT(m_desc.buffer.offset % rDesc.byteStride == 0);
				ENGI_ASSERT(m_desc.buffer.size % rDesc.byteStride == 0);
			}
			
			srvDesc.Format = detail::d3d11Format(m_desc.format);
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = m_desc.buffer.offset / rDesc.byteStride;
			srvDesc.Buffer.NumElements = m_desc.buffer.size / rDesc.byteStride;
			d3dResource = (ID3D11Resource*)r->getHandle();
		}; break;
		case GpuResourceType::RESOURCE_TEXTURE2D:
		{
			D3D11Texture* r = static_cast<D3D11Texture*>(resource);
			if (!r->getHandle())
				return false;

			const GpuTextureDesc& rDesc = r->getDesc();
			ENGI_ASSERT(rDesc.type == GpuTextureType::TEXTURE2D);
			ENGI_ASSERT(rDesc.miplevels > 0 && rDesc.miplevels >= m_desc.texture.mipLevels);
			ENGI_ASSERT(m_desc.texture.mipSlice < rDesc.miplevels);

			srvDesc.Format = detail::d3d11Format(m_desc.format);
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = m_desc.texture.mipLevels;
			srvDesc.Texture2D.MostDetailedMip = m_desc.texture.mipSlice;
			d3dResource = (ID3D11Resource*)r->getHandle();
		}; break;
		case GpuResourceType::RESOURCE_TEXTURE2D_ARRAY:
		{
			D3D11Texture* r = static_cast<D3D11Texture*>(resource);
			if (!r->getHandle())
				return false;

			const GpuTextureDesc& rDesc = r->getDesc();
			ENGI_ASSERT(rDesc.type == GpuTextureType::TEXTURE2D);
			ENGI_ASSERT(rDesc.arraySize > 0 && rDesc.arraySize >= m_desc.texture.arraySize);
			ENGI_ASSERT(rDesc.miplevels > 0 && rDesc.miplevels >= m_desc.texture.mipLevels);
			ENGI_ASSERT(m_desc.texture.mipSlice < rDesc.miplevels);
			ENGI_ASSERT(m_desc.texture.arraySize + m_desc.texture.firstArraySlice <= rDesc.arraySize);

			srvDesc.Format = detail::d3d11Format(m_desc.format);
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.MipLevels = m_desc.texture.mipLevels;
			srvDesc.Texture2DArray.MostDetailedMip = m_desc.texture.mipSlice;
			srvDesc.Texture2DArray.ArraySize = m_desc.texture.arraySize;
			srvDesc.Texture2DArray.FirstArraySlice = m_desc.texture.firstArraySlice;
			d3dResource = (ID3D11Resource*)r->getHandle();
		}; break;
		case GpuResourceType::RESOURCE_TEXTURECUBE:
		{
			D3D11Texture* r = static_cast<D3D11Texture*>(resource);
			if (!r->getHandle())
				return false;

			const GpuTextureDesc& rDesc = r->getDesc();
			ENGI_ASSERT(rDesc.type == GpuTextureType::TEXTURE2D);
			ENGI_ASSERT(rDesc.miplevels > 0 && rDesc.miplevels >= m_desc.texture.mipLevels);
			ENGI_ASSERT(rDesc.arraySize == 6);
			ENGI_ASSERT(m_desc.texture.mipSlice < rDesc.miplevels);

			srvDesc.Format = detail::d3d11Format(m_desc.format);
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = m_desc.texture.mipLevels;
			srvDesc.TextureCube.MostDetailedMip = m_desc.texture.mipSlice;
			d3dResource = (ID3D11Resource*)r->getHandle();
		}; break;
		case GpuResourceType::RESOURCE_TEXTURECUBE_ARRAY:
		{
			D3D11Texture* r = static_cast<D3D11Texture*>(resource);
			if (!r->getHandle())
				return false;

			const GpuTextureDesc& rDesc = r->getDesc();
			ENGI_ASSERT(rDesc.type == GpuTextureType::TEXTURE2D);
			ENGI_ASSERT(rDesc.miplevels > 0 && rDesc.miplevels >= m_desc.texture.mipLevels);
			// check if arraysize is valid (just in case)
			// although, we calculate the arraysize internally in TextureCube
			ENGI_ASSERT(rDesc.arraySize % 6 == 0); 
			ENGI_ASSERT(m_desc.texture.mipSlice < rDesc.miplevels);

			uint32_t numCubes = rDesc.arraySize / 6;
			srvDesc.Format = detail::d3d11Format(m_desc.format);
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
			srvDesc.TextureCubeArray.MipLevels = m_desc.texture.mipLevels;
			srvDesc.TextureCubeArray.MostDetailedMip = m_desc.texture.mipSlice;
			srvDesc.TextureCubeArray.First2DArrayFace = 0;
			srvDesc.TextureCubeArray.NumCubes = numCubes;
			d3dResource = (ID3D11Resource*)r->getHandle();
		}; break;
		default: ENGI_ASSERT(false); return false;
		}

		// This check is performed in default bracket of switch-case
		// if (!d3dResource)
		// 	return false;

		HRESULT hr = d3dDevice->CreateShaderResourceView(d3dResource, &srvDesc, m_handle.ReleaseAndGetAddressOf());
		if (FAILED(hr))
			return false;

		// TODO: Maybe somehow incapsulate this code in D3D11_Resource?
#if !defined(_NDEBUG)
		const std::string& name = this->getName();
		if (m_handle)
			m_handle->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str());
#endif

		return true;
	}

	void* D3D11ShaderResourceView::getHandle()
	{
		return m_handle.Get();
	}

	D3D11UnorderedAccessView::D3D11UnorderedAccessView(const std::string& name, D3D11Device* device, const GpuUavDesc& desc)
	{
		m_name = name;
		m_device = device;
		m_desc = desc;
	}

	bool D3D11UnorderedAccessView::init(IGpuResource* resource)
	{
		if (!resource)
			return false;

		D3D11Device* device = (D3D11Device*)m_device;
		ID3D11Device* d3dDevice = device->getHandle();

		ID3D11Resource* d3dResource = (ID3D11Resource*)resource->getHandle();
		if (!d3dResource)
			return false;

		if (m_desc.type == GpuResourceType::RESOURCE_UNKNOWN)
		{
			HRESULT hr = d3dDevice->CreateUnorderedAccessView(d3dResource, nullptr, m_handle.ReleaseAndGetAddressOf());
			return SUCCEEDED(hr);
		}

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = detail::d3d11Format(m_desc.format);

		switch (m_desc.type)
		{
		case GpuResourceType::RESOURCE_BUFFER:
		{
			D3D11Buffer* buffer = (D3D11Buffer*)resource;
			
			const GpuBufferDesc& rDesc = buffer->getDesc();
			if ((rDesc.otherFlags & GpuMisc::MISC_STRUCTURED_BUFFER) != 0)
			{
				// This is only true if the buffer is StructuredBuffer
				uint32_t byteOffset = m_desc.buffer.firstElement * 8;
				uint32_t uavBytes = m_desc.buffer.numElements * 8;
				ENGI_ASSERT(rDesc.byteStride != 0);
				ENGI_ASSERT(uavBytes + byteOffset <= rDesc.bytes);

				// UAV format must be UNKNOWN when creating a structured buffer
				ENGI_ASSERT(m_desc.format == GpuFormat::FORMAT_UNKNOWN);
			}

			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = m_desc.buffer.firstElement;
			uavDesc.Buffer.NumElements = m_desc.buffer.numElements;

			// If we want to use BYTEADDRESS buffer we need to specify a specific flag in UAV for that
			uavDesc.Buffer.Flags = 0;
			if ((rDesc.otherFlags & GpuMisc::MISC_BYTEADDRESS_BUFFER) != 0)
			{
				// D3D11 Api requires The UAV format bound to this resource to be created with the DXGI_FORMAT_R32_TYPELESS format.
				ENGI_ASSERT(m_desc.format == GpuFormat::R32T);
				uavDesc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
			}
		}; break;
		case GpuResourceType::RESOURCE_TEXTURE2D:
		{
			D3D11Texture* texture = (D3D11Texture*)resource;

			const GpuTextureDesc& rDesc = texture->getDesc();
			ENGI_ASSERT(rDesc.type == GpuTextureType::TEXTURE2D);
			ENGI_ASSERT(m_desc.texture.mipSlice < rDesc.miplevels);

			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = m_desc.texture.mipSlice;
		}; break;
		case GpuResourceType::RESOURCE_TEXTURE2D_ARRAY:
		{
			D3D11Texture* texture = (D3D11Texture*)resource;

			const GpuTextureDesc& rDesc = texture->getDesc();
			ENGI_ASSERT(rDesc.type == GpuTextureType::TEXTURE2D);
			ENGI_ASSERT(m_desc.texture.mipSlice < rDesc.miplevels);
			ENGI_ASSERT(m_desc.texture.firstArraySlice + m_desc.texture.arraySize <= rDesc.arraySize);

			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.MipSlice = m_desc.texture.mipSlice;
			uavDesc.Texture2DArray.FirstArraySlice = m_desc.texture.firstArraySlice;
			uavDesc.Texture2DArray.ArraySize = m_desc.texture.arraySize;
		}; break;
		default: ENGI_ASSERT(false); return false;
		}

		HRESULT hr = d3dDevice->CreateUnorderedAccessView(d3dResource, &uavDesc, m_handle.ReleaseAndGetAddressOf());
		if (FAILED(hr))
			return false;

#if !defined(_NDEBUG)
		const std::string& name = this->getName();
		if (m_handle)
			m_handle->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str());
#endif

		return true;
	}

	void* D3D11UnorderedAccessView::getHandle()
	{
		return m_handle.Get();
	}

}; // engi::gfx namespace