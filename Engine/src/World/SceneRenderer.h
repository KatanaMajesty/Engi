#pragma once

#include <vector>

#include "Utility/Memory.h"
#include "World/ParticleSystem/ParticleSystem.h"
#include "World/LightSystem/LightManager.h"
#include "World/ModelInstanceRegistry.h"
#include "World/CameraManager.h"
#include "Renderer/InstanceTable.h"
#include "Renderer/MeshManager.h"
#include "Renderer/ReflectionCapture.h"
#include "Renderer/RenderPass.h"
#include "Renderer/PostProcessor.h"

// TODO: Wip decals
#include "DecalSystem/Decal.h"
#include "DecalSystem/DecalManager.h"

namespace engi
{

	class Renderer;
	class ConstantBuffer;

	class SceneRenderer
	{
	public:
		bool init(Renderer* renderer) noexcept;
		void update(float timestep) noexcept;
		void render(bool debugPass) noexcept;

		inline void setCameraManager(CameraManager* cameraManager) noexcept { m_cameraManager = cameraManager; }
		inline constexpr Renderer* getRenderer() noexcept { return m_renderer; }

		inline ParticleSystem* getParticleSystem() noexcept { return m_particleSystem.get(); }
		inline LightManager* getLightManager() noexcept { return m_lightManager.get(); }

		inline InstanceTable* getInstanceTable() noexcept { return m_instanceTable.get(); }
		inline MeshManager* getMeshManager() noexcept { return m_meshManager.get(); }

		inline DecalManager* getDecalManager() noexcept { return m_decalManager.get(); }

		inline ModelInstanceRegistry* getInstanceRegistry() noexcept { return m_instanceRegistry.get(); }
		void setDissolutionTexture(Texture2D* texture) noexcept;
		void setDissolutionTime(float time) noexcept { m_dissolutionTime = time; }
		void setIncinerationTime(float time) noexcept { m_incinerationTime = time; }
		inline constexpr Texture2D* getDissolutionTexture() noexcept { return m_dissolutionTexture; }
		inline constexpr bool hasDissolutionTexture() const noexcept { return m_dissolutionTexture != nullptr; }
		inline constexpr float getDissolutionTime() const noexcept { return m_dissolutionTime; }
		inline constexpr float getIncinerationTime() const noexcept { return m_incinerationTime; }

		inline bool& isIBLEnabled() noexcept { return m_reflectionCapture->shouldUseIBL(); }

		inline Skybox* getSkybox() noexcept { return m_skybox.get(); }

		inline void setActiveSampler(Sampler* sampler) noexcept { m_activeSampler = sampler; }
		inline constexpr Sampler* getActiveSampler() noexcept { return m_activeSampler; }

		inline constexpr bool& useFXAA() noexcept { return m_useFXAA; }
		inline constexpr bool isFXAAEnabled() const noexcept { return m_useFXAA; }
		inline constexpr FXAASpec& getFXAASpecification() noexcept { return m_fxaaSpecification; }

	private:
		bool initSamplers() noexcept;

		void setSamplers() noexcept;
		void setSceneConstant() noexcept;
		void setViewConstant(const struct ViewConstant& viewConstant) noexcept;

		void renderShadowsForDirectionalLight() noexcept;
		void renderShadowsForPointLights() noexcept;
		void renderShadowsForSpotLights() noexcept;

		void deferredPass() noexcept;
		void forwardPass() noexcept;
		void performResolve() noexcept;
		void performDebugStage() noexcept;

		Renderer* m_renderer = nullptr;
		CameraManager* m_cameraManager = nullptr;
		
		float m_frameTimestep = 0.0f;
		UniqueHandle<ParticleSystem> m_particleSystem;
		UniqueHandle<LightManager> m_lightManager;

		UniqueHandle<InstanceTable> m_instanceTable;
		UniqueHandle<MeshManager> m_meshManager;
		uint32_t m_cameraInstanceID = uint32_t(-1);

		UniqueHandle<DecalManager> m_decalManager;

		UniqueHandle<ModelInstanceRegistry> m_instanceRegistry;
		Texture2D* m_dissolutionTexture = nullptr;
		float m_dissolutionTime = 0.0f;
		float m_incinerationTime = 0.0f;
		
		SharedHandle<Material> m_depthmap2DMaterial = nullptr;
		SharedHandle<Material> m_depthmapCubeMaterial = nullptr;
		SharedHandle<Material> m_normalVisMaterial = nullptr;
		SharedHandle<Material> m_deferredPBRMaterial = nullptr;
		SharedHandle<Material> m_deferredEmissiveMaterial = nullptr;
		SharedHandle<Material> m_incinerationParticlesMaterial = nullptr;

		UniqueHandle<ConstantBuffer> m_sceneConstantBuffer;
		UniqueHandle<ConstantBuffer> m_viewConstantBuffer;
		UniqueHandle<ReflectionCapture> m_reflectionCapture;
		UniqueHandle<Skybox> m_skybox;

		Sampler* m_activeSampler = nullptr;

		bool m_useFXAA = true;
		FXAASpec m_fxaaSpecification = FXAASpec();
		Texture2D* m_fxaaAttachment = nullptr;
	};

}; // engi namespace