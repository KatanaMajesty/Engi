#pragma once

#include <vector>
#include "GFX/Definitions.h"
#include "GFX/GPUResource.h"

namespace engi::gfx
{

	class IGpuInputLayout : public IGpuResource
	{
	public:
		IGpuInputLayout() = default;
		virtual ~IGpuInputLayout() = default;

	protected:
		std::vector<GpuInputAttributeDesc> m_attributes;
	}; // IGpuInputLayout class

}; // engi::gfx namespace