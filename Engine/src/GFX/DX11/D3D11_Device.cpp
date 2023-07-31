#include "GFX/DX11/D3D11_Device.h"

#include <cstring>
#include <vector>
#include "Core/CommonDefinitions.h"
#include "GFX/DX11/D3D11_Buffer.h"
#include "GFX/DX11/D3D11_InputLayout.h"
#include "GFX/DX11/D3D11_PipelineState.h"
#include "GFX/DX11/D3D11_Sampler.h"
#include "GFX/DX11/D3D11_Shader.h"
#include "GFX/DX11/D3D11_Swapchain.h"
#include "GFX/DX11/D3D11_Texture.h"
#include "GFX/DX11/D3D11_Descriptor.h"

namespace engi::gfx
{
    D3D11Device::~D3D11Device()
    {
#if !defined(_NDEBUG)
        m_d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
#endif
    }

    IGpuSwapchain* D3D11Device::createSwapchain(const std::string& name, const GpuSwapchainDesc& desc)
    {
        D3D11Swapchain* swapchain = m_resourceAllocator.createResource<D3D11Swapchain>(name, this, desc);
        if (!swapchain->initialize())
            destroy((IGpuResource*&)swapchain);

        return swapchain;
    }

    IGpuBuffer* D3D11Device::createBuffer(const std::string& name, const GpuBufferDesc& desc, const void* initialData)
    {
        D3D11Buffer* buffer = m_resourceAllocator.createResource<D3D11Buffer>(name, this, desc);
        if (!buffer->initialize(initialData))
            destroy((IGpuResource*&)buffer);

        return buffer;
    }

    IGpuShader* D3D11Device::createShader(const std::string& name, const GpuShaderDesc& desc, void* bytecode)
    {
        D3D11Shader* shader = m_resourceAllocator.createResource<D3D11Shader>(name, this, desc);
        if (!shader->initialize(bytecode))
            destroy((IGpuResource*&)shader);

        return shader;
    }

    IGpuTexture* D3D11Device::createTexture(const std::string& name, const GpuTextureDesc& desc, const void* initialData)
    {
        D3D11Texture* texture = m_resourceAllocator.createResource<D3D11Texture>(name, this, desc);
        if (!texture->init(initialData))
            destroy((IGpuResource*&)texture);

        return texture;
    }

    IGpuPipelineState* D3D11Device::createPipelineState(const std::string& name, const GpuPipelineStateDesc& desc)
    {
        D3D11PipelineState* pipelineState = m_resourceAllocator.createResource<D3D11PipelineState>(name, this, desc);
        if (!pipelineState->initialize())
            destroy((IGpuResource*&)pipelineState);
        
        return pipelineState;
    }

    IGpuSampler* D3D11Device::createSampler(const std::string& name, const GpuSamplerDesc& desc)
    {
        D3D11Sampler* sampler = m_resourceAllocator.createResource<D3D11Sampler>(name, this, desc);
        if (!sampler->initialize())
            destroy((IGpuResource*&)sampler);

        return sampler;
    }

    IGpuInputLayout* D3D11Device::createInputLayout(const std::string& name, const GpuInputAttributeDesc* attributes, uint32_t numAttributes, const GpuShaderBuffer& shaderBuffer)
    {
        D3D11InputLayout* inputLayout = m_resourceAllocator.createResource<D3D11InputLayout>(name, this, attributes, numAttributes);
        if (!inputLayout->initialize(shaderBuffer))
            destroy((IGpuResource*&)inputLayout);

        return inputLayout;
    }

    IGpuDescriptor* D3D11Device::createSRV(const std::string& name, const GpuSrvDesc& desc, IGpuResource* resource)
    {
        D3D11ShaderResourceView* srv = m_resourceAllocator.createResource<D3D11ShaderResourceView>(name, this, desc);
        if (!srv->init(resource))
            destroy((IGpuResource*&)srv);

        return srv;
    }

    IGpuDescriptor* D3D11Device::createUAV(const std::string& name, const GpuUavDesc& desc, IGpuResource* resource)
    {
        D3D11UnorderedAccessView* uav = m_resourceAllocator.createResource<D3D11UnorderedAccessView>(name, this, desc);
        if (!uav->init(resource))
            destroy((IGpuResource*&)uav);

        return uav;
    }

