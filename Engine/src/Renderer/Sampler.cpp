#include "Sampler.h"

#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "GFX/GPUSampler.h"
#include "GFX/GPUDevice.h"

namespace engi
{

	Sampler::Sampler(const std::string& name, gfx::IGpuDevice* device)
		: m_name(name)
		, m_device(device)
	{
		ENGI_ASSERT(device);
	}

	Sampler::~Sampler()
	{
		// To make UniqueHandle destruction work
	}

	bool Sampler::init(const gfx::GpuSamplerDesc& desc) noexcept
	{
		gfx::IGpuSampler* sampler = m_device->createSampler(m_name, desc);
		if (!sampler)
		{
			ENGI_LOG_WARN("Failed to create sampler. Device allocation failed");
			return false;
		}

		m_handle = gfx::makeGpuHandle(sampler, m_device->getResourceAllocator());
		return true;
	}

	void Sampler::bind(uint32_t slot) noexcept
	{
		ENGI_ASSERT(m_handle);
		m_device->setSampler(m_handle.get(), slot, gfx::PIXEL_SHADER | gfx::COMPUTE_SHADER);
	}

}; // engi namespace