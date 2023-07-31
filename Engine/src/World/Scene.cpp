#include "World/Scene.h"

#include "Utility/Random.h"
#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "Core/Input.h"
#include "Core/EventBus.h"
#include "World/Camera.h"
#include "World/CameraController.h"
#include "Renderer/Renderer.h"
#include "Renderer/MaterialRegistry.h"
#include "Renderer/ModelRegistry.h"
#include "Renderer/ModelLoader.h"
#include "Renderer/MaterialInstance.h"
#include "Renderer/TextureLibrary.h"
#include "Renderer/Skybox.h"

#include "SceneRenderer.h"

// TODO: Remove this header
#include <iostream>

namespace engi
{

	Scene::Scene()
	{
		EventBus::get().subscribe(this, &Scene::onKeyPressed);
		EventBus::get().subscribe(this, &Scene::onResize);
	}

	Scene::~Scene()
	{
		EventBus::get().unsubscribeAll(this);
	}

	void Scene::init(Renderer* renderer, uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;

		m_cameraManager = makeUnique<CameraManager>(new CameraManager(m_width, m_height));

		m_sceneRenderer = makeUnique<SceneRenderer>(new SceneRenderer());
		m_sceneRenderer->init(renderer);
		// TODO: This must be done in Scene::createCamera, but doing it here is okay as well. Be aware though
		m_sceneRenderer->setCameraManager(this->getCameraManager());

		m_flashlight = makeUnique<Flashlight>(new Flashlight(m_sceneRenderer.get()));
		m_instanceDragger.init(m_sceneRenderer->getInstanceRegistry());
	}

	void Scene::onUpdate(float timestep, bool debugPass)
	{
		if (!hasCamera())
			return;

		Camera& camera = m_cameraManager->getCamera();
		if (!this->isIOBorrowed())
		{
			m_cameraManager->update(timestep);
			m_instanceDragger.update(camera.castRay(EngiIO::get().getCursorPos()));
		}
		else this->isIOBorrowed() = false;

		if (this->hasFlashLight() && m_flashlightFollowing)
			m_flashlight->update();

		if (this->isTimePaused())
		{
			// If time is paused
			timestep = 0.0f;
		}
		else
		{
			// If time is running
			this->processSpawnedInstancesQueue();
			this->processRemovedInstancesQueue();
		}

		m_sceneRenderer->update(timestep);
		m_sceneRenderer->render(debugPass);

		Renderer* renderer = m_sceneRenderer->getRenderer();
		renderer->beginDebugBatch();
		for (uint32_t removedInstanceID : m_removedInstances)
		{
			ModelInstance* instance = this->getModelInstanceByID(removedInstanceID);
			if (!instance)
				continue;

			renderer->drawAABB(instance->getAABB());
		}
		renderer->endDebugBatch();
	}

	void Scene::onResize(const EvWindowResized& ev) noexcept
	{
		m_width = ev.width;
		m_height = ev.height;
		if (m_cameraManager)
			m_cameraManager->resize(ev.width, ev.height);
	}

	void Scene::onKeyPressed(const EvKeyPressed& ev) noexcept
	{
		if (ev.keycode == Keycode::F)
		{
			ENGI_LOG_TRACE("Scene: Flash light following is toggled: {}", !m_flashlightFollowing);
			m_flashlightFollowing = !m_flashlightFollowing;
		}

		if (ev.keycode == Keycode::P)
		{
			ENGI_LOG_TRACE("Scene: Paused the time: {}", !this->isTimePaused());
			this->isTimePaused() = !this->isTimePaused();
		}
	}

	void Scene::createCamera(const math::Vec3& pos, const math::Vec3& dir) noexcept
	{
		ENGI_ASSERT(this->hasCamera() && "Scene was not initialized correctly");

		CameraManager* cm = this->getCameraManager();
		if (!cm->init(pos, dir))
		{
			ENGI_LOG_WARN("Failed to initialize the camera");
			return;
		}
		m_sceneRenderer->setCameraManager(cm);
	}

