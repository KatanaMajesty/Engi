#include "Renderer/IndexBuffer.h"

#include "Core/CommonDefinitions.h"

namespace engi
{

	using namespace gfx;

	IndexBuffer::IndexBuffer(const std::string& name, IGpuDevice* device)
		: m_name(name)
		, m_device(device)
	{
	}

	bool IndexBuffer::initialize(const uint32_t* indices, uint32_t numIndices)
	{
		ENGI_ASSERT(m_device && "Logical device was nullptr in IndexBuffer::initialize");

		GpuBufferDesc desc;
		desc.usage = GpuUsage::IMMUTABLE;
		desc.bytes = numIndices * sizeof(uint32_t);
		desc.pipelineFlags = GpuBinding::INDEX_BUFFER; // use GpuBinding enum
		desc.cpuFlags = CpuAccess::ACCESS_UNUSED; // use CpuAccess enum
		desc.byteStride = 0;

		m_buffer = makeGpuHandle(m_device->createBuffer("IndexBuffer_" + m_name, desc, indices), m_device->getResourceAllocator());
		if (!m_buffer)
		{
			return false;
		}

		return true;
	}

	void IndexBuffer::bind(uint32_t numOffset)
	{
		ENGI_ASSERT(m_buffer && "Index buffer was not initialized correctly");
		m_device->setIndexBuffer(m_buffer.get(), numOffset * sizeof(uint32_t));
	}

}; // engi namespace