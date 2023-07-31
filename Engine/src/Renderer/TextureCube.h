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

	class TextureCube
	{
	public:
		TextureCube(const std::string& name, gfx::IGpuDevice* device);
		~TextureCube();

		bool init(gfx::IGpuTexture* handle) noexcept;
		bool init(uint32_t width, uint32_t height, uint32_t mips, uint32_t numCubes, gfx::GpuFormat format, 
			uint32_t pipelineFlags = gfx::GpuBinding::RENDER_TARGET | gfx::GpuBinding::SHADER_RESOURCE, 
			gfx::GpuUsage usage = gfx::GpuUsage::DEFAULT, uint32_t cpuFlags = gfx::CpuAccess::ACCESS_UNUSED) noexcept;

		// If format == FORMAT_UNKNOWN the format from m_handle will be used
		bool initCubeShaderView(gfx::GpuFormat format = gfx::GpuFormat::FORMAT_UNKNOWN) noexcept; // creates a TextureCube HLSL srv
		void bindCubeShaderView(uint32_t slot, uint32_t shaderTypes) noexcept;

		// This is only valid if TextureCube is not an array (arraysize == 1)
		// If format == FORMAT_UNKNOWN the format from m_handle will be used
		bool initArrayShaderView(gfx::GpuFormat format = gfx::GpuFormat::FORMAT_UNKNOWN) noexcept; // creates a Texture2D[6] HLSL srv
		void bindArrayShaderView(uint32_t slot, uint32_t shaderTypes) noexcept;

		inline bool hasCubeShaderView() const noexcept { return m_cubeSrv != nullptr; }
		inline bool hasArrayShaderView() const noexcept { return m_arraySrv != nullptr; }

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
		gfx::GpuHandle<gfx::IGpuDescriptor> m_cubeSrv = nullptr;
		gfx::GpuHandle<gfx::IGpuDescriptor> m_arraySrv = nullptr;
	};

}; // engi namespace