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

	// Temporary Buffer is suitable for "Fire-and-Forget" data which almost certainly lives in ram
	// Or it can also be used for data that is frequently updated (on per-frame basis)
	// DYNAMIC flag is used which offers for MAP/UNMAP functions to update the buffer
	class DynamicBuffer
	{
	public:
		DynamicBuffer(const std::string& name, gfx::IGpuDevice* device);
		DynamicBuffer(const DynamicBuffer&) = delete;
		DynamicBuffer& operator=(const DynamicBuffer&) = delete;
		~DynamicBuffer() = default;

		bool init(const void* data, uint32_t numVertices, uint32_t vertexSize) noexcept;
		void bind(uint32_t slot, uint32_t numOffset) const noexcept;
		void* map() noexcept;
		void unmap() noexcept;
		void copyFrom(const DynamicBuffer* other, uint32_t numOffset) noexcept;

	private:
		std::string m_name;
		gfx::IGpuDevice* m_device;
		gfx::GpuHandle<gfx::IGpuBuffer> m_handle = nullptr;
		uint32_t m_vertexSize = 0;
	};

}; // engi namespace