    void D3D11Device::destroy(IGpuResource*& resource)
    {
        m_resourceAllocator.destroyResource(resource);
    }

    void D3D11Device::beginRenderPass(const GpuRenderPassDesc& desc)
    {
        m_currentRenderPassDesc = desc;

        static constexpr size_t renderTargetCount = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
        ID3D11DeviceContext4* devcon = getContext();
        ID3D11RenderTargetView* d3dRtvs[renderTargetCount]{};
        UINT numRtvs = 0;
        for (uint32_t i = 0; i < renderTargetCount; ++i)
        {
            auto& rtvDesc = desc.rtvs[i];
            if (!rtvDesc.rtv)
                continue;

            ID3D11RenderTargetView* d3dRtv = reinterpret_cast<ID3D11RenderTargetView*>(((D3D11Texture*)rtvDesc.rtv)->getRTV(rtvDesc.mipSlice, rtvDesc.arraySlice));
            if (rtvDesc.useClearcolor)
                devcon->ClearRenderTargetView(d3dRtv, rtvDesc.clearcolor);

            d3dRtvs[numRtvs] = d3dRtv;
            ++numRtvs;
        }
        
        ID3D11DepthStencilView* dsv = nullptr;
        if (desc.depthStencilBuffer)
        {
            D3D11Texture* dsvTexture = (D3D11Texture*)desc.depthStencilBuffer;
            dsv = reinterpret_cast<ID3D11DepthStencilView*>(dsvTexture->getDSV(desc.depthMip, desc.depthSlice));

            UINT ClearFlags = 0;
            if (desc.useClearDepth)
                ClearFlags |= D3D11_CLEAR_DEPTH;

            if (desc.useClearStencil)
                ClearFlags |= D3D11_CLEAR_STENCIL;
            devcon->ClearDepthStencilView(dsv, ClearFlags, desc.clearDepth, desc.clearStencil);
        }

        UINT numUavs = 0;
        size_t uavMaxCount = renderTargetCount - numRtvs;

        ID3D11UnorderedAccessView* d3dUavs[renderTargetCount]{};
        for (uint32_t i = 0; i < renderTargetCount; ++i)
        {
            if (!desc.uavs[i])
                continue;

            ID3D11UnorderedAccessView* d3dUav = reinterpret_cast<ID3D11UnorderedAccessView*>(desc.uavs[i]->getHandle());
            d3dUavs[numUavs] = d3dUav;
            ++numUavs;
        }

        if (uavMaxCount < numUavs)
        {
            // This should not occur. D3D11 Api requires UAVStartSlot + NumUAVs <= 8
            ENGI_ASSERT(false);
        }

        ID3D11RenderTargetView* const* ParamRtvs = numRtvs == 0 ? nullptr : d3dRtvs;
        ID3D11UnorderedAccessView* const* ParamUavs = numUavs == 0 ? nullptr : d3dUavs;
        UINT uavInitialCounts[renderTargetCount]{}; // Leave as it is. we do not use that anyways
        devcon->OMSetRenderTargetsAndUnorderedAccessViews(numRtvs, ParamRtvs, dsv, numRtvs, numUavs, ParamUavs, uavInitialCounts);
    }

