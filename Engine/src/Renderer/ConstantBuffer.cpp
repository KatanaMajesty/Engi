#include "Renderer/ConstantBuffer.h"

#include "Core/CommonDefinitions.h"

namespace engi
{

	using namespace gfx;

	ConstantBuffer::ConstantBuffer(const std::string& name, IGpuDevice* device)
		: m_name(name)
		, m_device(device)
	{
	}

	bool ConstantBuffer::initialize(uint32_t size)
	{
		ENGI_ASSERT(m_device && "Logical device was nullptr in ConstantBuffer::initialize");
		ENGI_ASSERT(size % 16 == 0 && "Constant buffer alignment should be equal to 16");

		GpuBufferDesc desc;
		desc.usage = GpuUsage::DYNAMIC;
		desc.bytes = size;
		desc.pipelineFlags = GpuBinding::CONSTANT_BUFFER;
		desc.cpuFlags = CpuAccess::WRITE;
		desc.byteStride = 0;
		m_buffer = makeGpuHandle(m_device->createBuffer("ConstantBuffer_" + m_name, desc, nullptr), m_device->getResourceAllocator());
		if (!m_buffer)
		{
			return false;
		}

		return true;
	}

	void ConstantBuffer::bind(uint32_t slot, uint32_t shaderTypes)
	{
		ENGI_ASSERT(m_buffer && "Constant buffer was not initialized correctly");
		m_device->setConstantBuffer(m_buffer.get(), slot, shaderTypes);
	}

	void ConstantBuffer::upload(uint32_t offset, const void* data, uint32_t size)
	{
		void* mapping;
		m_device->mapBuffer(m_buffer.get(), &mapping);
		if (!mapping)
			return;

		uint8_t* dst = reinterpret_cast<uint8_t*>(mapping) + offset;
		std::memcpy(dst, data, size);
		m_device->unmapBuffer(m_buffer.get());
	}

}; // engi namespace