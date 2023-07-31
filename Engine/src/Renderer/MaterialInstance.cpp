#include "Renderer/MaterialInstance.h"

#include <algorithm>
#include "GFX/GPUDevice.h"
#include "Renderer/Texture2D.h"
#include "Renderer/TextureCube.h"

namespace engi
{

	MaterialInstance::MaterialInstance(const std::string& name, const SharedHandle<Material>& material, const MaterialConstant& data)
		: m_material(material)
		, m_name(name)
		, m_materialData(data)
	{
	}

	void MaterialInstance::setTexture(TextureType type, Texture2D* texture) noexcept
	{
		if (static_cast<size_t>(type) >= m_textures.size())
			return;

		switch (type)
		{
		case TEXTURE_ALBEDO: setUseAlbedoTexture(texture ? true : false); break;
		case TEXTURE_NORMAL: setUseNormalMap(texture ? true : false); break;
		case TEXTURE_METALNESS: setUseMetalnessMap(texture ? true : false); break;
		case TEXTURE_ROUGHNESS: setUseRoughnessMap(texture ? true : false); break;
		default: ENGI_ASSERT(false);
		}
		m_textures[type] = texture;
	}

	void MaterialInstance::bindTexture(TextureType type, uint32_t slot, uint32_t shaderTypes) const noexcept
	{
		Texture2D* texture = m_textures[type];
		if (!texture)
		{
			ENGI_ASSERT(m_material && m_material->m_device);
			m_material->m_device->setSRV(nullptr, slot, shaderTypes);
			return;
		}

		texture->bindShaderView(slot, shaderTypes);
	}

	bool MaterialInstance::operator==(const MaterialInstance& other) const noexcept
	{
		if (m_material != other.m_material)
			return false;

		if (m_materialData.roughness != other.m_materialData.roughness)
			return false;

		if (m_materialData.metallic != other.m_materialData.metallic)
			return false;

		if (m_materialData.useAlbedoTexture != other.m_materialData.useAlbedoTexture
			|| (m_materialData.useAlbedoTexture && m_textures[TEXTURE_ALBEDO] != other.m_textures[TEXTURE_ALBEDO]))
			return false;

		if (m_materialData.useNormalMap != other.m_materialData.useNormalMap
			|| (m_materialData.useNormalMap && m_textures[TEXTURE_NORMAL] != other.m_textures[TEXTURE_NORMAL]))
			return false;

		if (m_materialData.useMetalnessMap != other.m_materialData.useMetalnessMap
			|| (m_materialData.useMetalnessMap && m_textures[TEXTURE_METALNESS] != other.m_textures[TEXTURE_METALNESS]))
			return false;

		if (m_materialData.useRoughnessMap != other.m_materialData.useRoughnessMap
			|| (m_materialData.useRoughnessMap && m_textures[TEXTURE_ROUGHNESS] != other.m_textures[TEXTURE_ROUGHNESS]))
			return false;

		return true;
	}

}; // engi namespace