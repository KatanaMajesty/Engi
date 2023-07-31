#pragma once

#include <string>
#include "GFX/Definitions.h"

namespace engi
{

	class ShaderCompiler
	{
	public:
		ShaderCompiler() = default;
		
		[[nodiscard]] void* compileFromHLSLFile(const std::string& filepath, const std::string& entrypoint, gfx::GpuShaderType shaderType) noexcept;
		[[nodiscard]] void* compileFromHLSLFile(const gfx::GpuShaderDesc& desc) noexcept;
	};

}; // engi namespace