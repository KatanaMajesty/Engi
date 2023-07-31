#pragma once

#include "GFX/Definitions.h"
#include "GFX/GPUResource.h"

namespace engi::gfx
{

	class IGpuDescriptor : public IGpuResource
	{
	public:
		IGpuDescriptor() = default;
		virtual ~IGpuDescriptor() = default;
	};

}; // engi::gfx namespace