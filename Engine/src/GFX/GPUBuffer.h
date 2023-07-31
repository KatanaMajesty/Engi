#pragma once

#include "GFX/Definitions.h"
#include "GFX/GPUResource.h"

namespace engi::gfx
{

	class IGpuBuffer : public IGpuResource
	{
	public:
		IGpuBuffer() = default;
		virtual ~IGpuBuffer() = default;

		inline const GpuBufferDesc& getDesc() const { return m_desc; }

	protected:
		GpuBufferDesc m_desc{};
	}; // IGpuBuffer class

}; // engi::gfx namespace