	void Scene::createFlashlight(const math::Vec3& color, float intensity, float radius, Texture2D* lightMask) noexcept
	{
		ENGI_ASSERT(m_flashlight && "The scene was not initialized at this point");
		m_flashlight->init(&m_cameraManager->getCamera(), color, intensity, radius, lightMask);
	}

	void Scene::setSkybox(TextureCube* texture) noexcept
	{
		Skybox* skybox = m_sceneRenderer->getSkybox();
		ENGI_ASSERT(skybox);
		skybox->setTexture(texture);
	}

	void Scene::setDirLight(const math::Vec3& color, const math::Vec3& direction, float radius) noexcept
	{
		LightManager* lm = m_sceneRenderer->getLightManager();
		if (m_dirLightID == uint32_t(-1))
		{
			m_dirLightID = lm->createDirLight(color, direction, radius);
			return;
		}

		auto& light = lm->getDirLight(m_dirLightID);
		light.getColor() = color;
		light.getDirection() = direction;
		light.getRadius() = radius;
	}

	TextureCube* Scene::getSkybox() noexcept
	{
		Skybox* skybox = m_sceneRenderer->getSkybox();
		if (!skybox)
			return nullptr;

		return skybox->getTexture();
	}

	DirectionalLight& Scene::getDirLight() noexcept
	{
		LightManager* lm = m_sceneRenderer->getLightManager();
		if (m_dirLightID == uint32_t(-1)) // TODO: Maybe temp?
			m_dirLightID = lm->createDirLight(math::Vec3(0.2f, 0.7f, 0.9f), math::Vec3(0.0f, 0.0f, 1.0f), 0.4f);

		return lm->getDirLight(m_dirLightID);
	}

	uint32_t Scene::addInstance(const std::string& name, const SharedHandle<Model>& model, ArrayView<const MaterialInstance> materials, const InstanceData& instanceData) noexcept
	{
		uint32_t modelInstanceID = uint32_t(-1);
		if (!model)
		{
			ENGI_LOG_INFO("Scene::addInstance() -> provided model is nullptr. Doing nothing");
			return modelInstanceID;
		}

		if (materials.empty())
		{
			ENGI_LOG_TRACE("Scene::addInstance() -> provided ArrayView of materials was empty. Errors might occur, but the instance will still be added");
		}

		ModelInstanceRegistry* instanceRegistry = m_sceneRenderer->getInstanceRegistry();
		modelInstanceID = instanceRegistry->addInstance(name, model, materials, instanceData);
		if (!instanceRegistry->isValidID(modelInstanceID))
		{
			ENGI_LOG_WARN("Scene::addInstance() -> failed to add an instance to the instance registry. Returning nullptr");
			return modelInstanceID;
		}

		return modelInstanceID;
	}

	uint32_t Scene::spawnInstance(const std::string& name, const SharedHandle<Model>& model, ArrayView<const MaterialInstance> materials, const math::Vec3& color) noexcept
	{
		Camera& camera = getCameraManager()->getCamera();
		math::Transformation transform = this->getTransformForInstanceSpawn(camera);

		std::vector<MaterialInstance> dissolutionMaterials;
		this->copyMaterialInstanceForDissolution(dissolutionMaterials, materials);

		InstanceData instanceData(transform);
		instanceData.color = color;
		instanceData.time = 0.0f;
		instanceData.emission = Random::GenerateFloat3(math::Vec3(0.125f), math::Vec3(1.0f));
		instanceData.emissionPower = 8.0f;

		uint32_t modelInstanceID = this->addInstance(name, model, dissolutionMaterials, instanceData);
		if (!this->getModelInstanceByID(modelInstanceID))
		{
			ENGI_LOG_WARN("Failed to create an instance {}", name);
			return modelInstanceID;
		}

		for (auto& meshEntry : model->getStaticMeshEntries())
		{
			if (!meshEntry.mesh.hasTexCoords())
			{
				ENGI_LOG_WARN("Some meshes of model {} (mesh {}) have no texture coordinates. Dissolution effect will not be applied to that mesh",
					model->getName(), meshEntry.mesh.getName());
			}
		}
		m_spawnedInstances.push_back(modelInstanceID);
		ENGI_LOG_INFO("Added an instance {} of model {} to the spawned instances queue", name, model->getName());

		return modelInstanceID;
	}

