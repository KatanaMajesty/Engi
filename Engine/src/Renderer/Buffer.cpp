#include "Renderer/Buffer.h"

#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "GFX/Definitions.h"
#include "GFX/GPUDevice.h"
#include "GFX/GPUBuffer.h"
#include "GFX/GPUDescriptor.h"

namespace engi
{

	Buffer::Buffer(const std::string& name, gfx::IGpuDevice* device)
		: m_name(name)
		, m_device(device)
	{
		ENGI_ASSERT(device && "Logical device cannot be nullptr");
	}

	Buffer::~Buffer()
	{
	}

	bool Buffer::init(const void* data, uint32_t numElements, uint32_t elementStride, gfx::GpuFormat format) noexcept
	{
		using namespace gfx;

		GpuResourceAllocator* gpuAllocator = m_device->getResourceAllocator();
		uint32_t bytes = numElements * elementStride;

		GpuBufferDesc desc;
		desc.usage = GpuUsage::DEFAULT; 
		desc.bytes = bytes;
		desc.pipelineFlags = GpuBinding::SHADER_RESOURCE | GpuBinding::UNORDERED_ACCESS;
		desc.cpuFlags = CpuAccess::ACCESS_UNUSED; // We cannot use CpuAccess flags with MISC_INDIRECT_ARGS
		desc.otherFlags = GpuMisc::MISC_INDIRECT_ARGS;

		// This is not a StructuredBuffer, thus this field will be ignored by GPUDevice
		// Although we will store the stride here for the convenience and flexibility of our Buffer's API
		desc.byteStride = elementStride;
		m_handle = makeGpuHandle(m_device->createBuffer("Buffer_" + m_name, desc, data), gpuAllocator);
		if (!m_handle)
		{
			ENGI_LOG_WARN("Failed to create buffer handle for buffer {}", m_name);
			return false;
		}

		// Create SRV
		GpuSrvDesc srvDesc;
		srvDesc.type = GpuResourceType::RESOURCE_BUFFER;
		srvDesc.format = format;
		srvDesc.buffer.offset = 0;
		srvDesc.buffer.size = desc.bytes;
		m_srv = makeGpuHandle(m_device->createSRV("Buffer_Srv::" + m_name, srvDesc, m_handle.get()), gpuAllocator);
		if (!m_srv)
		{
			ENGI_LOG_WARN("Failed to create SRV for buffer {}", m_name);
			return false;
		}

		// Create UAV
		GpuUavDesc uavDesc;
		uavDesc.type = GpuResourceType::RESOURCE_BUFFER;
		uavDesc.format = format;
		uavDesc.buffer.firstElement = 0;
		uavDesc.buffer.numElements = numElements;
		m_uav = makeGpuHandle(m_device->createUAV("Buffer_Uav::" + this->getName(), uavDesc, m_handle.get()), gpuAllocator);
		if (!m_uav)
		{
			ENGI_LOG_WARN("Failed to create UAV for buffer {}", m_name);
			return false;
		}

		return true;
	}

	bool Buffer::initAsStructured(const void* data, uint32_t numElements, uint32_t elementStride) noexcept
	{
		ENGI_ASSERT(elementStride > 0 && elementStride < 2048 && "Stride should be greater than zero and less than 2048");
		ENGI_ASSERT(elementStride % 4 == 0 && "Stride should be multiple of 4");
		using namespace gfx;

		GpuResourceAllocator* gpuAllocator = m_device->getResourceAllocator();
		uint32_t bytes = numElements * elementStride;

		GpuBufferDesc desc;
		desc.usage = GpuUsage::DEFAULT;
		desc.bytes = bytes;
		desc.pipelineFlags = GpuBinding::SHADER_RESOURCE | GpuBinding::UNORDERED_ACCESS;
		desc.cpuFlags = CpuAccess::WRITE;
		desc.byteStride = elementStride;
		desc.otherFlags = GpuMisc::MISC_STRUCTURED_BUFFER;
		m_handle = makeGpuHandle(m_device->createBuffer("StructuredBuffer_" + m_name, desc, data), gpuAllocator);
		if (!m_handle)
		{
			ENGI_LOG_WARN("Failed to create buffer handle for structured buffer {}", m_name);
			return false;
		}

		// Create SRV
		GpuSrvDesc srvDesc;
		srvDesc.type = GpuResourceType::RESOURCE_BUFFER;
		srvDesc.format = GpuFormat::FORMAT_UNKNOWN;
		srvDesc.buffer.offset = 0;
		srvDesc.buffer.size = bytes;
		m_srv = makeGpuHandle(m_device->createSRV("StructuredBuffer_Srv::" + m_name, srvDesc, m_handle.get()), gpuAllocator);
		if (!m_srv)
		{
			ENGI_LOG_WARN("Failed to create SRV for structured buffer {}", m_name);
			return false;
		}

		// Create UAV
		GpuUavDesc uavDesc;
		uavDesc.type = GpuResourceType::RESOURCE_BUFFER;
		uavDesc.format = GpuFormat::FORMAT_UNKNOWN;
		uavDesc.buffer.firstElement = 0;
		uavDesc.buffer.numElements = numElements;
		m_uav = makeGpuHandle(m_device->createUAV("StructuredBuffer_Uav::" + this->getName(), uavDesc, m_handle.get()), gpuAllocator);
		if (!m_uav)
		{
			ENGI_LOG_WARN("Failed to create UAV for structured buffer {}", m_name);
			return false;
		}

		return true;
	}

