#pragma once

#include <vector>
#include "Math/Math.h"
#include "Utility/Memory.h"
#include "Utility/ArrayView.h"
#include "Core/InputEvent.h"
#include "Core/WindowEvent.h"

#include "World/ModelInstanceRegistry.h"
#include "World/InstanceDragger.h"
#include "World/Flashlight.h"
#include "World/CameraManager.h"
#include "World/ParticleSystem/ParticleSystem.h"
#include "World/LightSystem/DirectionalLight.h"
#include "World/SceneRenderer.h"

#include "Renderer/MaterialInstance.h"
#include "Renderer/TextureCube.h"
#include "Renderer/Texture2D.h"

namespace engi
{

	class Renderer;

	class Scene
	{
	public:
		Scene();
		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		~Scene();

		void init(Renderer* renderer, uint32_t width, uint32_t height);
		void onUpdate(float timestep, bool debugPass);
		void onResize(const EvWindowResized& ev) noexcept;
		void onKeyPressed(const EvKeyPressed& ev) noexcept;

		inline SceneRenderer* getSceneRenderer() noexcept { return m_sceneRenderer.get(); }

		void createCamera(const math::Vec3& pos = math::Vec3(0.0f, 0.5f, -2.0f), const math::Vec3& dir = math::Vec3(0.0f, 0.0f, 1.0f)) noexcept;
		void createFlashlight(const math::Vec3& color, float intensity, float radius, Texture2D* lightMask) noexcept;
		inline bool hasCamera() const noexcept { return m_cameraManager != nullptr; }
		inline bool hasFlashLight() const noexcept { return m_flashlight != nullptr && m_flashlight->isValid(); }
		inline CameraManager* getCameraManager() noexcept { return m_cameraManager.get(); }
		inline Flashlight* getFlashlight() noexcept { return m_flashlight.get(); }
		
		void setSkybox(TextureCube* texture) noexcept;
		void setDirLight(const math::Vec3& color, const math::Vec3& direction, float radius) noexcept;
		TextureCube* getSkybox() noexcept;
		DirectionalLight& getDirLight() noexcept;

		inline void setDissolutionTexture(Texture2D* texture) noexcept { m_sceneRenderer->setDissolutionTexture(texture); }
		inline void setDissolutionTime(float time) noexcept { m_sceneRenderer->setDissolutionTime(time); }
		inline void setIncinerationTime(float time) noexcept { m_sceneRenderer->setIncinerationTime(time); }
		inline constexpr float getDissolutionTime() const noexcept { return m_sceneRenderer->getDissolutionTime(); }
		inline constexpr float getIncinerationTime() const noexcept { return m_sceneRenderer->getIncinerationTime(); }

		uint32_t addInstance(const std::string& name, const SharedHandle<Model>& model, ArrayView<const MaterialInstance> materials, const InstanceData& instanceData) noexcept;
		uint32_t spawnInstance(const std::string& name, const SharedHandle<Model>& model, ArrayView<const MaterialInstance> materials, const math::Vec3& color) noexcept;
		void removeInstance(uint32_t modelInstanceID, const math::Vec3& hitpos) noexcept;

		ModelInstance* getModelInstanceByID(uint32_t modelInstanceID) noexcept;

		math::Transformation getTransformForInstanceSpawn(const Camera& camera) const noexcept;
		void copyMaterialInstanceForDissolution(std::vector<MaterialInstance>& dissolutionMaterials, ArrayView<const MaterialInstance> materials) const noexcept;
		
		inline bool& isIOBorrowed() noexcept { return m_ioBorrowed; }
		inline bool& isTimePaused() noexcept { return m_timePaused; }
		inline bool isIOAvailable() const noexcept { return !m_ioBorrowed; }
		inline bool isTimePaused() const noexcept { return m_timePaused; }

	private:
		bool isIncineratableMaterial(const SharedHandle<Material>& material) const noexcept;
		void processSpawnedInstancesQueue() noexcept;
		void processRemovedInstancesQueue() noexcept;

		UniqueHandle<SceneRenderer> m_sceneRenderer;
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		uint32_t m_dirLightID = uint32_t(-1);

		InstanceDragger m_instanceDragger;
		UniqueHandle<Flashlight> m_flashlight;
		UniqueHandle<CameraManager> m_cameraManager;
		bool m_flashlightFollowing = true;
		bool m_ioBorrowed = false;
		bool m_timePaused = false;
		
		std::vector<uint32_t> m_spawnedInstances;
		std::vector<uint32_t> m_removedInstances;
	};

}; // engi namespace