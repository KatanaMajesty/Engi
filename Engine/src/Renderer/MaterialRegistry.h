#pragma once

#include <string>
#include <unordered_map>
#include "Utility/Memory.h"
#include "Renderer/Material.h"

namespace engi
{

	namespace gfx
	{
		class IGpuDevice;
	}
	class ShaderLibrary;

	// MaterialType enum represents all materials that are handled by Engi internally and may be used by user
	enum MaterialType
	{
		MATERIAL_HOLOGRAM = 0,
		MATERIAL_NORMALS = 1,
		MATERIAL_ALBEDO_COLOR = 2,
		MATERIAL_EMISSIVE = 3,
		MATERIAL_BRDF_PBR = 4,
		MATERIAL_BRDF_PBR_NO_CULLING = 5, // This is temporary solution for meshes that should not use backface culling
		MATERIAL_BRDF_PBR_DISSOLUTION = 6,
		MATERIAL_BRDF_PBR_INCINERATION = 7,
	};

	class MaterialRegistry
	{
	public:
		MaterialRegistry(gfx::IGpuDevice* device, ShaderLibrary* shaderLibrary);
		MaterialRegistry(const MaterialRegistry&) = default;
		MaterialRegistry& operator=(const MaterialRegistry&) = default;
		~MaterialRegistry();

		bool init() noexcept;
		SharedHandle<Material> registerMaterial(const std::string& name) noexcept;
		SharedHandle<Material> getMaterial(const std::string& name) noexcept;
		SharedHandle<Material> getMaterial(MaterialType type) noexcept;
		inline constexpr const auto& getAllMaterials() const noexcept { return m_materials; }

	private:
		gfx::IGpuDevice* m_device;
		ShaderLibrary* m_shaderLibrary;
		std::unordered_map<std::string, SharedHandle<Material>> m_materials;
	};

}; // engi namespace