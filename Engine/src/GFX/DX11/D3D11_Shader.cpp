#include "GFX/DX11/D3D11_Shader.h"

#include "Core/CommonDefinitions.h"
#include "GFX/DX11/D3D11_API.h"
#include "GFX/DX11/D3D11_Utility.h"
#include "GFX/DX11/D3D11_Device.h"

// TODO: Remove this header
#include <iostream>
#include <fstream>

namespace engi::gfx
{

    D3D11Shader::D3D11Shader(const std::string& name, D3D11Device* device, const GpuShaderDesc& desc)
    {
        m_device = device;
        m_name = name;
        m_desc = desc;
    }

    bool D3D11Shader::initialize(void* bytecode)
    {
        ENGI_ASSERT(bytecode);
        D3D11Device* device = (D3D11Device*)m_device;
        ID3D11Device* d3dDevice = device->getHandle();

        attachBytecode(bytecode);
        GpuShaderBuffer buffer = getBytecode();

        ENGI_ASSERT(m_desc.type != GpuShaderType::UNKNOWN_SHADER);
        HRESULT hr;

        // TODO: Maybe somehow incapsulate this code in D3D11_Resource?
#if !defined(_NDEBUG)
        std::string name = "Shader_" + this->getName();
        switch (m_desc.type)
        {
        case VERTEX_SHADER: 
            hr = d3dDevice->CreateVertexShader(buffer.bytecode, buffer.size, nullptr, (ID3D11VertexShader**)m_handle.ReleaseAndGetAddressOf());
            name += "_VertexShader";
            break;
        case PIXEL_SHADER:
            hr = d3dDevice->CreatePixelShader(buffer.bytecode, buffer.size, nullptr, (ID3D11PixelShader**)m_handle.ReleaseAndGetAddressOf());
            name += "_PixelShader";
            break;
        case GEOMETRY_SHADER: 
            hr = d3dDevice->CreateGeometryShader(buffer.bytecode, buffer.size, nullptr, (ID3D11GeometryShader**)m_handle.ReleaseAndGetAddressOf());
            name += "_GeometryShader";
            break;
        case HULL_SHADER: 
            hr = d3dDevice->CreateHullShader(buffer.bytecode, buffer.size, nullptr, (ID3D11HullShader**)m_handle.ReleaseAndGetAddressOf());
            name += "_HullShader";
            break;
        case DOMAIN_SHADER: 
            hr = d3dDevice->CreateDomainShader(buffer.bytecode, buffer.size, nullptr, (ID3D11DomainShader**)m_handle.ReleaseAndGetAddressOf());
            name += "_DomainShader";
            break;
        case COMPUTE_SHADER: 
            hr = d3dDevice->CreateComputeShader(buffer.bytecode, buffer.size, nullptr, (ID3D11ComputeShader**)m_handle.ReleaseAndGetAddressOf()); 
            name += "_ComputeShader";
            break;
        default: ENGI_ASSERT(false);
        }

        if (m_handle)
            m_handle->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str());
#else
        switch (m_desc.type)
        {
        case VERTEX_SHADER: hr = d3dDevice->CreateVertexShader(buffer.bytecode, buffer.size, nullptr, (ID3D11VertexShader**)m_handle.ReleaseAndGetAddressOf()); break;
        case PIXEL_SHADER: hr = d3dDevice->CreatePixelShader(buffer.bytecode, buffer.size, nullptr, (ID3D11PixelShader**)m_handle.ReleaseAndGetAddressOf()); break;
        case GEOMETRY_SHADER: hr = d3dDevice->CreateGeometryShader(buffer.bytecode, buffer.size, nullptr, (ID3D11GeometryShader**)m_handle.ReleaseAndGetAddressOf()); break;
        case HULL_SHADER: hr = d3dDevice->CreateHullShader(buffer.bytecode, buffer.size, nullptr, (ID3D11HullShader**)m_handle.ReleaseAndGetAddressOf()); break;
        case DOMAIN_SHADER: hr = d3dDevice->CreateDomainShader(buffer.bytecode, buffer.size, nullptr, (ID3D11DomainShader**)m_handle.ReleaseAndGetAddressOf()); break;
        case COMPUTE_SHADER: hr = d3dDevice->CreateComputeShader(buffer.bytecode, buffer.size, nullptr, (ID3D11ComputeShader**)m_handle.ReleaseAndGetAddressOf()); break;
        default: ENGI_ASSERT(false);
        }
#endif

        return SUCCEEDED(hr);
    }

    void D3D11Shader::attachBytecode(void* bytecode)
    {
        ID3D10Blob* buffer = (ID3D10Blob*)bytecode;
        ENGI_ASSERT(buffer->GetBufferPointer() && buffer->GetBufferSize() > 0);
        m_shaderBlob.Reset();
        m_shaderBlob.Attach(buffer);
    }

    void* D3D11Shader::getHandle()
    {
        return m_handle.Get();
    }

    GpuShaderBuffer D3D11Shader::getBytecode()
    {
        GpuShaderBuffer buffer;
        buffer.bytecode = m_shaderBlob->GetBufferPointer();
        buffer.size = m_shaderBlob->GetBufferSize();
        return buffer;
    }

}; // engi::gfx namespace