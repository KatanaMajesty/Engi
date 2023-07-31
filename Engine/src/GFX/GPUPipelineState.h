#pragma once

#include "GFX/Definitions.h"
#include "GFX/GPUResource.h"

namespace engi::gfx
{

	class IGpuPipelineState : public IGpuResource
	{
	public:
		IGpuPipelineState() = default;
		virtual ~IGpuPipelineState() = default;

		const GpuPipelineStateDesc& getDesc() const { return m_desc; }

	protected:
		GpuPipelineStateDesc m_desc{};
	};

}; // engi::gfx namespace