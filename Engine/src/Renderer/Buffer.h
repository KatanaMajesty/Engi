#pragma once

#include <cstdint>
#include "GFX/GPUResourceAllocator.h"
#include "GFX/Definitions.h"

namespace engi
{

	namespace gfx
	{
		class IGpuDevice;
		class IGpuBuffer;
		class IGpuDescriptor;
	}

	enum BufferType
	{

	};

	class Buffer
	{
	public:
		Buffer(const std::string& name, gfx::IGpuDevice* device);
		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
		~Buffer();

		// Buffer::init() will initialize a RWBuffer. This buffer will also be suitable for Indirect Drawcalls provided by Renderer (and GpuDevice)
		// Notice! Other buffers (StructuredBuffer and ByteAddressBuffer) cannot be used in Indirect Drawcalls
		bool init(const void* data, uint32_t numElements, uint32_t elementStride, gfx::GpuFormat format) noexcept;
		bool initAsStructured(const void* data, uint32_t numElements, uint32_t elementStride) noexcept;
		bool initAsByteAddress(const uint32_t* data, uint32_t numElements) noexcept;

		// Buffer will use its own stride, that is stored inside of it
		void upload(uint32_t numBufferOffset, const void* data, uint32_t numElements) noexcept;
		void copyFrom(const Buffer* other, uint32_t byteOffset) noexcept;

		bool isStructuredBuffer() const noexcept;
		bool isByteAddressBuffer() const noexcept;

		void bind(uint32_t slot, uint32_t shaderTypes) const noexcept;
		gfx::IGpuDescriptor* getUav() noexcept { return m_uav.get(); }

		uint32_t getBufferBytes() const noexcept;
		uint32_t getBufferStride() const noexcept;
		uint32_t getNumElements() const noexcept;
		const std::string& getName() const noexcept { return m_name; }

		gfx::IGpuBuffer* getHandle() noexcept { return m_handle.get(); }

	private:
		std::string m_name;
		gfx::IGpuDevice* m_device;
		gfx::GpuHandle<gfx::IGpuBuffer> m_handle = nullptr;
		gfx::GpuHandle<gfx::IGpuDescriptor> m_srv = nullptr;
		gfx::GpuHandle<gfx::IGpuDescriptor> m_uav = nullptr;
	};

}; // engi namespace