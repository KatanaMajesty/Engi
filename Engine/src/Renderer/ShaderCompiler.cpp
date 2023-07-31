#include "Renderer/ShaderCompiler.h"

#include <d3dcompiler.h>
#include <fstream>
#include <cstdint>
#include "GFX/GPUDevice.h"

// TODO: Remove this header
#include <iostream>

namespace engi
{

    static LPCSTR d3dShaderTarget(gfx::GpuShaderType type)
    {
        switch (type)
        {
        case gfx::GpuShaderType::VERTEX_SHADER: return "vs_5_0";
        case gfx::GpuShaderType::PIXEL_SHADER: return "ps_5_0";
        case gfx::GpuShaderType::GEOMETRY_SHADER: return "gs_5_0";
        case gfx::GpuShaderType::HULL_SHADER: return "hs_5_0";
        case gfx::GpuShaderType::DOMAIN_SHADER: return "ds_5_0";
        case gfx::GpuShaderType::COMPUTE_SHADER: return "cs_5_0";
        default: return "unknown";
        }
    }

    void* ShaderCompiler::compileFromHLSLFile(const std::string& filepath, const std::string& entrypoint, gfx::GpuShaderType shaderType) noexcept
	{
		std::ifstream stream(filepath);
		if (!stream.is_open())
		{
			std::cout << "Failed to open the file " << filepath << std::endl;
			return nullptr;
		}

		std::string src = std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
		stream.close();

		uint32_t flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if !defined(_NDEBUG)
		flags |= D3DCOMPILE_DEBUG;
#endif

        ID3D10Blob* shaderBytecode = nullptr;
        ID3D10Blob* shaderErrors = nullptr;
        HRESULT hr = D3DCompile(src.data(), src.length(), filepath.data(), nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entrypoint.c_str(),
            d3dShaderTarget(shaderType),
            flags,
            0, &shaderBytecode, &shaderErrors);

        if (FAILED(hr))
        {
            std::cout << "Error while compiling " << filepath << " as HLSL\n";
            if (shaderErrors)
            {
                // TODO: Add Logger
                std::cout << "Failed to compile " << filepath << "::" << entrypoint << " shader:\n";
                std::cout << (char*)shaderErrors->GetBufferPointer();
                shaderErrors->Release();
            }
            if (shaderBytecode)
            {
                shaderBytecode->Release();
            }
            return nullptr;
        }

        return shaderBytecode;
	}

    void* ShaderCompiler::compileFromHLSLFile(const gfx::GpuShaderDesc& desc) noexcept
    {
        return compileFromHLSLFile(desc.filepath, desc.entrypoint, desc.type);
    }

}; // engi namespace