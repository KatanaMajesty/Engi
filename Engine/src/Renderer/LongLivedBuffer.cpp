#include "Renderer/LongLivedBuffer.h"

#include "Core/CommonDefinitions.h"
#include "GFX/GPUDevice.h"
#include "GFX/GPUBuffer.h"

namespace engi
{

	LongLivedBuffer::LongLivedBuffer(const std::string& name, gfx::IGpuDevice* device)
		: m_name(name)
		, m_device(device)
	{
	}

	bool LongLivedBuffer::init(const void* data, uint32_t numVertices, uint32_t vertexSize) noexcept
	{
		using namespace gfx;
		if (!m_device)
			return false;

		m_vertexSize = vertexSize;

		GpuBufferDesc desc;
		desc.usage = GpuUsage::DEFAULT;
		desc.bytes = numVertices * vertexSize;
		desc.pipelineFlags = GpuBinding::VERTEX_BUFFER;
		desc.cpuFlags = CpuAccess::ACCESS_UNUSED;
		desc.byteStride = 0;
		m_handle = makeGpuHandle(m_device->createBuffer("LongLivedBuffer_" + m_name, desc, data), m_device->getResourceAllocator());
		if (!m_handle)
		{
			return false;
		}

		return true;
	}

	void LongLivedBuffer::bind(uint32_t slot, uint32_t numOffset) const noexcept
	{
		uint32_t byteOffset = numOffset * m_vertexSize;
		m_device->setVertexBuffer(m_handle.get(), slot, m_vertexSize, byteOffset);
	}

	void LongLivedBuffer::updateBuffer(uint32_t numOffset, const void* data, uint32_t numVertices) noexcept
	{
		if (!m_handle)
			return;

		uint32_t byteOffset = numOffset * m_vertexSize;
		uint32_t byteSize = numVertices * m_vertexSize;
		m_device->updateBuffer(m_handle.get(), byteOffset, data, byteSize);
	}

	void LongLivedBuffer::copyFrom(const LongLivedBuffer* other, uint32_t numOffset) noexcept
	{
		ENGI_ASSERT(other && "Buffer cannot be nullptr");
		if (!m_handle || !other->m_handle)
			return;

		if (m_device != other->m_device)
			return;

		if (other->m_vertexSize == 0)
			return;

		uint32_t byteOffset = m_vertexSize * numOffset;
		m_device->copyBuffer(other->m_handle.get(), m_handle.get(), byteOffset);
	}

}; // engi namespace