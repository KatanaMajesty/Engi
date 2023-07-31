#pragma once

#include <unordered_map>
#include "Utility/Memory.h"
#include "Utility/SolidVector.h"
#include "World/LightSystem/DirectionalLight.h"
#include "World/LightSystem/PointLight.h"
#include "World/LightSystem/SpotLight.h"
#include "Renderer/Material.h"

namespace engi
{

	class Renderer;
	class InstanceTable;
	class Buffer;

	class LightManager
	{
	public:
		LightManager(Renderer* renderer);
		LightManager(const LightManager&) = delete;
		LightManager& operator=(const LightManager&) = delete;
		~LightManager();

		bool init() noexcept;
		void update() noexcept;

		uint32_t createDirLight(const math::Vec3& color, const math::Vec3& direction, float radius) noexcept;
		uint32_t createPointLight(const math::Vec3& color, float intensity, float radius) noexcept;
		uint32_t createSpotLight(const math::Vec3& color, float intensity, float radius) noexcept;
		
		auto& getDirLight(uint32_t ID) noexcept { ENGI_ASSERT(m_dirLights.isOccupied(ID)); return m_dirLights[ID]; }
		auto& getSpotLight(uint32_t ID) noexcept { ENGI_ASSERT(m_spotLights.isOccupied(ID)); return m_spotLights[ID]; }
		auto& getPointLight(uint32_t ID) noexcept { ENGI_ASSERT(m_pointLights.isOccupied(ID)); return m_pointLights[ID]; }

		void removeDirLight(uint32_t ID) noexcept;
		void removeSpotLight(uint32_t ID) noexcept;
		void removePointLight(uint32_t ID) noexcept;
		
		auto& getAllDirLights() noexcept { return m_dirLights; }
		auto& getAllSpotLights() noexcept { return m_spotLights; }
		auto& getAllPointLights() noexcept { return m_pointLights; }

		inline bool& isShadowmappingEnabled() noexcept { return m_shadowMapping; }
		inline const bool& isShadowmappingEnabled() const noexcept { return m_shadowMapping; }
		inline void useShadowmapping(bool val) noexcept { m_shadowMapping = val; }

		Texture2D* getDirectionalDepthmapArray() noexcept;
		Texture2D* getSpotDepthmapArray() noexcept;
		TextureCube* getPointDepthmapArray() noexcept;

		inline constexpr uint32_t getNumDirLights() const noexcept { return m_dirLights.size(); }
		inline constexpr uint32_t getNumSpotLights() const noexcept { return m_spotLights.size(); }
		inline constexpr uint32_t getNumPointLights() const noexcept { return m_pointLights.size(); }
		
		inline constexpr uint32_t getMaxDirLights() const noexcept { return 1; }
		inline constexpr uint32_t getMaxSpotLights() const noexcept { return 8; }
		inline constexpr uint32_t getMaxPointLights() const noexcept { return 8; }
		
		Buffer* getDirLightBuffer() noexcept { return m_dirLightBuffer.get(); }
		Buffer* getSpotLightBuffer() noexcept { return m_spotLightBuffer.get(); }
		Buffer* getPointLightBuffer() noexcept { return m_pointLightBuffer.get(); }

	private:
		bool createDirLightArray() noexcept;
		bool createSpotLightArray() noexcept;
		bool createPointLightArray() noexcept;

		void updateBuffer(const DirectionalLight* lights, uint32_t numLights, uint32_t offset);
		void updateBuffer(const PointLight* lights, uint32_t numLights, uint32_t offset);
		void updateBuffer(const SpotLight* lights, uint32_t numLights, uint32_t offset);

		Renderer* m_renderer;
		UniqueHandle<Buffer> m_dirLightBuffer;
		UniqueHandle<Buffer> m_spotLightBuffer;
		UniqueHandle<Buffer> m_pointLightBuffer;

		SolidVector<DirectionalLight> m_dirLights;
		SolidVector<SpotLight> m_spotLights;
		SolidVector<PointLight> m_pointLights;
		
		bool m_shadowMapping = true;
	};

}; // engi namespace