#include "GFX/DX11/D3D11_Buffer.h"

#include "GFX/DX11/D3D11_Utility.h"
#include "GFX/DX11/D3D11_Device.h"

namespace engi::gfx
{

    D3D11Buffer::D3D11Buffer(const std::string& name, D3D11Device* device, const GpuBufferDesc& desc)
    {
        m_device = device;
        m_name = name;
        m_desc = desc;
    }

    bool D3D11Buffer::initialize(const void* initialData)
    {
        D3D11Device* device = static_cast<D3D11Device*>(m_device);
        ID3D11Device* d3dDevice = device->getHandle();

        D3D11_BUFFER_DESC d3dBufferDesc;
        d3dBufferDesc.ByteWidth = m_desc.bytes;
        d3dBufferDesc.Usage = detail::d3d11Usage(m_desc.usage);
        d3dBufferDesc.BindFlags = detail::d3d11BindFlags(m_desc.pipelineFlags);
        d3dBufferDesc.CPUAccessFlags = detail::d3d11CpuAccessFlags(m_desc.cpuFlags);

        // Remark: There is a GpuMisc::MISC_BYTEADDRESS_BUFFER flag which is not converted to D3D11 Flag, which is an expected behavior
        // The flag GpuMisc::MISC_BYTEADDRESS_BUFFER is used in UAV Descriptor to specify correct flags for the buffer
        d3dBufferDesc.MiscFlags = detail::d3d11MiscFlags(m_desc.otherFlags);
        d3dBufferDesc.StructureByteStride = m_desc.byteStride;

        D3D11_SUBRESOURCE_DATA d3dSubresourceData;
        d3dSubresourceData.pSysMem = initialData;
        d3dSubresourceData.SysMemPitch = 0;
        d3dSubresourceData.SysMemSlicePitch = 0;

        HRESULT hr = d3dDevice->CreateBuffer(&d3dBufferDesc, (initialData ? &d3dSubresourceData : nullptr), m_handle.ReleaseAndGetAddressOf());
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

    void* D3D11Buffer::getHandle()
    {
        return m_handle.Get();
    }

}; // engi::gfx namespace