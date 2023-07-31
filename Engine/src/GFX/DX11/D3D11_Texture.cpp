#include "GFX/DX11/D3D11_Texture.h"

#include "Math/Math.h"
#include "Core/CommonDefinitions.h"
#include "GFX/DX11/D3D11_Utility.h"
#include "GFX/DX11/D3D11_Device.h"

namespace engi::gfx
{

    D3D11Texture::D3D11Texture(const std::string& name, D3D11Device* device, const GpuTextureDesc& desc)
    {
        m_name = name;
        m_device = device;
        m_desc = desc;
    }

    static HRESULT createTexture1D(ID3D11Device* pD3dDevice, const void* pInitialData, GpuTextureDesc& desc, ID3D11Resource** ppResource)
    {
        D3D11_TEXTURE1D_DESC d3dDesc;
        d3dDesc.Width = desc.width;
        // Handle correct amount of mips manually due to API preferences
        if (desc.miplevels == 0)
        {
            UINT mips = static_cast<UINT>(math::log2((float)desc.width)) + 1;
            d3dDesc.MipLevels = mips;
            desc.miplevels = mips;
        }
        else d3dDesc.MipLevels = desc.miplevels;
        d3dDesc.ArraySize = desc.arraySize;
        d3dDesc.Format = detail::d3d11Format(desc.format);
        d3dDesc.Usage = detail::d3d11Usage(desc.usage);
        d3dDesc.BindFlags = detail::d3d11BindFlags(desc.pipelineFlags);
        d3dDesc.CPUAccessFlags = detail::d3d11CpuAccessFlags(desc.cpuFlags);
        d3dDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA subresourceData;
        subresourceData.pSysMem = pInitialData;
        subresourceData.SysMemPitch = 0; // TODO: TEMP IS ZERO!
        subresourceData.SysMemSlicePitch = 0;

        return pD3dDevice->CreateTexture1D(&d3dDesc, pInitialData ? &subresourceData : nullptr, (ID3D11Texture1D**)ppResource);
    }

    static HRESULT createTexture2D(ID3D11Device* pD3dDevice, const void* pInitialData, GpuTextureDesc& desc, ID3D11Resource** ppResource)
    {
        D3D11_TEXTURE2D_DESC d3dDesc;
        d3dDesc.Width = desc.width;
        d3dDesc.Height = desc.height;
        // Handle correct amount of mips manually due to API preferences
        if (desc.miplevels == 0)
        {
            math::Vec2 res((float)desc.width, (float)desc.height);
            UINT mips = static_cast<UINT>(math::log2(res.getMin())) + 1;
            d3dDesc.MipLevels = mips;
            desc.miplevels = mips;
        }
        else d3dDesc.MipLevels = desc.miplevels;
        d3dDesc.ArraySize = desc.arraySize;
        d3dDesc.Format = detail::d3d11Format(desc.format);
        d3dDesc.SampleDesc.Count = 1;
        d3dDesc.SampleDesc.Quality = 0;
        d3dDesc.Usage = detail::d3d11Usage(desc.usage);
        d3dDesc.BindFlags = detail::d3d11BindFlags(desc.pipelineFlags);
        d3dDesc.CPUAccessFlags = detail::d3d11CpuAccessFlags(desc.cpuFlags);
        d3dDesc.MiscFlags = detail::d3d11MiscFlags(desc.otherFlags);

        D3D11_SUBRESOURCE_DATA subresourceData;
        subresourceData.pSysMem = pInitialData;
        subresourceData.SysMemPitch = 0; // TODO: TEMP IS ZERO!
        subresourceData.SysMemSlicePitch = 0;

        return pD3dDevice->CreateTexture2D(&d3dDesc, pInitialData ? &subresourceData : nullptr, (ID3D11Texture2D**)ppResource);
    }