    void D3D11Device::endRenderPass()
    {
        m_currentRenderPassDesc = {};
        ID3D11DeviceContext4* devcon = getContext();
        devcon->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void D3D11Device::draw(uint32_t numVertices, uint32_t vertexOffset)
    {
        getContext()->Draw(numVertices, vertexOffset);
    }

    void D3D11Device::drawIndexed(uint32_t numIndices, uint32_t indexOffset, uint32_t vertexOffset)
    {
        getContext()->DrawIndexed(numIndices, indexOffset, vertexOffset);
    }

    void D3D11Device::drawInstanced(uint32_t numVerticesPerInstance, uint32_t numInstances, uint32_t vertexOffset, uint32_t instanceOffset)
    {
        getContext()->DrawInstanced(numVerticesPerInstance, numInstances, vertexOffset, instanceOffset);
    }

    void D3D11Device::drawIndexedInstanced(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceOffset)
    {
        getContext()->DrawIndexedInstanced(numIndices, numInstances, indexOffset, vertexOffset, instanceOffset);
    }

    void D3D11Device::drawIndexedInstancedIndirect(IGpuBuffer* buffer, uint32_t byteOffset)
    {
        ENGI_ASSERT(buffer);

        ID3D11Buffer* d3dBuffer = (ID3D11Buffer*)buffer->getHandle();
        getContext()->DrawIndexedInstancedIndirect(d3dBuffer, byteOffset);
    }

    void D3D11Device::dispatch(uint32_t threadGroupsX, uint32_t threadGroupsY, uint32_t threadGroupsZ)
    {
        getContext()->Dispatch(threadGroupsX, threadGroupsY, threadGroupsZ);
    }

    void D3D11Device::dispatchIndirect(IGpuBuffer* buffer, uint32_t byteOffset)
    {
        ENGI_ASSERT(buffer);

        ID3D11Buffer* d3dBuffer = (ID3D11Buffer*)buffer->getHandle();
        getContext()->DispatchIndirect(d3dBuffer, byteOffset);
    }

    void D3D11Device::setPipelineState(IGpuPipelineState* state)
    {
        ID3D11DeviceContext* devcon = getContext();
        if (!state)
        {
            devcon->OMSetBlendState(nullptr, nullptr, 0xffffffff);
            devcon->OMSetDepthStencilState(nullptr, 0);
            devcon->RSSetState(nullptr);
            devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            devcon->VSSetShader(nullptr, nullptr, 0);
            devcon->HSSetShader(nullptr, nullptr, 0);
            devcon->DSSetShader(nullptr, nullptr, 0);
            devcon->GSSetShader(nullptr, nullptr, 0);
            devcon->PSSetShader(nullptr, nullptr, 0);
            devcon->CSSetShader(nullptr, nullptr, 0);
            return;
        }
        
        ENGI_ASSERT(state->getDesc().vs || state->getDesc().cs && "Vertex shader (or Compite shader for that matter) cannot be nullptr in active pipeline state");
        D3D11PipelineState* d3dState = (D3D11PipelineState*)state;

        devcon->OMSetBlendState(d3dState->getBlendState(), nullptr, 0xffffffff);
        devcon->OMSetDepthStencilState(d3dState->getDepthStencilState(), state->getDesc().depthStencil.stencilRef);
        devcon->RSSetState(d3dState->getRasterizerState());
        devcon->IASetPrimitiveTopology(d3dState->getPrimitiveTopology());
        devcon->VSSetShader(d3dState->getVertexShader(), nullptr, 0);
        devcon->HSSetShader(d3dState->getHullShader(), nullptr, 0);
        devcon->DSSetShader(d3dState->getDomainShader(), nullptr, 0);
        devcon->GSSetShader(d3dState->getGeometryShader(), nullptr, 0);
        devcon->PSSetShader(d3dState->getPixelShader(), nullptr, 0);
        devcon->CSSetShader(d3dState->getComputeShader(), nullptr, 0);
    }

    void D3D11Device::setInputLayout(IGpuInputLayout* inputLayout)
    {
        ID3D11DeviceContext4* devcon = getContext();
        if (!inputLayout)
        {
            devcon->IASetInputLayout(nullptr);
            return;
        }

        ID3D11InputLayout* layout = (ID3D11InputLayout*)inputLayout->getHandle();
        devcon->IASetInputLayout(layout);
    }

    void D3D11Device::setVertexBuffer(IGpuBuffer* buffer, uint32_t slot, uint32_t stride, uint32_t offset)
    {
        ENGI_ASSERT(buffer && "Buffer cannot be nullptr");
        ENGI_ASSERT((buffer->getDesc().pipelineFlags & VERTEX_BUFFER) == VERTEX_BUFFER && "Buffer must be bound to vertex buffer pipeline");
        ID3D11Buffer* d3dBuffer = (ID3D11Buffer*)buffer->getHandle();
        getContext()->IASetVertexBuffers(slot, 1, &d3dBuffer, &stride, &offset);
    }

    void D3D11Device::setIndexBuffer(IGpuBuffer* buffer, uint32_t offset)
    {
        ENGI_ASSERT(buffer && "Buffer cannot be nullptr");
        ENGI_ASSERT((buffer->getDesc().pipelineFlags & INDEX_BUFFER) == INDEX_BUFFER && "Buffer must be bound to index buffer pipeline");
        ID3D11Buffer* d3dBuffer = (ID3D11Buffer*)buffer->getHandle();
        getContext()->IASetIndexBuffer(d3dBuffer, DXGI_FORMAT_R32_UINT, offset);
    }

    void D3D11Device::setConstantBuffer(IGpuBuffer* buffer, uint32_t slot, uint32_t shaderTypes)
    {
        ENGI_ASSERT(buffer && "Buffer cannot be nullptr");
        ID3D11Buffer* d3dBuffer = (ID3D11Buffer*)buffer->getHandle();
        ID3D11DeviceContext4* devcon = getContext();
        if ((shaderTypes & VERTEX_SHADER) != 0)
            devcon->VSSetConstantBuffers(slot, 1, &d3dBuffer);
        if ((shaderTypes & PIXEL_SHADER) != 0)
            devcon->PSSetConstantBuffers(slot, 1, &d3dBuffer);
        if ((shaderTypes & GEOMETRY_SHADER) != 0)
            devcon->GSSetConstantBuffers(slot, 1, &d3dBuffer);
        if ((shaderTypes & HULL_SHADER) != 0)
            devcon->HSSetConstantBuffers(slot, 1, &d3dBuffer);
        if ((shaderTypes & DOMAIN_SHADER) != 0)
            devcon->DSSetConstantBuffers(slot, 1, &d3dBuffer);
        if ((shaderTypes & COMPUTE_SHADER) != 0)
            devcon->CSSetConstantBuffers(slot, 1, &d3dBuffer);
    }

    void D3D11Device::setSRV(const IGpuDescriptor* descriptor, uint32_t slot, uint32_t shaderTypes)
    {
        D3D11ShaderResourceView* srv = (D3D11ShaderResourceView*)descriptor;
        ID3D11ShaderResourceView* d3dSrv = (srv) ? (ID3D11ShaderResourceView*)srv->getHandle() : nullptr;
        ID3D11DeviceContext4* devcon = getContext();
        if ((shaderTypes & VERTEX_SHADER) != 0)
            devcon->VSSetShaderResources(slot, 1, &d3dSrv);
        if ((shaderTypes & PIXEL_SHADER) != 0)
            devcon->PSSetShaderResources(slot, 1, &d3dSrv);
        if ((shaderTypes & GEOMETRY_SHADER) != 0)
            devcon->GSSetShaderResources(slot, 1, &d3dSrv);
        if ((shaderTypes & HULL_SHADER) != 0)
            devcon->HSSetShaderResources(slot, 1, &d3dSrv);
        if ((shaderTypes & DOMAIN_SHADER) != 0)
            devcon->DSSetShaderResources(slot, 1, &d3dSrv);
        if ((shaderTypes & COMPUTE_SHADER) != 0)
            devcon->CSSetShaderResources(slot, 1, &d3dSrv);
    }

    void D3D11Device::setComputeUAV(const IGpuDescriptor* descriptor, uint32_t slot)
    {
        ID3D11UnorderedAccessView* d3dUav = nullptr;
        if (descriptor)
            d3dUav = (ID3D11UnorderedAccessView*)((D3D11UnorderedAccessView*)descriptor)->getHandle();

        UINT initialCounts = -1;
        getContext()->CSSetUnorderedAccessViews(slot, 1, &d3dUav, &initialCounts);
    }

    void D3D11Device::setSampler(const IGpuSampler* sampler, uint32_t slot, uint32_t shaderTypes)
    {
        D3D11Sampler* s = (D3D11Sampler*)sampler;
        ID3D11SamplerState* handle = (s) ? (ID3D11SamplerState*)s->getHandle() : nullptr;
        ID3D11DeviceContext4* devcon = getContext();
        if ((shaderTypes & VERTEX_SHADER) != 0)
            devcon->VSSetSamplers(slot, 1, &handle);
        if ((shaderTypes & PIXEL_SHADER) != 0)
            devcon->PSSetSamplers(slot, 1, &handle);
        if ((shaderTypes & GEOMETRY_SHADER) != 0)
            devcon->GSSetSamplers(slot, 1, &handle);
        if ((shaderTypes & HULL_SHADER) != 0)
            devcon->HSSetSamplers(slot, 1, &handle);
        if ((shaderTypes & DOMAIN_SHADER) != 0)
            devcon->DSSetSamplers(slot, 1, &handle);
        if ((shaderTypes & COMPUTE_SHADER) != 0)
            devcon->CSSetSamplers(slot, 1, &handle);
    }

    void D3D11Device::setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = static_cast<FLOAT>(x);
        viewport.TopLeftY = static_cast<FLOAT>(y);
        viewport.Width = static_cast<FLOAT>(width);
        viewport.Height = static_cast<FLOAT>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        getContext()->RSSetViewports(1, &viewport);
    }

