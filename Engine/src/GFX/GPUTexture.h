#pragma once

#include "GFX/Definitions.h"
#include "GFX/GPUResource.h"

namespace engi::gfx
{

	class IGpuTexture : public IGpuResource
	{
	public:
		IGpuTexture() = default;
		virtual ~IGpuTexture() = default;

		virtual void* getRTV(uint32_t mipSlice, uint32_t arraySlice) = 0;
		virtual void* getDSV(uint32_t mipSlice, uint32_t arraySlice) = 0;

		const GpuTextureDesc& getDesc() const { return m_desc; }
	
	protected:
		GpuTextureDesc m_desc{};
	}; // IGpuTexture class

}; // engi::gfx namespace