#pragma once

#include <array>
#include <vector>
#include "Utility/ArrayView.h"
#include "Utility/Optional.h"
#include "Math/Math.h"
#include "Renderer/Model.h"
#include "Renderer/StaticMeshInstance.h"
#include "Renderer/MeshManager.h"
#include "Renderer/InstanceTable.h"
#include "World/LightSystem/SpotLight.h"
#include "World/LightSystem/PointLight.h"
#include "World/ParticleSystem/ParticleSystem.h"

namespace engi
{

	struct InstanceIntersection
	{
		inline constexpr bool isValid() const noexcept { return meshIndex != uint32_t(-1) && meshIntersection.isValid(); }
		inline constexpr bool operator<(const InstanceIntersection& other) const noexcept { return this->meshIntersection < other.meshIntersection; }
		inline constexpr bool operator>(const InstanceIntersection& other) const noexcept { return this->meshIntersection > other.meshIntersection; }

		uint32_t meshIndex = uint32_t(-1);
		StaticMeshIntersection meshIntersection;
	};

	class SceneRenderer;

	class ModelInstance
	{
	public:
		ModelInstance(SceneRenderer* sceneRenderer, const std::string& name = "Unknown model instance");
		~ModelInstance();

		bool init(const SharedHandle<Model>& model, ArrayView<const MaterialInstance> materials, const InstanceData& data) noexcept;
		InstanceData& getData() noexcept;
		const InstanceData& getData() const noexcept;
		inline constexpr uint32_t getInstanceID() const noexcept { return m_instanceID; }

		MaterialInstance getMaterialInstance(uint32_t meshIndex) const noexcept;
		void setMaterialInstance(uint32_t meshIndex, const MaterialInstance& materialInstance) noexcept;
		uint32_t getNumMeshInstances() const noexcept { return static_cast<uint32_t>(m_meshInstances.size()); }
		const auto& getMeshInstances() const noexcept { return m_meshInstances; }
		void updateMeshData() noexcept;

		math::AABB getAABB() const noexcept;

		void addPointLight(const math::Vec3& color, float intensity, float radius) noexcept;
		bool hasPointLight() const noexcept;
		PointLight* getPointLight() noexcept;
		void updatePointLight() noexcept; // Updates GPU part of a point light

		void addSpotLight(const math::Vec3& color, float intensity, float radius) noexcept;
		bool hasSpotLight() const noexcept;
		SpotLight* getSpotLight() noexcept;
		void updateSpotLight() noexcept; // Updates GPU part of a spot light

		void addParticleEmitter(Texture2D* mvea, Texture2D* dbf, Texture2D* rlu) noexcept;
		bool hasParticleEmitter() const noexcept;
		SmokeEmitter* getParticleEmitter() noexcept;
		void updateParticleEmitter() noexcept; // Updates GPU part of a particle emitter

		void addDecal(Texture2D* normalMap, const math::Vec3& albedo, const math::Mat4x4& decalToWorld) noexcept;
		void removeAllDecals() noexcept;
		inline constexpr uint32_t getNumDecals() const noexcept { return static_cast<uint32_t>(m_decalIDs.size()); }
		
		const std::string& getName() const noexcept { return m_name; }
		SharedHandle<Model> getModel() const noexcept { return m_model; }
		InstanceIntersection intersect(const math::Ray& ray) const noexcept;

	private:
		std::string m_name;
		SceneRenderer* m_sceneRenderer;

		SharedHandle<Model> m_model = nullptr;
		std::vector<StaticMeshInstance> m_meshInstances;
		
		uint32_t m_instanceID = uint32_t(-1);
		uint32_t m_spotLightID = uint32_t(-1);
		uint32_t m_pointLightID = uint32_t(-1);
		uint32_t m_particleEmitterID = uint32_t(-1);
		std::vector<uint32_t> m_decalIDs;
	};

}; // engi namespace