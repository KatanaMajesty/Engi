#include "Renderer/TextureLibrary.h"

#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "GFX/GPUDevice.h"

namespace engi
{

	TextureLibrary::TextureLibrary(gfx::IGpuDevice* device)
		: m_device(device)
	{
		ENGI_ASSERT(device && "Logical gpu device cannot be nullptr");
	}

	Texture2D* TextureLibrary::createTexture2D(const std::string& name) noexcept
	{
		Texture2D* texture = getTexture2D(name);
		if (texture)
			return texture;

		texture = new Texture2D(name, m_device);
		m_texture2Ds[name] = makeUnique<Texture2D>(texture);
		return texture;
	}

	TextureCube* TextureLibrary::createTextureCube(const std::string& name) noexcept
	{
		TextureCube* texture = getTextureCube(name);
		if (texture)
			return texture;

		texture = new TextureCube(name, m_device);
		m_textureCubes[name] = makeUnique<TextureCube>(texture);
		return texture;
	}

	Texture2D* TextureLibrary::getTexture2D(const std::string& name) noexcept
	{
		auto it = m_texture2Ds.find(name);
		return it == m_texture2Ds.end() ? nullptr : it->second.get();
	}

	TextureCube* TextureLibrary::getTextureCube(const std::string& name) noexcept
	{
		auto it = m_textureCubes.find(name);
		return it == m_textureCubes.end() ? nullptr : it->second.get();
	}

	bool TextureLibrary::removeTexture2D(const std::string& name) noexcept
	{
		return m_texture2Ds.erase(name) > 0;
	}

	bool TextureLibrary::removeTextureCube(const std::string& name) noexcept
	{
		return m_textureCubes.erase(name) > 0;
	}

	void TextureLibrary::addTexture2D(const std::string& name, Texture2D* texture) noexcept
	{
		m_texture2Ds[name].reset(texture);
	}

}; // engi namespace