#pragma once

#include <filesystem>
#include <unordered_map>
#include "Renderer/Texture2D.h"
#include "Renderer/TextureCube.h"

namespace engi
{

	namespace gfx { class IGpuDevice; }

	class TextureLibrary
	{
	public:
		TextureLibrary(gfx::IGpuDevice* device);

		Texture2D* createTexture2D(const std::string& name) noexcept;
		TextureCube* createTextureCube(const std::string& name) noexcept;
		Texture2D* getTexture2D(const std::string& name) noexcept;
		TextureCube* getTextureCube(const std::string& name) noexcept;
		bool removeTexture2D(const std::string& name) noexcept;
		bool removeTextureCube(const std::string& name) noexcept;

		// TODO: Temporary solution for TextureLoader::loadTextureAtlas. Texture2D* must be created with new keyword
		void addTexture2D(const std::string& name, Texture2D* texture) noexcept;

	private:
		gfx::IGpuDevice* m_device;
		std::unordered_map<std::string, UniqueHandle<TextureCube>> m_textureCubes;
		std::unordered_map<std::string, UniqueHandle<Texture2D>> m_texture2Ds;
	};

}; // engi namespace