	void Scene::removeInstance(uint32_t modelInstanceID, const math::Vec3& hitpos) noexcept
	{
		ModelInstance* instance = this->getModelInstanceByID(modelInstanceID);
		if (!instance)
			return;

		auto it = std::ranges::find(m_removedInstances, modelInstanceID);
		if (it != m_removedInstances.end())
		{
			ENGI_LOG_WARN("Trying to remove the instance that is already being removed. Skipping...");
			return;
		}

		MaterialRegistry* materialRegistry = m_sceneRenderer->getRenderer()->getMaterialRegistry();
		for (uint32_t meshIndex = 0; meshIndex < instance->getNumMeshInstances(); ++meshIndex)
		{
			MaterialInstance materialInstance = instance->getMaterialInstance(meshIndex);
			SharedHandle<Material> material = materialInstance.getMaterial();
			if (!isIncineratableMaterial(material))
			{
				ENGI_LOG_WARN("Cannot incinerate the instance {} as some of the Mesh Instances are using non-incineratable material ({}). Removing it instantly", 
					instance->getName(), 
					material->getName());
				m_sceneRenderer->getInstanceRegistry()->removeInstance(modelInstanceID);
				return;
			}

			materialInstance.setMaterial(materialRegistry->getMaterial(MATERIAL_BRDF_PBR_INCINERATION));
			instance->setMaterialInstance(meshIndex, materialInstance);
		}

		math::AABB aabb = instance->getAABB();
		float radius = aabb.size().length();

		InstanceData& data = instance->getData();
		data.sphereOrigin = hitpos * data.worldToModel;
		data.sphereRadiusMax = radius;
		data.time = 0.0f;
		data.emission = Random::GenerateFloat3(math::Vec3(0.125f), math::Vec3(1.0f));
		data.emissionPower = 8.0f;

		m_removedInstances.push_back(modelInstanceID);
	}

	ModelInstance* Scene::getModelInstanceByID(uint32_t modelInstanceID) noexcept
	{
		ModelInstanceRegistry* instanceRegistry = m_sceneRenderer->getInstanceRegistry();
		return instanceRegistry->getModelInstance(modelInstanceID);
	}

	math::Transformation Scene::getTransformForInstanceSpawn(const Camera& camera) const noexcept
	{
		math::Vec3 eyeDir = camera.getDirection();
		eyeDir.normalize();

		// if cross product's Y component is negative, then camera is to the left from the z-axis
		math::Vec3 z = math::Vec3(0.0f, 0.0f, 1.0f);
		math::Vec3 p = eyeDir.cross(z);
		float cosY = eyeDir.dot(z);
		float rotY = std::acos(cosY);
		if (p.y > 0.0f)
			rotY = math::Numeric::pi2() - rotY;

		math::Transformation transform;
		transform.translation = camera.getPosition() + eyeDir * 3.0f;
		transform.rotation = math::Vec3(0.0f, rotY, 0.0f);
		transform.scale = math::Vec3(1.0f);
		return transform;
	}

	void Scene::copyMaterialInstanceForDissolution(std::vector<MaterialInstance>& dissolutionMaterials, ArrayView<const MaterialInstance> materials) const noexcept
	{
		if (materials.empty())
			return;

		dissolutionMaterials.reserve(materials.size());
		MaterialRegistry* materialRegistry = m_sceneRenderer->getRenderer()->getMaterialRegistry();
		SharedHandle<Material> dissolutionMaterial = materialRegistry->getMaterial(MATERIAL_BRDF_PBR_DISSOLUTION);

		std::ranges::copy(materials.begin(), materials.end(), std::back_inserter(dissolutionMaterials));
		for (MaterialInstance& mi : dissolutionMaterials)
			mi.setMaterial(dissolutionMaterial);
	}