    static UINT getSubresourceIndex(UINT mipslice, UINT arrayslice, UINT miplevels) noexcept
    {
        return arrayslice * miplevels + mipslice;
    }

    void D3D11Device::copyTexture(IGpuTexture* dst, uint32_t dstMipslice, uint32_t dstArrayslice, uint32_t dstX, uint32_t dstY, uint32_t dstZ,
        IGpuTexture* src, uint32_t srcMipslice, uint32_t srcArrayslice, uint32_t srcX, uint32_t srcY, uint32_t srcZ,
        uint32_t width, uint32_t height, uint32_t depth)
    {
        ENGI_ASSERT(src && dst);
        auto& dstDesc = dst->getDesc();
        auto& srcDesc = src->getDesc();
        ENGI_ASSERT(dstDesc.usage != IMMUTABLE && "You can't use an Immutable resource as a destination");
        // We no longer copy whole subresource, no need for dimensions to match
        //ENGI_ASSERT(dstDesc.width == srcDesc.width && dstDesc.height == srcDesc.height && dstDesc.depth == srcDesc.depth);

        ID3D11Resource* srcD3DResource = (ID3D11Resource*)src->getHandle();
        ID3D11Resource* dstD3DResource = (ID3D11Resource*)dst->getHandle();
        ID3D11DeviceContext4* devcon = getContext();
        
        D3D11_BOX srcBox;
        ENGI_ZEROMEM(&srcBox);
        srcBox.left = srcX;
        srcBox.top = srcY;
        srcBox.front = srcZ;
        srcBox.right = srcX + width;
        srcBox.bottom = srcY + height;
        srcBox.back = srcZ + depth;

        const D3D11_BOX* boxPtr = nullptr;
        if (width > 0 || height > 0 || depth > 0)
        {
            boxPtr = &srcBox;
        }

        UINT dstSubresourceIndex = getSubresourceIndex(dstMipslice, dstArrayslice, dstDesc.miplevels);
        UINT srcSubresourceIndex = getSubresourceIndex(srcMipslice, srcArrayslice, srcDesc.miplevels);
        devcon->CopySubresourceRegion(dstD3DResource, dstSubresourceIndex, dstX, dstY, dstZ, srcD3DResource, srcSubresourceIndex, boxPtr);
    }

