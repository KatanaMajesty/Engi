#pragma once

#include "GFX/Definitions.h"
#include "GFX/GPUResource.h"

namespace engi::gfx
{

	class IGpuShader : public IGpuResource
	{
	public:
		IGpuShader() = default;
		virtual ~IGpuShader() = default;

		virtual bool initialize(void* bytecode) = 0;
		virtual GpuShaderBuffer getBytecode() = 0;
		
		inline const GpuShaderDesc& getDesc() const { return m_desc; }

	protected:
		GpuShaderDesc m_desc{};
	};

}; // engi::gfx namespace