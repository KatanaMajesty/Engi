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
	
	// Long Lived Buffers are for data that is streamed from disk but is expected to last for a long time
	// VertexBuffers, IndexBuffers that are updated sometimes
	// USAGE_DEFAULT, UpdateSubresource to update
	class LongLivedBuffer
	{
	public:
		LongLivedBuffer(const std::string& name, gfx::IGpuDevice* device);
		LongLivedBuffer(const LongLivedBuffer&) = delete;
		LongLivedBuffer& operator=(const LongLivedBuffer&) = delete;
		~LongLivedBuffer() = default;

		bool init(const void* data, uint32_t numVertices, uint32_t vertexSize) noexcept;
		void bind(uint32_t slot, uint32_t numOffset) const noexcept;
		void updateBuffer(uint32_t numOffset, const void* data, uint32_t numVertices) noexcept;
		void copyFrom(const LongLivedBuffer* other, uint32_t numOffset) noexcept;

	private:
		std::string m_name;
		gfx::IGpuDevice* m_device;
		gfx::GpuHandle<gfx::IGpuBuffer> m_handle = nullptr;
		uint32_t m_vertexSize = 0;
	};

}; // engi namespace