	bool Buffer::initAsByteAddress(const uint32_t* data, uint32_t numElements) noexcept
	{
		using namespace gfx;

		GpuResourceAllocator* gpuAllocator = m_device->getResourceAllocator();
		uint32_t bytes = numElements * sizeof(uint32_t);

		GpuBufferDesc desc;
		desc.usage = GpuUsage::DEFAULT;
		desc.bytes = bytes;
		desc.pipelineFlags = GpuBinding::SHADER_RESOURCE | GpuBinding::UNORDERED_ACCESS;
		desc.cpuFlags = CpuAccess::WRITE;

		// This is not a StructuredBuffer, thus this field will be ignored by GPUDevice
		// Although we will store the stride here for the convenience and flexibility of our Buffer's API
		desc.byteStride = sizeof(uint32_t);
		desc.otherFlags = GpuMisc::MISC_BYTEADDRESS_BUFFER;
		m_handle = makeGpuHandle(m_device->createBuffer("ByteAddressBuffer_" + m_name, desc, data), gpuAllocator);
		if (!m_handle)
		{
			ENGI_LOG_WARN("Failed to create ByteAddressBuffer handle for buffer {}", m_name);
			return false;
		}

		// Create SRV
		GpuSrvDesc srvDesc;
		srvDesc.type = GpuResourceType::RESOURCE_BUFFER;
		srvDesc.format = GpuFormat::R32T; // Required (by D3D11 atleast)
		srvDesc.buffer.offset = 0;
		srvDesc.buffer.size = desc.bytes;
		m_srv = makeGpuHandle(m_device->createSRV("ByteAddressBuffer_Srv::" + m_name, srvDesc, m_handle.get()), gpuAllocator);
		if (!m_srv)
		{
			ENGI_LOG_WARN("Failed to create SRV for ByteAddressBuffer buffer {}", m_name);
			return false;
		}

		// Create UAV
		GpuUavDesc uavDesc;
		uavDesc.type = GpuResourceType::RESOURCE_BUFFER;
		uavDesc.format = GpuFormat::R32T;
		uavDesc.buffer.firstElement = 0;
		uavDesc.buffer.numElements = numElements;
		m_uav = makeGpuHandle(m_device->createUAV("ByteAddressBuffer_Uav::" + this->getName(), uavDesc, m_handle.get()), gpuAllocator);
		if (!m_uav)
		{
			ENGI_LOG_WARN("Failed to create UAV for ByteAddressBuffer {}", m_name);
			return false;
		}

		return true;
	}

	void Buffer::upload(uint32_t numBufferOffset, const void* data, uint32_t numElements) noexcept
	{
		ENGI_ASSERT(m_handle);

		// this is guaranteed to be non-zero if the buffer was initialized correctly
		uint32_t elementStride = this->getBufferStride();
		ENGI_ASSERT(elementStride > 0);

		uint32_t bufferNumElements = this->getBufferBytes() / elementStride;
		m_device->updateBuffer(m_handle.get(), numBufferOffset * elementStride, data, numElements * elementStride);
	}

	void Buffer::copyFrom(const Buffer* other, uint32_t byteOffset) noexcept
	{
		ENGI_ASSERT(other && "Buffer cannot be nullptr");
		if (!m_handle || !other->m_handle)
			return;

		if (m_device != other->m_device)
			return;

		if (this->isStructuredBuffer())
		{
			ENGI_ASSERT(other->isStructuredBuffer());
			uint32_t otherElementSize = other->m_handle->getDesc().byteStride;
			uint32_t elementSize = m_handle->getDesc().byteStride;
			if (otherElementSize == 0 || otherElementSize != elementSize)
				return;

			uint32_t buffersNumElements = this->getBufferBytes() / elementSize;
			ENGI_ASSERT(byteOffset < (elementSize * buffersNumElements));
		}

		m_device->copyBuffer(other->m_handle.get(), m_handle.get(), byteOffset);
	}

	bool Buffer::isStructuredBuffer() const noexcept
	{
		using namespace gfx;

		ENGI_ASSERT(m_handle);
		return (m_handle->getDesc().otherFlags & GpuMisc::MISC_STRUCTURED_BUFFER) != 0;
	}

	bool Buffer::isByteAddressBuffer() const noexcept
	{
		using namespace gfx;

		ENGI_ASSERT(m_handle);
		return (m_handle->getDesc().otherFlags & GpuMisc::MISC_BYTEADDRESS_BUFFER) != 0;
	}

	void Buffer::bind(uint32_t slot, uint32_t shaderTypes) const noexcept
	{
		m_device->setSRV(m_srv.get(), slot, shaderTypes);
	}

	uint32_t Buffer::getBufferBytes() const noexcept { return m_handle->getDesc().bytes; }
	uint32_t Buffer::getBufferStride() const noexcept { return m_handle->getDesc().byteStride; }
	uint32_t Buffer::getNumElements() const noexcept { return this->getBufferBytes() / this->getBufferStride(); }

}; // engi namespace