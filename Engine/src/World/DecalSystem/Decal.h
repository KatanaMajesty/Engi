#pragma once

#include "Math/Math.h"
#include "Utility/Memory.h"
#include "Renderer/Material.h"
#include "Renderer/MaterialInstance.h"
#include "Renderer/InstanceTable.h"
#include "Renderer/Texture2D.h"

namespace engi
{

	struct DecalInstance
	{
		DecalInstance() = default;

		inline constexpr bool isValid() const noexcept { return normalMap && instanceTable && parentInstanceID != uint32_t(-1); }

		void setNormalMap(Texture2D* texture) noexcept;
		void setParentInstance(InstanceTable* instanceTable, uint32_t parentInstanceID) noexcept;
		void setLocalSpaceTransform(const math::Mat4x4& decalToWorld) noexcept;

		math::Mat4x4 getWorldToDecal() const noexcept;
		math::Mat4x4 getDecalToWorld() const noexcept;
		
		inline constexpr void setEmissive(const math::Vec3& emissive) noexcept { this->emissive = emissive; }
		inline constexpr void setAlbedo(const math::Vec3& albedo) noexcept { this->albedo = albedo; }
		inline constexpr void setMetalness(float metalness) noexcept { this->metalness = metalness; }
		inline constexpr void setRoughness(float roughness) noexcept { this->roughness = roughness; }

		Texture2D* normalMap = nullptr;

		InstanceTable* instanceTable = nullptr; // InstanceTable may be used to access instance's data
		uint32_t parentInstanceID = uint32_t(-1);

		math::Vec3 albedo;
		math::Vec3 emissive;
		float metalness = 0.0f;
		float roughness = 0.0f;

		math::Mat4x4 decalToInstance;
		math::Mat4x4 instanceToDecal;
	};

}; // engi namespace