    static HRESULT createTexture3D(ID3D11Device* pD3dDevice, const void* pInitialData, GpuTextureDesc& desc, ID3D11Resource** ppResource)
    {
        D3D11_TEXTURE3D_DESC d3dDesc;
        d3dDesc.Width = desc.width;
        d3dDesc.Height = desc.height;
        d3dDesc.Depth = desc.depth;
        // Handle correct amount of mips manually due to API preferences
        if (desc.miplevels == 0)
        {
            math::Vec2 res((float)desc.width, (float)desc.height);
            UINT mips = static_cast<UINT>(math::log2(res.getMin())) + 1;
            d3dDesc.MipLevels = mips;
            desc.miplevels = mips;
        }
        else d3dDesc.MipLevels = desc.miplevels;
        d3dDesc.Format = detail::d3d11Format(desc.format);
        d3dDesc.Usage = detail::d3d11Usage(desc.usage);
        d3dDesc.BindFlags = detail::d3d11BindFlags(desc.pipelineFlags);
        d3dDesc.CPUAccessFlags = detail::d3d11CpuAccessFlags(desc.cpuFlags);
        d3dDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA subresourceData;
        subresourceData.pSysMem = pInitialData;
        subresourceData.SysMemPitch = 0; // TODO: TEMP IS ZERO!
        subresourceData.SysMemSlicePitch = 0;

        return pD3dDevice->CreateTexture3D(&d3dDesc, pInitialData ? &subresourceData : nullptr, (ID3D11Texture3D**)ppResource);
    }

