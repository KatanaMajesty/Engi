#pragma once

#include "GFX/GPUResourceAllocator.h"
#include "GFX/Definitions.h"

namespace engi
{

	namespace gfx
	{
		class IGpuDevice;
		class IGpuBuffer;
	}

	// Immutable Buffers are useful for geometry that is loaded once
	// uses IMMUTABLE flag and is not accessible by CPU at all
	class ImmutableBuffer
	{
	public:
		ImmutableBuffer(const std::string& name, gfx::IGpuDevice* device);
		~ImmutableBuffer() = default;

		bool init(const void* data, uint32_t numVertices, uint32_t vertexSize) noexcept;
		void bind(uint32_t slot, uint32_t numOffset) const noexcept;

	private:
		std::string m_name;
		gfx::IGpuDevice* m_device;
		gfx::GpuHandle<gfx::IGpuBuffer> m_handle = nullptr;
		uint32_t m_vertexSize = 0;
	};

}; // engi namespace