    void D3D11Device::copyBuffer(IGpuBuffer* src, IGpuBuffer* dst, uint32_t dstOffset)
    {
        ENGI_ASSERT(src && dst);
        ID3D11Buffer* srcD3dBuffer = (ID3D11Buffer*)src->getHandle();
        ID3D11Buffer* dstD3dBuffer = (ID3D11Buffer*)dst->getHandle();
        ID3D11DeviceContext* devcon = getContext();
        devcon->CopySubresourceRegion(dstD3dBuffer, 0, dstOffset, 0, 0, srcD3dBuffer, 0, nullptr);
    }

    void D3D11Device::updateBuffer(IGpuBuffer* buffer, uint32_t bufferOffset, const void* data, uint32_t byteSize)
    {
        ENGI_ASSERT(buffer && "Buffer cannot be nullptr");
        if (!buffer)
            return;

        ENGI_ASSERT(buffer->getDesc().usage == GpuUsage::DEFAULT && "Cannot update buffer with non-default usage. Use map/unmap");
        if (buffer->getDesc().usage != GpuUsage::DEFAULT)
            return;

        ID3D11Buffer* d3dBuffer = (ID3D11Buffer*)buffer->getHandle();
        ID3D11DeviceContext4* devcon = getContext();
        D3D11_BOX box;
        box.left = bufferOffset;
        box.right = bufferOffset + byteSize;
        box.top = 0;
        box.bottom = 1;
        box.front = 0;
        box.back = 1;

        devcon->UpdateSubresource(d3dBuffer, 0, &box, data, 0, 0);
    }

