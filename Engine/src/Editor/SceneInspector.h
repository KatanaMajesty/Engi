#pragma once

#include "Math/Math.h"
#include "Renderer/Renderer.h"
#include "Renderer/ModelLoader.h"
#include "World/CameraManager.h"
#include "World/Scene.h"
#include "World/Flashlight.h"
#include "Editor/EditorEvent.h"

namespace engi
{

	class SceneInspector
	{
	public:
		SceneInspector(Renderer* renderer);

		void OnUpdate() noexcept;
		void OnInstanceSelected(const editor::EvInstanceSelected& ev) noexcept;

		inline constexpr void SetActiveScene(Scene* scene) noexcept { m_activeScene = scene; }
		inline constexpr Scene* GetActiveScene() noexcept { return m_activeScene; }
		
		inline constexpr void RemoveInstanceSelection() noexcept { m_selectedInstanceID = uint32_t(-1); }
		inline constexpr uint32_t GetSelectedInstanceID() const noexcept { return m_selectedInstanceID; }
		inline constexpr const InstanceIntersection& GetSelectedInstanceIntersection() const noexcept { return m_instanceIntersection; }
		inline constexpr const math::Ray& GetIntersectionRay() const noexcept { return m_intersectionRay; }

		void DrawSceneRendererUI() noexcept;
		void DrawCameraUI() noexcept;
		void DrawWorldUI() noexcept;
		void DrawInstanceUI() noexcept;

		void AddCamera() noexcept;
		void AddFlashLight(const math::Vec3& color, float intensity, float radius, Texture2D* cookie) noexcept;
		
		void SetSkybox(const std::string& filename) noexcept;
		void SetDissolutionTexture(const std::string& filename) noexcept;
		void SetSceneLight(const math::Vec3& ambient, const math::Vec3& direction, float radius = 1.0f) noexcept;
		
		ModelInstance* AddInstance(const std::string& name, SharedHandle<Model> model, ArrayView<const MaterialInstance> materials, const InstanceData& instanceData) noexcept;
		ModelInstance* AddCube(const std::string& name, const MaterialInstance& material, const math::Transformation& transformation, const math::Vec3& color = math::Vec3()) noexcept;
		ModelInstance* AddSphere(const std::string& name, const MaterialInstance& material, const InstanceData& data) noexcept;
		ModelInstance* AddPointLight(const std::string& name, const math::Vec3& translation, const math::Vec3& emissive, float intensity, float radius) noexcept;
		ModelInstance* AddSmokeEmitter(const std::string& name, Texture2D* mvea, Texture2D* dbf, Texture2D* rlu, const math::Transformation& transformation, const math::Vec3& color = math::Vec3()) noexcept;
		ModelInstance* SpawnInstance(const std::string& name, SharedHandle<Model> model, ArrayView<const MaterialInstance> materials, const math::Vec3& color = math::Vec3()) noexcept;

	private:
		void DrawControls(CameraGPUProperties& gpuProps) noexcept;
		void DrawControls(CameraPhysicalProperties& physProps) noexcept;
		void DrawControls(Flashlight* flashlight) noexcept;

		bool DrawControls(PointLight* pointLight) noexcept;
		bool DrawControls(SpotLight* spotLight) noexcept;
		bool DrawControls(SmokeEmitter* particleEmitter) noexcept;
		bool DrawControls(DirectionalLight* dirLight) noexcept;
		
		void DrawAllMeshInstances() noexcept;
		void DrawSelectedMeshInstance(MaterialInstance& instanceMaterial) noexcept;

		Renderer* m_renderer;
		Scene* m_activeScene = nullptr;

		bool m_instanceWasJustSelected = false;

		uint32_t m_selectedInstanceID = uint32_t(-1);
		uint32_t m_meshIndex = uint32_t(-1); // we need this to be able to select other meshes in editor. We still ned the copy of intersection!
		InstanceIntersection m_instanceIntersection;
		math::Ray m_intersectionRay;
	};

}; // engi namespace