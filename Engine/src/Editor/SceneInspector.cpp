#include "Editor/SceneInspector.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include "Core/Logger.h"
#include "World/Camera.h"
#include "World/CameraController.h"
#include "Renderer/PostProcessor.h"
#include "Renderer/InstanceData.h"
#include "Renderer/TextureLoader.h"
#include "Renderer/MaterialInstance.h"
#include "Renderer/MaterialRegistry.h"
#include "Renderer/Skybox.h"

#include "World/ParticleSystem/ParticleSystem.h"

namespace engi
{

	SceneInspector::SceneInspector(Renderer* renderer)
		: m_renderer(renderer)
	{
	}

	void SceneInspector::OnUpdate() noexcept
	{
		ImGui::SeparatorText("Scene Inspector");
		if (!m_activeScene)
		{
			ImGui::Text("No active scene was set");
			return;
		}

		// This is needed in case we have a dangling ID that might be used unintentionally when a new instance is created for the ID
		if (!m_activeScene->getModelInstanceByID(m_selectedInstanceID))
			this->RemoveInstanceSelection();

		ImGuiTabBarFlags tabFlags = ImGuiTabBarFlags_Reorderable;
		if (ImGui::BeginTabBar("##scene_inspector", tabFlags))
		{
			ImGuiTabItemFlags itemFlags = 0;
			if (ImGui::BeginTabItem("Scene Renderer", nullptr, itemFlags))
			{
				this->DrawSceneRendererUI();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("World", nullptr, itemFlags))
			{
				this->DrawWorldUI();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Camera", nullptr, itemFlags))
			{
				this->DrawCameraUI();
				ImGui::EndTabItem();
			}

			uint32_t instanceFlags = 0;
			if (m_instanceWasJustSelected)
			{
				instanceFlags |= ImGuiTabItemFlags_SetSelected;
				m_instanceWasJustSelected = false;
			}

			if (ImGui::BeginTabItem("Instance", nullptr, itemFlags | instanceFlags))
			{
				this->DrawInstanceUI();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	void SceneInspector::OnInstanceSelected(const editor::EvInstanceSelected& ev) noexcept
	{
		m_selectedInstanceID = ev.modelInstanceID;
		m_instanceIntersection = ev.instanceIntersection;
		m_intersectionRay = ev.ray;
		m_meshIndex = ev.instanceIntersection.meshIndex;

		// Needed to set focus on a instance editing tab
		if (ev.instanceIntersection.isValid())
			m_instanceWasJustSelected = true;
	}

	void SceneInspector::DrawSceneRendererUI() noexcept
	{
		ImGui::SeparatorText("Scene Renderer");
		if (ImGui::Button("Select sampler"))
		{
			ImGui::OpenPopup("##Sampler_Selection");
		}

		SceneRenderer* sceneRenderer = m_activeScene->getSceneRenderer();
		Renderer* renderer = sceneRenderer->getRenderer();
		if (ImGui::BeginPopup("##Sampler_Selection"))
		{
			ImGui::SeparatorText("Choose a sampler type");

			if (ImGui::Selectable("Linear Sampler"))
			{
				sceneRenderer->setActiveSampler(renderer->getLinearSampler());
			}

			if (ImGui::Selectable("Trilinear Sampler"))
			{
				sceneRenderer->setActiveSampler(renderer->getTrilinearSampler());
			}

			if (ImGui::Selectable("Anisotropic Sampler"))
			{
				sceneRenderer->setActiveSampler(renderer->getAnisotropicSampler());
			}

			ImGui::EndPopup();
		}

		ImGui::Checkbox("Enable IBL", &sceneRenderer->isIBLEnabled());
		ImGui::Checkbox("Enable Shadowmapping", &sceneRenderer->getLightManager()->isShadowmappingEnabled());

		ImGui::SeparatorText("Decal System");
		ImGui::SliderFloat("Decal Normal Threshold", &sceneRenderer->getDecalManager()->normalThreshold(), 0.0f, 180.0f);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Decal Normal Threshold is the max allowed angle difference\n"
				"between decal box orientation and underlying normal.");
		}

		ImGui::SeparatorText("Post Processing");
		ImGui::Checkbox("Enable FXAA", &sceneRenderer->useFXAA());

		FXAASpec& fxaaSpec = sceneRenderer->getFXAASpecification();
		ImGui::SliderFloat("Quality Subpix", &fxaaSpec.qualitySubpix, 0.0f, 1.0f);
		ImGui::SliderFloat("Quality Edge Threshold", &fxaaSpec.qualityEdgeThreshold, 0.063f, 0.333f);
		ImGui::SliderFloat("Quality Edge Threshold (min)", &fxaaSpec.qualityEdgeThresholdMin, 0.0312f, 0.0833f);
		if (ImGui::SmallButton("Reset FXAA spec"))
			fxaaSpec = FXAASpec();
	}

	void SceneInspector::DrawCameraUI() noexcept
	{
		ImGui::SeparatorText("Scene Camera");
		if (!m_activeScene->hasCamera())
		{
			if (ImGui::SmallButton("Add Camera"))
				AddCamera();

			return;
		}

		CameraManager* cm = m_activeScene->getCameraManager();
		Camera& camera = cm->getCamera();
		math::Vec3 pos = camera.getPosition();
		math::Vec3 dir = camera.getDirection();
		ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.9f, 1.0f), "Camera pos: Vec3(%.3f, %.3f, %.3f)", pos.x, pos.y, pos.z);
		ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.9f, 1.0f), "Camera dir: Vec3(%.3f, %.3f, %.3f)", dir.x, dir.y, dir.z);

		if (ImGui::CollapsingHeader("GPU Camera"))
		{
			this->DrawControls(cm->getGPUCameraProperties());
		}

		if (ImGui::CollapsingHeader("Physical Camera"))
		{
			this->DrawControls(cm->getPhysicalCameraProperties());
		}

		if (m_activeScene->hasFlashLight() && ImGui::CollapsingHeader("Camera Flashlight"))
		{
			this->DrawControls(m_activeScene->getFlashlight());
		}
	}

	void SceneInspector::DrawWorldUI() noexcept
	{
		ImGui::SeparatorText("World settings");
		ImGui::Checkbox("Pause time", &m_activeScene->isTimePaused());

		ImGui::SeparatorText("Directional Light");
		DirectionalLight& dirlight = m_activeScene->getDirLight();
		this->DrawControls(&dirlight);

		ImGui::SeparatorText("Used skybox");
		TextureCube* skymap = m_activeScene->getSkybox();
		if (skymap)
		{
			static std::string filename = std::filesystem::path(skymap->getName()).filename().string();
			ImGui::InputText("Skybox", &filename);
			if (ImGui::SmallButton("Update skybox"))
				this->SetSkybox(filename);
		}

		ImGui::SeparatorText("Instance configuration");
		float dt = m_activeScene->getDissolutionTime();
		if (ImGui::SliderFloat("Dissolution time", &dt, 0.0f, 10.0f))
		{
			m_activeScene->setDissolutionTime(dt);
		}

		SceneRenderer* sceneRenderer = m_activeScene->getSceneRenderer();
		Texture2D* dissolutionTexture = sceneRenderer->getDissolutionTexture();
		if (dissolutionTexture)
		{
			static std::string filename = std::filesystem::path(dissolutionTexture->getName()).filename().string();

			ImGui::InputText("Dissolution texture", &filename);
			if (ImGui::SmallButton("Change texture"))
			{
				this->SetDissolutionTexture(filename);
			}
		}
	}

	void SceneInspector::DrawInstanceUI() noexcept
	{
		ModelInstance* selectedInstance = m_activeScene->getModelInstanceByID(m_selectedInstanceID);

		ImGui::SeparatorText("Selected Instance");
		if (!selectedInstance)
		{ 
			ImGui::Text("No instance was selected (use ALT + LMB)");
			return;
		}

		ENGI_ASSERT(m_meshIndex != uint32_t(-1));
		const char* instanceName = selectedInstance->getName().c_str();
		const char* modelpath = selectedInstance->getModel()->getPath().c_str();
		ImGui::Text("Instance name: %s", instanceName);
		ImGui::Text("Model path: %s", modelpath);

		InstanceData& data = selectedInstance->getData();
		math::Vec3 pos = data.modelToWorld.getTranslation();
		ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.9f, 1.0f), "Position: Vec3(%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);
		ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR;
		if (ImGui::ColorEdit3("Instance Color", &data.color.x, colorFlags))
		{
			selectedInstance->updateMeshData();
		}

		if (selectedInstance->hasPointLight() && ImGui::CollapsingHeader("Point Light"))
		{
			ImGui::PushID(0);
			if (this->DrawControls(selectedInstance->getPointLight()))
			{
				selectedInstance->updatePointLight();
			}
			ImGui::PopID();
		}
		
		if (selectedInstance->hasSpotLight() && ImGui::CollapsingHeader("Spot Light"))
		{
			ImGui::PushID(1);
			if (this->DrawControls(selectedInstance->getSpotLight()))
			{
				selectedInstance->updateSpotLight();
			}
			ImGui::PopID();
		}

		if (selectedInstance->hasParticleEmitter() && ImGui::CollapsingHeader("Particle Emitter"))
		{
			ImGui::PushID(2);
			if (this->DrawControls(selectedInstance->getParticleEmitter()))
			{
				selectedInstance->updateParticleEmitter();
			}
			ImGui::PopID();
		}

		if (selectedInstance->getNumDecals() > 0 && ImGui::Button("Clear Decals"))
		{
			selectedInstance->removeAllDecals();
		}

		this->DrawAllMeshInstances();
		
		ImGui::SeparatorText("Selected mesh instance");
		MaterialInstance materialInstance = selectedInstance->getMaterialInstance(m_meshIndex);
		this->DrawSelectedMeshInstance(materialInstance);
	}

	void SceneInspector::AddCamera() noexcept
	{
		if (!m_activeScene)
			return;

		m_activeScene->createCamera();
	}

	void SceneInspector::SetSkybox(const std::string& filename) noexcept
	{
		// TODO: Temp solution for accessing a texture. We should use resource panel for accessing texture assets
		static const std::filesystem::path skyboxFolder = (FileSystem::getInstance().getAssetsPath() / "Textures" / "Skybox");
		std::string filepath = (skyboxFolder / filename).string();

		TextureLoader* tl = m_renderer->getTextureLoader();
		TextureCube* texture = tl->loadTextureCube(filepath, true);
		if (texture)
		{
			m_activeScene->setSkybox(texture);
			ENGI_LOG_INFO("Successfully set a skybox to {}", filename);
		}
		else
		{
			ENGI_LOG_ERROR("Failed to set a skybox to {}\nCheck the Assets/Textures/Skybox folder", filename);
		}
	}

	void SceneInspector::SetDissolutionTexture(const std::string& filename) noexcept
	{
		// TODO: Temp solution for accessing a texture. We should use resource panel for accessing texture assets
		static const std::filesystem::path textureFolder = (FileSystem::getInstance().getAssetsPath() / "Textures");
		std::string filepath = (textureFolder / filename).string();

		TextureLoader* tl = m_renderer->getTextureLoader();
		Texture2D* texture = tl->loadTexture2D(filepath, true);
		if (texture)
		{
			m_activeScene->setDissolutionTexture(texture);
			ENGI_LOG_INFO("Successfully changed dissolution texture to {}", filename);
		}
		else
		{
			ENGI_LOG_ERROR("Failed to change a dissolution texture to {}\nCheck the Assets/Textures folder", filename);
		}
	}

	void SceneInspector::SetSceneLight(const math::Vec3& ambient, const math::Vec3& direction, float radius) noexcept
	{
		m_activeScene->setDirLight(ambient, direction, radius);
	}

	void SceneInspector::AddFlashLight(const math::Vec3& color, float intensity, float radius, Texture2D* cookie) noexcept
	{
		if (!m_activeScene)
			return;

		m_activeScene->createFlashlight(color, intensity, radius, cookie);
	}

	ModelInstance* SceneInspector::AddInstance(const std::string& name, SharedHandle<Model> model, ArrayView<const MaterialInstance> materials, const InstanceData& instanceData) noexcept
	{
		if (!m_activeScene)
			return nullptr;

		uint32_t modelInstanceID = m_activeScene->addInstance(name, model, materials, instanceData);
		return m_activeScene->getModelInstanceByID(modelInstanceID);
	}

	ModelInstance* SceneInspector::AddCube(const std::string& name, const MaterialInstance& material, const math::Transformation& transformation, const math::Vec3& color) noexcept
	{
		if (!m_activeScene)
			return nullptr;

		ModelRegistry* modelRegistry = m_activeScene->getSceneRenderer()->getRenderer()->getModelRegistry();
		SharedHandle<Model> model = modelRegistry->getModel(MODEL_TYPE_CUBE);
		ENGI_ASSERT(model);

		InstanceData instanceData(transformation);
		instanceData.color = color;

		ModelInstance* result = this->AddInstance(name, model, viewOf(material), instanceData);
		return result;
	}

	ModelInstance* SceneInspector::AddSphere(const std::string& name, const MaterialInstance& material, const InstanceData& data) noexcept
	{
		if (!m_activeScene)
			return nullptr;

		ModelRegistry* modelRegistry = m_activeScene->getSceneRenderer()->getRenderer()->getModelRegistry();
		SharedHandle<Model> model = modelRegistry->getModel(MODEL_TYPE_SPHERE);
		ENGI_ASSERT(model);

		ModelInstance* result = this->AddInstance(name, model, viewOf(material), data);
		return result;
	}

	ModelInstance* SceneInspector::AddPointLight(const std::string& name, const math::Vec3& translation, const math::Vec3& emissive, float intensity, float radius) noexcept
	{
		if (!m_activeScene)
			return nullptr;

		MaterialRegistry* materialRegistry = m_activeScene->getSceneRenderer()->getRenderer()->getMaterialRegistry();
		SharedHandle<Material> material = materialRegistry->getMaterial(MATERIAL_EMISSIVE);
		ENGI_ASSERT(material);

		MaterialInstance sphereMaterial(name + "_MaterialInstance", material);

		math::Transformation transformation;
		transformation.rotation = math::Vec3();
		transformation.scale = math::Vec3(radius);
		transformation.translation = translation;

		InstanceData instanceData(transformation);
		instanceData.emission = emissive;
		instanceData.emissionPower = intensity;

		ModelInstance* sphere = this->AddSphere(name, MaterialInstance(name + "_Material", material), instanceData);
		if (!sphere)
			return nullptr;

		sphere->addPointLight(emissive, intensity, radius);
		return sphere;
	}

	ModelInstance* SceneInspector::AddSmokeEmitter(const std::string& name, Texture2D* mvea, Texture2D* dbf, Texture2D* rlu, const math::Transformation& transformation, const math::Vec3& color) noexcept
	{
		if (!m_activeScene)
			return nullptr;

		MaterialRegistry* materialRegistry = m_activeScene->getSceneRenderer()->getRenderer()->getMaterialRegistry();
		SharedHandle<Material> material = materialRegistry->getMaterial(MATERIAL_EMISSIVE);
		ENGI_ASSERT(material);

		MaterialInstance sphereMaterial(name + "_MaterialInstance", material);

		// We use color as an emission color and some constant emission power
		InstanceData instanceData(transformation);
		instanceData.emission = color;
		instanceData.emissionPower = 4.0f;

		ModelInstance* sphere = this->AddSphere(name, MaterialInstance(name + "_Material", material), instanceData);
		if (!sphere)
			return nullptr;

		sphere->addParticleEmitter(mvea, dbf, rlu);
		return nullptr;
	}

	ModelInstance* SceneInspector::SpawnInstance(const std::string& name, SharedHandle<Model> model, ArrayView<const MaterialInstance> materials, const math::Vec3& color) noexcept
	{
		if (!m_activeScene)
			return nullptr;

		uint32_t modelInstanceID = m_activeScene->spawnInstance(name, model, materials, color);
		return m_activeScene->getModelInstanceByID(modelInstanceID);
	}

	void SceneInspector::DrawControls(CameraGPUProperties& gpuProps) noexcept
	{
		CameraManager* cm = m_activeScene->getCameraManager();
		bool gpuUpdate = false;
		if (ImGui::SliderFloat("FOV", &gpuProps.fov, 30.0f, 100.0f)) gpuUpdate = true;
		if (ImGui::SliderFloat("Near plane", &gpuProps.nearPlane, 0.1f, 10.0f)) gpuUpdate = true;
		if (ImGui::SliderFloat("Far plane", &gpuProps.farPlane, 10.0f, 1000.0f)) gpuUpdate = true;
		if (ImGui::SliderFloat("Shdaow plane", &gpuProps.shadowPlane, 10.0f, 1000.0f)) gpuUpdate = true;
		if (ImGui::SliderFloat("Rotation speed", &gpuProps.rotationSpeed, 90.0f, 360.0f)) gpuUpdate = true;
		if (ImGui::SliderFloat("Translation speed", &gpuProps.translationSpeed, 1.0f, 100.0f)) gpuUpdate = true;
		if (gpuUpdate)
			cm->updateProjection();
	}

	void SceneInspector::DrawControls(CameraPhysicalProperties& physProps) noexcept
	{
		ImGui::SliderFloat("Exposure value", &physProps.exposureValue, -7.0f, 7.0f);
		ImGui::SliderFloat("Gamma correction", &physProps.gammaCorrection, 1.0f, 3.0f);
	}

	void SceneInspector::DrawControls(Flashlight* flashlight) noexcept
	{
		SpotLight& light = flashlight->getSpotlight();
		this->DrawControls(&light);
	}

	bool SceneInspector::DrawControls(PointLight* pointLight) noexcept
	{
		if (!pointLight)
			return false;

		bool res = false;
		if (ImGui::DragFloat3("Local Position", &pointLight->getRelativePosition().x)) res = true;
		if (ImGui::ColorEdit3("Color", &pointLight->getColor().x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR)) res = true;
		if (ImGui::DragFloat("Intensity", &pointLight->getIntensity(), 0.1f, 0.0f, 8.0f)) res = true;
		if (ImGui::SliderFloat("Radius", &pointLight->getRadius(), 0.05f, 4.0f)) res = true;
		return res;
	}

	bool SceneInspector::DrawControls(SpotLight* spotLight) noexcept
	{
		if (!spotLight)
			return false;

		bool res = false;
		if (ImGui::DragFloat3("Position", &spotLight->getRelativePosition().x)) res = true;
		if (ImGui::DragFloat3("Direction", &spotLight->getDirection().x, 0.1f)) res = true;
		if (ImGui::ColorEdit3("Color", &spotLight->getColor().x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR)) res = true;
		if (ImGui::DragFloat("Intensity", &spotLight->getIntensity(), 0.1f, 0.0f, 8.0f)) res = true;
		if (ImGui::SliderFloat("Radius", &spotLight->getRadius(), 0.05f, 4.0f)) res = true;
		if (ImGui::SliderFloat("Cutoff", &spotLight->getCutoff(), 5.0f, 60.0f)) res = true;
		if (ImGui::SliderFloat("Smoothing", &spotLight->getSmoothing(), 0.0f, 30.0f)) res = true;
		return res;
	}

	bool SceneInspector::DrawControls(SmokeEmitter* particleEmitter) noexcept
	{
		if (!particleEmitter)
			return false;

		EmitterSettings& settings = particleEmitter->getSettings();

		bool res = false;
		if (ImGui::DragInt("Number of particles", (int*)&settings.numParticles, 1.0f, 0, 200)) res = true;
		if (ImGui::DragFloat3("Particle color", &settings.color.x, 0.1f, 0.0f, 1.0f)) res = true;
		if (ImGui::DragFloat3("Minimum particle speed", &settings.minimumSpeed.x, 0.1f, -10.0f, 10.0f)) res = true;
		if (ImGui::DragFloat3("Maximum particle speed", &settings.maximumSpeed.x, 0.1f, -10.0f, 10.0f)) res = true;
		if (ImGui::SliderFloat("Minimum particle size", &settings.minimumParticleSize, 0.0f, 5.0f)) res = true;
		if (ImGui::SliderFloat("Maximum particle size", &settings.maximumParticleSize, 0.0f, 5.0f)) res = true;
		if (ImGui::SliderFloat("Minimum particle growth", &settings.minimumParticleGrowth, 0.0f, 5.0f)) res = true;
		if (ImGui::SliderFloat("Maximum particle growth", &settings.maximumParticleGrowth, 0.0f, 5.0f)) res = true;
		if (ImGui::SliderFloat("Minimum Lifetime", &settings.minimumLifetime, 1.0f, 20.0f)) res = true;
		if (ImGui::SliderFloat("Maximum Lifetime", &settings.maximumLifetime, 1.0f, 20.0f)) res = true;
		if (ImGui::SliderFloat("Spawn rate", &settings.spawnRate, 0.1f, 10.0f)) res = true;
		return res;
	}

	bool SceneInspector::DrawControls(DirectionalLight* dirLight) noexcept
	{
		if (!dirLight)
			return false;

		bool res = false;
		if (ImGui::ColorEdit3("Ambient", &dirLight->getColor().x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR)) res = true;
		if (ImGui::SliderFloat3("Direction", &dirLight->getDirection().x, -2.0f, 2.0f)) res = true;
		if (ImGui::SliderFloat("Radius", &dirLight->getRadius(), 0.05f, 1.0f)) res = true;
		return res;
	}

	void SceneInspector::DrawAllMeshInstances() noexcept
	{
		ModelInstance* selectedInstance = m_activeScene->getModelInstanceByID(m_selectedInstanceID);

		if (!selectedInstance)
			return;

		const auto& meshInstances = selectedInstance->getMeshInstances();
		const auto& selectedMesh = meshInstances.at(m_meshIndex);
		if (ImGui::CollapsingHeader("All Mesh Instances"))
		{
			uint32_t numMeshes = selectedInstance->getNumMeshInstances();
			ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable;
			if (ImGui::BeginTable("##mesh_table", 2, tableFlags))
			{
				ImGui::TableSetupColumn("Mesh instance index");
				ImGui::TableSetupColumn("Mesh instance name");
				ImGui::TableHeadersRow();
				for (uint32_t i = 0; i < numMeshes; ++i)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("%d", i);

					ImGui::TableNextColumn();
					const auto& currentMesh = meshInstances[i];
					bool selected = (m_meshIndex == i);
					if (ImGui::Selectable(currentMesh.mesh->getName().c_str(), selected))
						m_meshIndex = i;

					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndTable();
			}
		}
	}

	void SceneInspector::DrawSelectedMeshInstance(MaterialInstance& instanceMaterial) noexcept
	{
		ModelInstance* selectedInstance = m_activeScene->getModelInstanceByID(m_selectedInstanceID);
	
		if (!selectedInstance)
			return;

		const auto& meshInstances = selectedInstance->getMeshInstances();
		const auto& selectedMesh = meshInstances.at(m_meshIndex);
		ENGI_ASSERT(selectedMesh.hasMaterial());
		const char* meshName = selectedMesh.mesh->getName().c_str();
		const char* materialInstanceName = selectedMesh.materialInstance.getName().c_str();
		const char* materialName = selectedMesh.materialInstance.getMaterial()->getName().c_str();
		ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.9f, 1.0f), "Selected mesh name: %s", meshName);
		ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.9f, 1.0f), "Selected mesh material instance name: %s", materialInstanceName);

		bool shouldSet = false;
		MaterialInstance materialInstance = selectedInstance->getMaterialInstance(m_meshIndex);
		MaterialConstant& materialData = materialInstance.getData();
		//if (ImGui::SliderFloat("Glossines", &materialData.glossiness, 2.0f, 64.0f)) shouldSet = true;
		if (ImGui::SliderFloat("Metalness", &materialData.metallic, 0.0f, 1.0f)) shouldSet = true;
		if (ImGui::SliderFloat("Roughness", &materialData.roughness, 0.0f, 1.0f)) shouldSet = true;
		if (ImGui::Checkbox("Use albedo texture", &materialData.useAlbedoTexture)) shouldSet = true;
		if (ImGui::Checkbox("Use normal map", &materialData.useNormalMap)) shouldSet = true;
		if (ImGui::Checkbox("Use metalness map", &materialData.useMetalnessMap)) shouldSet = true;
		if (ImGui::Checkbox("Use roughness map", &materialData.useRoughnessMap)) shouldSet = true;

		if (shouldSet)
			selectedInstance->setMaterialInstance(m_meshIndex, materialInstance);

		ImGui::Text("Change material: "); ImGui::SameLine();
		if (ImGui::BeginCombo("##avail_materials", materialName))
		{
			for (auto& [name, material] : m_renderer->getMaterialRegistry()->getAllMaterials())
			{
				bool selected = (selectedMesh.materialInstance.getMaterial() == material);
				if (ImGui::Selectable(name.c_str(), selected))
				{
					ENGI_ASSERT(!instanceMaterial.isEmpty());
					instanceMaterial.setMaterial(material);
					selectedInstance->setMaterialInstance(m_meshIndex, instanceMaterial);
				}

				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

}; // engi namespace