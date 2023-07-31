#pragma once

#include "GFX/Definitions.h"
#include "GFX/GPUResource.h"

namespace engi::gfx
{

	class IGpuSampler : public IGpuResource
	{
	public:
		IGpuSampler() = default;
		virtual ~IGpuSampler() = default;

		const GpuSamplerDesc& getDesc() const { return m_desc; }

	protected:
		GpuSamplerDesc m_desc{};
	};

}; // engi::gfx namespace