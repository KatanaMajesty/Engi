#include "Renderer/ImmutableBuffer.h"

#include "GFX/GPUDevice.h"
#include "GFX/GPUBuffer.h"

namespace engi
{

	ImmutableBuffer::ImmutableBuffer(const std::string& name, gfx::IGpuDevice* device)
		: m_name(name)
		, m_device(device)
	{
	}

	bool ImmutableBuffer::init(const void* data, uint32_t numVertices, uint32_t vertexSize) noexcept
	{
		using namespace gfx;
		if (!m_device)
			return false;

		m_vertexSize = vertexSize;

		GpuBufferDesc desc;
		desc.usage = GpuUsage::IMMUTABLE;
		desc.bytes = numVertices * vertexSize;
		desc.pipelineFlags = GpuBinding::VERTEX_BUFFER;
		desc.cpuFlags = CpuAccess::ACCESS_UNUSED;
		desc.byteStride = 0;
		m_handle = makeGpuHandle(m_device->createBuffer("ImmutableBuffer_" + m_name, desc, data), m_device->getResourceAllocator());
		if (!m_handle)
		{
			return false;
		}

		return true;
	}

	void ImmutableBuffer::bind(uint32_t slot, uint32_t numOffset) const noexcept
	{
		uint32_t byteOffset = numOffset * m_vertexSize;
		m_device->setVertexBuffer(m_handle.get(), slot, m_vertexSize, byteOffset);
	}

};