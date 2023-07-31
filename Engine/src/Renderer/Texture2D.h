#pragma once

#include <string>
#include "GFX/Definitions.h"
#include "GFX/GPUResourceAllocator.h"

namespace engi
{

	namespace gfx
	{
		class IGpuDevice;
		class IGpuDescriptor;
		class IGpuTexture;
	}

	struct Texture2DCopyState
	{
		uint32_t dstMipslice = 0;
		uint32_t dstArrayslice = 0;
		uint32_t dstX = 0;
		uint32_t dstY = 0;

		uint32_t srcMipslice = 0;
		uint32_t srcArrayslice = 0;
		uint32_t srcX = 0;
		uint32_t srcY = 0;

		uint32_t width = 0;
		uint32_t height = 0;
	};

	class Texture2D
	{
	public:
		Texture2D(const std::string& name, gfx::IGpuDevice* device);
		~Texture2D();

		// takes ownership over the handle
		bool init(gfx::IGpuTexture* handle) noexcept;
		bool init(uint32_t width, uint32_t height, uint32_t mips, uint32_t arraySize, gfx::GpuFormat format, 
			uint32_t pipelineFlags = gfx::GpuBinding::RENDER_TARGET | gfx::GpuBinding::SHADER_RESOURCE,
			gfx::GpuUsage usage = gfx::GpuUsage::DEFAULT, uint32_t cpuFlags = gfx::CpuAccess::ACCESS_UNUSED) noexcept;

		// If format == FORMAT_UNKNOWN the format from m_handle will be used
		bool initShaderView(gfx::GpuFormat format = gfx::GpuFormat::FORMAT_UNKNOWN) noexcept;
		void bindShaderView(uint32_t slot, uint32_t shaderTypes) noexcept;
		inline bool hasShaderView() const noexcept { return m_srv != nullptr; }

		void copyFrom(Texture2D* srcTexture, const Texture2DCopyState& copyState) noexcept;

		auto getHandle() noexcept { return m_handle.get(); }
		const std::string& getName() const noexcept { return m_name; }
		uint32_t getNumMips() const noexcept;
		uint32_t getMaxMips() const noexcept;
		uint32_t getWidth() const noexcept;
		uint32_t getHeight() const noexcept;
		uint32_t getArraySize() const noexcept;

	private:
		friend class TextureLoader;

		std::string m_name;
		gfx::IGpuDevice* m_device;
		gfx::GpuHandle<gfx::IGpuTexture> m_handle = nullptr;
		gfx::GpuHandle<gfx::IGpuDescriptor> m_srv = nullptr;
	};

}; // engi namespace