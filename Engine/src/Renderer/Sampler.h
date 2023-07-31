#pragma once

#include <string>
#include "GFX/Definitions.h"
#include "GFX/GPUResourceAllocator.h"

namespace engi
{

	namespace gfx
	{
		class IGpuDevice;
		class IGpuSampler;
	}

	class Sampler
	{
	public:
		Sampler(const std::string& name, gfx::IGpuDevice* device);
		~Sampler();

		bool init(const gfx::GpuSamplerDesc& desc) noexcept;
		void bind(uint32_t slot) noexcept;

		inline gfx::IGpuSampler* getHandle() noexcept { return m_handle.get(); }
		inline constexpr const std::string& getName() const noexcept { return m_name; }

	private:
		std::string m_name;
		gfx::IGpuDevice* m_device;
		gfx::GpuHandle<gfx::IGpuSampler> m_handle;
	};

}; // engi namespace