#pragma once

#include <array>
#include "Utility/Memory.h"
#include "Renderer/Material.h"
#include "Math/Vec3.h"

namespace engi
{

	namespace gfx { class IGpuDescriptor; }
	class Texture2D;

	enum TextureType
	{
		TEXTURE_ALBEDO = 0,
		TEXTURE_NORMAL = 1,
		TEXTURE_METALNESS = 2,
		TEXTURE_ROUGHNESS = 3,
	};

	struct MaterialConstant
	{
		float metallic = 0.0f;
		float roughness = 0.0f;
		bool useAlbedoTexture = false;
		bool useNormalMap = false;
		bool useMetalnessMap = false;
		bool useRoughnessMap = false;
	};

	struct MaterialInstance
	{
		static inline MaterialInstance empty() noexcept { return MaterialInstance("Empty material instance", nullptr); }

		MaterialInstance(const std::string& name, const SharedHandle<Material>& material, const MaterialConstant& data = MaterialConstant());
		~MaterialInstance() = default;

		constexpr void setRoughness(float roughness) noexcept { m_materialData.roughness = roughness; }
		constexpr void setMetallic(float metallic) noexcept { m_materialData.metallic = metallic; }
		constexpr void setUseAlbedoTexture(bool val) noexcept { m_materialData.useAlbedoTexture = val; }
		constexpr void setUseNormalMap(bool val) noexcept { m_materialData.useNormalMap = val; }
		constexpr void setUseMetalnessMap(bool val) noexcept { m_materialData.useMetalnessMap = val; }
		constexpr void setUseRoughnessMap(bool val) noexcept { m_materialData.useRoughnessMap = val; }
		constexpr auto getName() const noexcept -> const std::string& { return m_name; }
		constexpr auto getRoughness() const noexcept -> float { return m_materialData.roughness; }
		constexpr auto getData() const noexcept -> const MaterialConstant& { return m_materialData; }
		constexpr auto getData() noexcept -> MaterialConstant& { return m_materialData; }
		constexpr bool isAlbedoTextureUsed() const noexcept { return m_materialData.useAlbedoTexture; }
		constexpr bool isNormalMapUsed() const noexcept { return m_materialData.useNormalMap; }
		constexpr bool isMetalnessMapUsed() const noexcept { return m_materialData.useMetalnessMap; }
		constexpr bool isRoughnessMapUsed() const noexcept { return m_materialData.useRoughnessMap; }
		bool isEmpty() const noexcept { return m_material == nullptr; }
		void setMaterial(const SharedHandle<Material>& material) noexcept { m_material = material; }
		auto getMaterial() const noexcept -> SharedHandle<Material> { return m_material; }
		void setTexture(TextureType type, Texture2D* texture) noexcept;
		void bindTexture(TextureType type, uint32_t slot, uint32_t shaderTypes) const noexcept;
		bool operator==(const MaterialInstance& other) const noexcept;

	private:
		std::string m_name;
		std::array<Texture2D*, 4> m_textures{};
		SharedHandle<Material> m_material;
		MaterialConstant m_materialData;
	};

}; // engi namespace