    void D3D11Device::mapBuffer(IGpuBuffer* buffer, void** mapping)
    {
        if (!mapping)
            return;

        ENGI_ASSERT(buffer && "Buffer cannot be nullptr");
        if (!buffer)
            return;

        ENGI_ASSERT((buffer->getDesc().cpuFlags & WRITE) != 0 && "Buffer must be writeable in order to uplod memory into it");
        ID3D11Buffer* d3dBuffer = (ID3D11Buffer*)buffer->getHandle();
        ID3D11DeviceContext4* devcon = getContext();

        D3D11_MAPPED_SUBRESOURCE subresource;
        bool isDynamic = ((buffer->getDesc().usage & GpuUsage::DYNAMIC) != 0);
        HRESULT hr = devcon->Map(d3dBuffer, 0, (isDynamic ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE), 0, &subresource);
        if (FAILED(hr))
        {
            *mapping = nullptr;
            return;
        }

        *mapping = subresource.pData;
    }

    void D3D11Device::unmapBuffer(IGpuBuffer* buffer)
    {
        if (!buffer)
            return;

        ID3D11Buffer* d3dBuffer = (ID3D11Buffer*)buffer->getHandle();
        ID3D11DeviceContext4* devcon = getContext();
        devcon->Unmap(d3dBuffer, 0);
    }

    bool D3D11Device::initialize()
    {
        createDXGIFactory();
        createD3D11Device();
        createD3D11Debug();
        return true;
    }

    void D3D11Device::createDXGIFactory()
    {
        HRESULT hr;
        ComPtr<IDXGIFactory> factory;
        hr = CreateDXGIFactory(IID_IDXGIFactory, (void**)factory.GetAddressOf());
        ENGI_ASSERT(SUCCEEDED(hr) && "Failed to create DXGI Factory");

        hr = factory->QueryInterface<IDXGIFactory5>(m_dxgiFactory.ReleaseAndGetAddressOf());
        ENGI_ASSERT(SUCCEEDED(hr) && "Failed to query the DXGIFactory5 interface");
    }

    void D3D11Device::createD3D11Device()
    {
        ENGI_ASSERT(m_dxgiFactory.Get() && "DXGI Factory must be initialized before D3D11Device::createD3D11Device");
        HRESULT hr = m_dxgiFactory->EnumAdapters(0, &m_d3dAdapter);
        ENGI_ASSERT(SUCCEEDED(hr) && "Failed to query an adapter");

        D3D_FEATURE_LEVEL reqFeatures = D3D_FEATURE_LEVEL_11_1;
        D3D_FEATURE_LEVEL outFeatures = D3D_FEATURE_LEVEL_11_0;
        UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifndef _NDEBUG
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> devcon;
        hr = D3D11CreateDevice(
            m_d3dAdapter,
            D3D_DRIVER_TYPE_UNKNOWN, // OS should select driver type
            nullptr,
            deviceFlags,
            &reqFeatures,
            1,
            D3D11_SDK_VERSION,
            device.GetAddressOf(),
            &outFeatures,
            devcon.GetAddressOf());
        ENGI_ASSERT(SUCCEEDED(hr) && "Failed to create device or immediate context");

        hr = device->QueryInterface<ID3D11Device5>(m_d3dDevice.ReleaseAndGetAddressOf());
        ENGI_ASSERT(SUCCEEDED(hr) && "Failed to query ID3D11Device5 interface");

        hr = devcon->QueryInterface<ID3D11DeviceContext4>(m_d3dContext.ReleaseAndGetAddressOf());
        ENGI_ASSERT(SUCCEEDED(hr) && "Failed to query ID3D11DeviceContext4 interface");
    }

    void D3D11Device::createD3D11Debug()
    {
        HRESULT hr = m_d3dDevice->QueryInterface<ID3D11Debug>(m_d3dDebug.ReleaseAndGetAddressOf());
        ENGI_ASSERT(SUCCEEDED(hr) && "Failed to query ID3D11Debug interface");
    }

}; // engi::gfx namespace