	//ModelInstance* Scene::createCube(const std::string& name, const MaterialInstance& material, const math::Transformation& transformation, const math::Vec3& color) noexcept
	//{
	//	if (material.isEmpty())
	//		return nullptr;

	//	SharedHandle<Model> model = m_renderer->getModelRegistry()->getModel(ModelType::MODEL_TYPE_CUBE);
	//	ENGI_ASSERT(model && "Failed to init models");

	//	InstanceData data;
	//	data.modelToWorld = math::Mat4x4::toWorld(transformation.translation, transformation.rotation, transformation.scale);
	//	data.worldToModel = data.modelToWorld.inverse();
	//	data.color = color;

	//	ModelInstance* instance = m_modelInstanceRegistry->addInstance(name, model, viewOf(material), data);
	//	if (!instance)
	//	{
	//		std::cout << "Failed to create cube instance\n";
	//		return nullptr;
	//	}

	//	return instance;
	//}

	//ModelInstance* Scene::createSphere(const std::string& name, const MaterialInstance& material, const math::Transformation& transformation, const math::Vec3& color) noexcept
	//{
	//	if (material.isEmpty())
	//		return nullptr;

	//	SharedHandle<Model> model = m_sceneRenderer->getRenderer()->getModelRegistry()->getModel(ModelType::MODEL_TYPE_SPHERE);
	//	ENGI_ASSERT(model && "Failed to init models");

	//	InstanceData data;
	//	data.modelToWorld = math::Mat4x4::toWorld(transformation.translation, transformation.rotation, transformation.scale);
	//	data.worldToModel = data.modelToWorld.inverse();
	//	data.color = color;

	//	ModelInstance* instance = m_modelInstanceRegistry->addInstance(name, model, viewOf(material), data);
	//	if (!instance)
	//	{
	//		std::cout << "Failed to create sphere instance\n";
	//		return nullptr;
	//	}

	//	return instance;
	//}

	//ModelInstance* Scene::createLightSphere(const std::string& name, const math::Vec3& translation, const math::Vec3& color, float intensity, float radius) noexcept
	//{
	//	SharedHandle<Material> material = m_sceneRenderer->getRenderer()->getMaterialRegistry()->getMaterial(MaterialType::MATERIAL_EMISSIVE);
	//	ENGI_ASSERT(material && "Failed to init materials");

	//	std::string materialInstanceSign = name + "::MaterialInstance";
	//	MaterialInstance materialInstance(materialInstanceSign, material);
	//	materialInstance.setUseAlbedoTexture(false);
	//	materialInstance.setUseMetalnessMap(false);
	//	materialInstance.setUseNormalMap(false);
	//	materialInstance.setUseRoughnessMap(false);

	//	math::Transformation transformation(translation, math::Vec3(0.0f), math::Vec3(radius));
	//	ModelInstance* instance = createSphere(name, materialInstance, transformation, color);
	//	if (!instance)
	//		return nullptr;

	//	// TODO: Temp
	//	instance->setPointLight(color, intensity, radius);
	//	return instance;
	//}

	//SmokeEmitter& Scene::createSmokeEmitter(const std::string& name, const math::Vec3& translation, const math::Vec3& color) noexcept
	//{
	//	SharedHandle<Material> material = m_renderer->getMaterialRegistry()->getMaterial(MaterialType::MATERIAL_EMISSIVE);
	//	ENGI_ASSERT(material && "Failed to init materials");

