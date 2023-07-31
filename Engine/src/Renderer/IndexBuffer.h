#pragma once

#include "GFX/GPUBuffer.h"
#include "GFX/GPUDevice.h"
#include "GFX/GPUResourceAllocator.h"

namespace engi
{

	class IndexBuffer
	{
	public:
		using IndexType = uint32_t;

		IndexBuffer(const std::string& name, gfx::IGpuDevice* device);
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
		~IndexBuffer() = default;

		bool initialize(const uint32_t* indices, uint32_t numIndices);
		void bind(uint32_t numOffset);

	private:
		std::string m_name;
		gfx::IGpuDevice* m_device;
		gfx::GpuHandle<gfx::IGpuBuffer> m_buffer = nullptr;
	}; // IndexBuffer class

}; // engi namespace