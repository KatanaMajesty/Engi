#pragma once

#include <string>
#include "Renderer/Texture2D.h"
#include "Renderer/TextureCube.h"
#include "Renderer/TextureLibrary.h"

namespace engi
{

	namespace gfx
	{
		class IGpuDevice;
		class IGpuTexture;
		class IGpuDescriptor;
	}
	
	enum CompressionFormat
	{
		COMPRESSION_NONE,
		COMPRESSION_BC1_LINEAR,			// RGB, 1 bit Alpha
		COMPRESSION_BC1_SRGB,		// RGB, 1-bit Alpha, SRGB
		COMPRESSION_BC3_LINEAR,			// RGBA
		COMPRESSION_BC3_SRGB,		// RGBA, SRGB
		COMPRESSION_BC4_UNSIGNED,		// GRAY, unsigned
		COMPRESSION_BC4_SIGNED,			// GRAY, signed
		COMPRESSION_BC5_UNSIGNED,		// RG, unsigned
		COMPRESSION_BC5_SIGNED,			// RG, signed
		COMPRESSION_BC6_UNSIGNED,		// RGB HDR, unsigned
		COMPRESSION_BC6_SIGNED,			// RGB HDR, signed
		COMPRESSION_BC7_LINEAR,			// RGBA Advanced
		COMPRESSION_BC7_SRGB,		// RGBA Advanced, SRGB
	};

	class TextureLoader
	{
	public:
		TextureLoader(gfx::IGpuDevice* device, TextureLibrary* textureLibrary);
		~TextureLoader() = default;

		Texture2D* loadTextureAtlas(const std::string& filepath, uint32_t numWidthTextures, uint32_t numHeightTextures, bool requestSrv) noexcept;
		Texture2D* loadTexture2D(const std::string& filepath, bool requestSrv) noexcept;
		TextureCube* loadTextureCube(const std::string& filepath, bool requestSrv) noexcept;
		bool saveToFile(Texture2D* texture, const std::string& filepath, bool mips, CompressionFormat format = COMPRESSION_NONE) noexcept;
		bool saveToFile(TextureCube* texture, const std::string& filepath, bool mips, CompressionFormat format = COMPRESSION_NONE) noexcept;
		TextureLibrary* getLibrary() noexcept { return m_textureLibrary; }

	private:
		gfx::IGpuTexture* loadGPUTextureFromDDS(const std::string& filepath) noexcept;
		bool saveGPUTextureToDDS(gfx::IGpuTexture* texture, const std::string& filepath, bool mips, CompressionFormat format) noexcept;

		gfx::IGpuDevice* m_device;
		TextureLibrary* m_textureLibrary;
	};

}; // engi namespace