	//	std::string materialInstanceSign = name + "::MaterialInstance";
	//	MaterialInstance materialInstance(materialInstanceSign, material);
	//	materialInstance.setUseAlbedoTexture(false);
	//	materialInstance.setUseMetalnessMap(false);
	//	materialInstance.setUseNormalMap(false);
	//	materialInstance.setUseRoughnessMap(false);

	//	math::Transformation transformation(translation, math::Vec3(0.0f), math::Vec3(0.1f));
	//	ModelInstance* instance = createSphere(name, materialInstance, transformation, color);
	//	if (!instance)
	//	{
	//		ENGI_ASSERT(false);
	//	}

	//	return m_particleSystem->addSmokeEmitter(instance->getInstanceTable(), instance->getInstanceID());
	//}

	//void Scene::setDissolutionTexture(Texture2D* texture) noexcept
	//{
	//	ENGI_ASSERT(texture->hasShaderView() && "Texture should have a resource view at this point");
	//	// TODO: Add checks if the texture is of BC4 compression format

	//	// I love C++17
	//	(texture ? ENGI_LOG_TRACE("Changed dissolution texture to {}", texture->getName()) : ENGI_LOG_TRACE("Dissolution texture is now unbound"));
	//	m_dissolutionTexture = texture;
	//}

	void Scene::processSpawnedInstancesQueue() noexcept
	{
		// Remove from dissolution group those instances, that have expired
		MaterialRegistry* materialRegistry = m_sceneRenderer->getRenderer()->getMaterialRegistry();
		std::erase_if(m_spawnedInstances, [=](uint32_t& modelInstanceID) -> bool
			{
				ModelInstance* instance = this->getModelInstanceByID(modelInstanceID);
				if (!instance)
				{
					// if instance is invalid, it should be removed from queue
					ENGI_LOG_INFO("Removing an instance (ModelInstanceID {}) from spawned instances queue as it is no longer a valid instance", modelInstanceID);
					return true;
				}
		
				if (instance->getData().time < getDissolutionTime())
					return false;

				// Update the material instance
				ENGI_LOG_INFO("Removing an instance {} from spawned instances queue", instance->getName());
				uint32_t numMeshes = instance->getNumMeshInstances();
				for (uint32_t meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
				{
					const auto& meshInstance = instance->getMeshInstances().at(meshIndex);
					MaterialInstance mi = instance->getMaterialInstance(meshIndex);
					bool twosided = meshInstance.mesh->isTwoSided();
					mi.setMaterial(materialRegistry->getMaterial(twosided ? MaterialType::MATERIAL_BRDF_PBR_NO_CULLING : MaterialType::MATERIAL_BRDF_PBR));
					instance->setMaterialInstance(meshIndex, mi);
				}

				return true;
			});
	}

	void Scene::processRemovedInstancesQueue() noexcept
	{
		std::erase_if(m_removedInstances, [this](uint32_t& modelInstanceID) -> bool
			{
				ModelInstance* instance = this->getModelInstanceByID(modelInstanceID);
				if (!instance)
				{
					// if instance is invalid, it should be removed from queue
					ENGI_LOG_INFO("Removing an instance (ModelInstanceID {}) from removed instances queue as it is no longer a valid instance", modelInstanceID);
					return true;
				}

				if (instance->getData().time < this->getIncinerationTime())
					return false;

				ENGI_LOG_INFO("Removing an instance {} from removed instances queue", instance->getName());
				m_sceneRenderer->getInstanceRegistry()->removeInstance(modelInstanceID);
				return true;
			});
	}

	bool Scene::isIncineratableMaterial(const SharedHandle<Material>& material) const noexcept
	{
		MaterialRegistry* mr = m_sceneRenderer->getRenderer()->getMaterialRegistry();
		return material == mr->getMaterial(MATERIAL_BRDF_PBR)
			|| material == mr->getMaterial(MATERIAL_BRDF_PBR_NO_CULLING)
			|| material == mr->getMaterial(MATERIAL_BRDF_PBR_DISSOLUTION);
	}

}; // engi namespace