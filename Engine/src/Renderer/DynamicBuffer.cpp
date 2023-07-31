#include "Renderer/DynamicBuffer.h"

#include "Core/CommonDefinitions.h"
#include "GFX/GPUDevice.h"
#include "GFX/GPUBuffer.h"

namespace engi
{

	DynamicBuffer::DynamicBuffer(const std::string& name, gfx::IGpuDevice* device)
		: m_name(name)
		, m_device(device)
	{
	}

	bool DynamicBuffer::init(const void* data, uint32_t numVertices, uint32_t vertexSize) noexcept
	{
		using namespace gfx;
		if (!m_device)
			return false;

		m_vertexSize = vertexSize;

		GpuBufferDesc desc;
		desc.usage = GpuUsage::DYNAMIC;
		desc.bytes = numVertices * vertexSize;
		desc.pipelineFlags = GpuBinding::VERTEX_BUFFER;
		desc.cpuFlags = CpuAccess::WRITE;
		desc.byteStride = 0;
		m_handle = makeGpuHandle(m_device->createBuffer("DynamicBuffer_" + m_name, desc, data), m_device->getResourceAllocator());
		if (!m_handle)
		{
			return false;
		}

		return true;
	}

	void DynamicBuffer::bind(uint32_t slot, uint32_t numOffset) const noexcept
	{
		uint32_t byteOffset = numOffset * m_vertexSize;
		m_device->setVertexBuffer(m_handle.get(), slot, m_vertexSize, byteOffset);
	}

	void* DynamicBuffer::map() noexcept
	{
		if (!m_handle)
			return nullptr;

		void* mapping = nullptr;
		m_device->mapBuffer(m_handle.get(), &mapping);
		return mapping;
	}

	void DynamicBuffer::unmap() noexcept
	{
		if (!m_handle)
			return;

		m_device->unmapBuffer(m_handle.get());
	}

	void DynamicBuffer::copyFrom(const DynamicBuffer* other, uint32_t numOffset) noexcept
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