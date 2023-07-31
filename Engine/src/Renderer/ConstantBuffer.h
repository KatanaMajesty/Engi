#pragma once

#include "GFX/GPUBuffer.h"
#include "GFX/GPUDevice.h"
#include "GFX/GPUResourceAllocator.h"

namespace engi
{

	class ConstantBuffer
	{
	public:
		ConstantBuffer(const std::string& name, gfx::IGpuDevice* device);
		ConstantBuffer(const ConstantBuffer&) = delete;
		ConstantBuffer& operator=(const ConstantBuffer&) = delete;
		~ConstantBuffer() = default;

		bool initialize(uint32_t size);
		void bind(uint32_t slot, uint32_t shaderTypes);
		void upload(uint32_t offset, const void* data, uint32_t size);

	private:
		std::string m_name;
		gfx::IGpuDevice* m_device;
		gfx::GpuHandle<gfx::IGpuBuffer> m_buffer = nullptr;
	}; // ConstantBuffer class

}; // engi namespace