    bool D3D11Texture::init(const void* initialData)
    {
        D3D11Device* device = (D3D11Device*)m_device;
        ID3D11Device* handle = device->getHandle();
        HRESULT hr;
        switch (m_desc.type)
        {
        case TEXTURE1D: hr = createTexture1D(handle, initialData, m_desc, m_handle.ReleaseAndGetAddressOf()); break;
        case TEXTURE2D: hr = createTexture2D(handle, initialData, m_desc, m_handle.ReleaseAndGetAddressOf()); break;
        case TEXTURE3D: hr = createTexture3D(handle, initialData, m_desc, m_handle.ReleaseAndGetAddressOf()); break;
        }
        m_rtvs.clear();

        // TODO: Maybe somehow incapsulate this code in D3D11_Resource?
#if !defined(_NDEBUG)
        const std::string& name = this->getName();
        if (m_handle)
            m_handle->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str());
#endif
        return SUCCEEDED(hr);
    }

    void* D3D11Texture::getHandle()
    {
        return m_handle.Get();
    }

    void* D3D11Texture::getRTV(uint32_t mipSlice, uint32_t arraySlice)
    {
        ENGI_ASSERT((m_desc.pipelineFlags & RENDER_TARGET) != 0 && "D3D11Texture pipeline binding must be RENDER_TARGET to get its rtv");
        ENGI_ASSERT(m_desc.arraySize > arraySlice);
        ENGI_ASSERT(m_desc.miplevels > mipSlice);

        if (m_rtvs.empty())
        {
            size_t subresources = m_desc.arraySize * m_desc.miplevels;
            m_rtvs.resize(subresources);
        }

        uint32_t index = m_desc.miplevels * arraySlice + mipSlice;
        if (!m_rtvs[index])
        {
            D3D11Device* device = (D3D11Device*)m_device;
            D3D11_RENDER_TARGET_VIEW_DESC desc;
            desc.Format = detail::d3d11Format(m_desc.format);

            // pick correct rtv dimension
            bool isArray = m_desc.arraySize > 1;
            switch (m_desc.type)
            {
            case TEXTURE1D: 
            {
                if (isArray)
                {
                    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
                    desc.Texture1DArray.ArraySize = 1;
                    desc.Texture1DArray.FirstArraySlice = arraySlice;
                    desc.Texture1DArray.MipSlice = mipSlice;
                }
                else
                {
                    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
                    desc.Texture1D.MipSlice = mipSlice;
                }
            }; break;
            case TEXTURE2D: 
            {
                if (isArray)
                {
                    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                    desc.Texture2DArray.ArraySize = 1;
                    desc.Texture2DArray.FirstArraySlice = arraySlice;
                    desc.Texture2DArray.MipSlice = mipSlice;
                }
                else
                {
                    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipSlice = mipSlice;
                }
            }; break;
            case TEXTURE3D: 
            {
                ENGI_ASSERT(!isArray && "D3D11 RTVs cannot be of TEXTURE3D_ARRAY type");
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
                desc.Texture3D.WSize = 1;
                desc.Texture3D.FirstWSlice = arraySlice;
                desc.Texture3D.MipSlice = mipSlice;
            }; break;
            default: ENGI_ASSERT(false);
            }
            device->getHandle()->CreateRenderTargetView(m_handle.Get(), &desc, m_rtvs[index].GetAddressOf());
        }

        return m_rtvs.at(index).Get();
    }

    void* D3D11Texture::getDSV(uint32_t mipSlice, uint32_t arraySlice)
    {
        ENGI_ASSERT((m_desc.pipelineFlags & DEPTH_STENCIL) != 0 && "D3D11Texture pipeline binding must be DEPTH_STENCIL to get its dsv");
        ENGI_ASSERT(m_desc.arraySize > arraySlice);
        ENGI_ASSERT(m_desc.miplevels > mipSlice);

        if (m_dsvs.empty())
        {
            size_t subresources = m_desc.arraySize * m_desc.miplevels;
            m_dsvs.resize(subresources);
        }

        uint32_t index = m_desc.miplevels * arraySlice + mipSlice;
        if (!m_dsvs[index])
        {
            D3D11Device* device = (D3D11Device*)m_device;

            D3D11_DEPTH_STENCIL_VIEW_DESC desc;
            desc.Format = detail::d3d11Format(GpuFormat::DEPTH_STENCIL_FORMAT);
            desc.Flags = 0; // READ-ONLY? TODO: Make in future for optimization

            bool isArray = m_desc.arraySize > 1;
            switch (m_desc.type)
            {
            case TEXTURE1D:
            {
                if (isArray)
                {
                    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
                    desc.Texture1DArray.ArraySize = 1;
                    desc.Texture1DArray.FirstArraySlice = arraySlice;
                    desc.Texture1DArray.MipSlice = mipSlice;
                }
                else
                {
                    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
                    desc.Texture1D.MipSlice = mipSlice;
                }
            }; break;
            case TEXTURE2D:
            {
                if (isArray)
                {
                    // check if the texture is a cubemap and handle it separately
                    bool isCubemap = ((m_desc.otherFlags & GpuMisc::MISC_TEXTURECUBE) != 0);
                    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                    desc.Texture2DArray.ArraySize = isCubemap ? 6 : 1;
                    desc.Texture2DArray.FirstArraySlice = isCubemap ? arraySlice * 6 : arraySlice;
                    desc.Texture2DArray.MipSlice = mipSlice;
                }
                else
                {
                    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipSlice = mipSlice;
                }
            }; break;
            case TEXTURE3D: // no DSV for texture3D in D3D11
            default: ENGI_ASSERT(false);
            }
            device->getHandle()->CreateDepthStencilView(m_handle.Get(), &desc, m_dsvs[index].GetAddressOf());
        }

        return m_dsvs[index].Get();
    }

    /*ID3D11RenderTargetView* D3D11Texture::getRtv()
    {
        ENGI_ASSERT((m_desc.pipelineFlags & RENDER_TARGET) != 0 && "D3D11Texture pipeline binding must be RENDER_TARGET to get its rtv");
        if (!m_rtv)
        {
            D3D11Device* device = (D3D11Device*)m_device;
            device->getHandle()->CreateRenderTargetView(m_handle.Get(), nullptr, m_rtv.GetAddressOf());
        }

        return m_rtv.Get();
    }

    ID3D11DepthStencilView* D3D11Texture::getDsv()
    {
        ENGI_ASSERT((m_desc.pipelineFlags & DEPTH_STENCIL) != 0 && "D3D11Texture pipeline binding must be DEPTH_STENCIL to get its dsv");
        if (!m_dsv)
        {
            D3D11Device* device = (D3D11Device*)m_device;
            device->getHandle()->CreateDepthStencilView(m_handle.Get(), nullptr, m_dsv.GetAddressOf());
        }

        return m_dsv.Get();
    }*/

    /*ID3D11ShaderResourceView* D3D11Texture::getSrv()
    {
        ENGI_ASSERT((m_desc.pipelineFlags & SHADER_RESOURCE) != 0 && "D3D11Texture pipeline binding must be SHADER_RESOURCE to get its srv");
        if (!m_srv)
        {
            D3D11Device* device = (D3D11Device*)m_device;
            device->getHandle()->CreateShaderResourceView(m_handle.Get(), nullptr, m_srv.GetAddressOf());
        }

        return m_srv.Get();
    }*/

}; // engi::gfx namespace
