#include "World/ModelInstance.h"

#include <algorithm>
#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "Renderer/MeshManager.h"
#include "Renderer/InstanceTable.h"
#include "World/SceneRenderer.h"
#include "World/LightSystem/LightManager.h"

namespace engi
{

	ModelInstance::ModelInstance(SceneRenderer* sceneRenderer, const std::string& name)
		: m_name(name)
		, m_sceneRenderer(sceneRenderer)
	{
		ENGI_ASSERT(sceneRenderer && "Scene renderer cannot be nullptr");
	}

	ModelInstance::~ModelInstance()
	{
		MeshManager* meshManager = m_sceneRenderer->getMeshManager();
		uint32_t numMeshes = getNumMeshInstances();
		for (uint32_t meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
		{
			const MaterialInstance& materialInstance = m_meshInstances[meshIndex].materialInstance;
			bool r = meshManager->removeInstance(m_model, meshIndex, materialInstance, m_instanceID);
			ENGI_ASSERT(r && "Failed to remove instance!");
		}

		this->removeAllDecals();

		LightManager* lightManager = m_sceneRenderer->getLightManager();
		if (this->hasSpotLight())
			lightManager->removeSpotLight(m_spotLightID);

		if (this->hasPointLight())
			lightManager->removePointLight(m_pointLightID);

		if (this->hasParticleEmitter())
		{
			ParticleSystem* particleSystem = m_sceneRenderer->getParticleSystem();
			particleSystem->removeEmitter(m_particleEmitterID);
		}

		meshManager->getInstanceTable()->removeInstanceData(m_instanceID);
	}

	bool ModelInstance::init(const SharedHandle<Model>& model, ArrayView<const MaterialInstance> materials, const InstanceData& data) noexcept
	{
		ENGI_ASSERT(model && model->getNumStaticMeshes() > 0 && "Weird model");
		uint32_t numMeshes = model->getNumStaticMeshes();
		uint32_t numMaterials = static_cast<uint32_t>(materials.size());
		ENGI_ASSERT(numMaterials > 0);

		MeshManager* meshManager = m_sceneRenderer->getMeshManager();
		m_model = model;
		m_meshInstances.reserve(numMeshes);

		InstanceTable* instanceTable = meshManager->getInstanceTable();
		m_instanceID = instanceTable->addInstanceData(data);

		InstanceData& emplacedData = instanceTable->getInstanceData(m_instanceID);
		emplacedData.instanceID = m_instanceID;

		for (uint32_t meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
		{
			uint32_t materialIndex = (meshIndex >= numMaterials) ? numMaterials - 1 : meshIndex;
			const MaterialInstance& materialInstance = materials[materialIndex];
			// std::cout << std::format("Submitting mesh index {} with material instance {}", materialIndex, materialInstance.getName()) << std::endl;
			m_meshInstances.emplace_back(StaticMeshInstance(&model->getStaticMeshEntries()[meshIndex].mesh, materialInstance));
			if (!meshManager->submitInstance(model, meshIndex, materialInstance, m_instanceID))
			{
				ENGI_LOG_WARN("Failed to submit instance of mesh {} pf {} model", meshIndex, model->getPath());
				return false;
			}
		}

		return true;
	}

	InstanceData& ModelInstance::getData() noexcept
	{
		ENGI_ASSERT(m_instanceID != uint32_t(-1) && "Not initted");

		MeshManager* meshManager = m_sceneRenderer->getMeshManager();
		return meshManager->getInstanceTable()->getInstanceData(m_instanceID);
	}

	const InstanceData& ModelInstance::getData() const noexcept
	{
		return const_cast<ModelInstance*>(this)->getData();
	}

	MaterialInstance ModelInstance::getMaterialInstance(uint32_t meshIndex) const noexcept
	{
		ENGI_ASSERT(meshIndex < getNumMeshInstances());

		const StaticMeshInstance& meshInstance = m_meshInstances[meshIndex];
		return meshInstance.hasMaterial() ? meshInstance.materialInstance : MaterialInstance::empty();
	}

	void ModelInstance::setMaterialInstance(uint32_t meshIndex, const MaterialInstance& materialInstance) noexcept
	{
		ENGI_ASSERT(meshIndex < getNumMeshInstances());

		StaticMeshInstance& meshInstance = m_meshInstances[meshIndex];
		if (materialInstance == meshInstance.materialInstance)
			return;

		MeshManager* meshManager = m_sceneRenderer->getMeshManager();
		if (meshInstance.hasMaterial() && !meshManager->removeInstance(m_model, meshIndex, meshInstance.materialInstance, m_instanceID))
		{
			ENGI_LOG_WARN("Failed to remove the material instance for editing");
			return;
		}

		meshInstance.setMaterial(materialInstance);
		bool r = meshManager->submitInstance(m_model, meshIndex, materialInstance, m_instanceID);
		ENGI_ASSERT(r);
	}

	void ModelInstance::updateMeshData() noexcept
	{
		ENGI_ASSERT(m_instanceID != uint32_t(-1) && "Not initted");

		MeshManager* meshManager = m_sceneRenderer->getMeshManager();
		uint32_t numMeshes = getNumMeshInstances();
		for (uint32_t meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
		{
			const MaterialInstance& materialInstance = m_meshInstances[meshIndex].materialInstance;
			bool r = meshManager->updateInstance(m_model, meshIndex, materialInstance, m_instanceID);
			ENGI_ASSERT(r && "Failed to update instance!");
		}
	}

	math::AABB ModelInstance::getAABB() const noexcept
	{
		uint32_t numMeshes = this->getNumMeshInstances();
		ENGI_ASSERT(numMeshes > 0);

		const math::Mat4x4 instanceToWorld = this->getData().modelToWorld;
		math::AABB aabb = m_meshInstances[0].mesh->getAABB();
		math::Mat4x4 meshToWorld = m_meshInstances[0].mesh->getMeshToModel() * instanceToWorld;
		math::AABB worldAABB = aabb.applyMatrix(meshToWorld);

		math::AABB result = worldAABB;
		for (uint32_t meshIndex = 1; meshIndex < numMeshes; ++meshIndex)
		{
			const StaticMeshInstance& meshInstance = m_meshInstances[meshIndex];

			aabb = meshInstance.mesh->getAABB();
			meshToWorld = meshInstance.mesh->getMeshToModel() * instanceToWorld;
			worldAABB = aabb.applyMatrix(meshToWorld);
			
			// TODO: Im not sure if we need to iterate through all of the points as we are in the correct space already
			result.min = math::Vec3::min(result.min, worldAABB.min);
			result.max = math::Vec3::max(result.max, worldAABB.max);
		}
		return result;
	}

	void ModelInstance::addPointLight(const math::Vec3& color, float intensity, float radius) noexcept
	{
		LightManager* lightManager = m_sceneRenderer->getLightManager();
		if (this->hasPointLight())
		{
			lightManager->removePointLight(m_pointLightID);
		}

		m_pointLightID = lightManager->createPointLight(color, intensity, radius);
		PointLight& light = lightManager->getPointLight(m_pointLightID);

		// Set instance ID of a Light to this instance
		MeshManager* meshManager = m_sceneRenderer->getMeshManager();
		light.setEntityID(meshManager->getInstanceTable(), m_instanceID);
		lightManager->update();
	}

	bool ModelInstance::hasPointLight() const noexcept
	{
		LightManager* lightManager = m_sceneRenderer->getLightManager();
		return m_pointLightID != uint32_t(-1) && lightManager->getAllPointLights().isOccupied(m_pointLightID);
	}

	PointLight* ModelInstance::getPointLight() noexcept
	{
		if (!this->hasPointLight())
			return nullptr;

		LightManager* lightManager = m_sceneRenderer->getLightManager();
		PointLight& light = lightManager->getPointLight(m_pointLightID);
		ENGI_ASSERT(light.hasEntity());
		return &light;
	}

	void ModelInstance::updatePointLight() noexcept
	{
		ENGI_ASSERT(this->hasPointLight());
		m_sceneRenderer->getLightManager()->update();
	}

	void ModelInstance::addSpotLight(const math::Vec3& color, float intensity, float radius) noexcept
	{
		LightManager* lightManager = m_sceneRenderer->getLightManager();
		if (this->hasSpotLight())
		{
			lightManager->removeSpotLight(m_spotLightID);
		}

		m_spotLightID = lightManager->createPointLight(color, intensity, radius);
		auto& light = lightManager->getSpotLight(m_spotLightID);

		// Set instance ID of a Light to this instance
		MeshManager* meshManager = m_sceneRenderer->getMeshManager();
		light.setEntityID(meshManager->getInstanceTable(), m_instanceID);
		lightManager->update();
	}

	bool ModelInstance::hasSpotLight() const noexcept
	{
		LightManager* lightManager = m_sceneRenderer->getLightManager();
		return m_spotLightID != uint32_t(-1) && lightManager->getAllSpotLights().isOccupied(m_spotLightID);
	}

	SpotLight* ModelInstance::getSpotLight() noexcept
	{
		if (!this->hasSpotLight())
			return nullptr;

		LightManager* lightManager = m_sceneRenderer->getLightManager();
		SpotLight& light = lightManager->getSpotLight(m_spotLightID);
		ENGI_ASSERT(light.hasEntity());
		return &light;
	}

	void ModelInstance::updateSpotLight() noexcept
	{
		ENGI_ASSERT(this->hasSpotLight());
		m_sceneRenderer->getLightManager()->update();
	}

	void ModelInstance::addParticleEmitter(Texture2D* mvea, Texture2D* dbf, Texture2D* rlu) noexcept
	{
		ParticleSystem* particleSystem = m_sceneRenderer->getParticleSystem();
		if (this->hasParticleEmitter())
		{
			particleSystem->removeEmitter(m_particleEmitterID);
		}

		MeshManager* meshManager = m_sceneRenderer->getMeshManager();
		m_particleEmitterID = particleSystem->addEmitter(meshManager->getInstanceTable(), m_instanceID, mvea, dbf, rlu);
	}

	bool ModelInstance::hasParticleEmitter() const noexcept
	{
		ParticleSystem* particleSystem = m_sceneRenderer->getParticleSystem();
		return m_particleEmitterID != uint32_t(-1) && particleSystem->isEmitterID(m_particleEmitterID);
	}

	SmokeEmitter* ModelInstance::getParticleEmitter() noexcept
	{
		if (!this->hasParticleEmitter())
			return nullptr;

		ParticleSystem* particleSystem = m_sceneRenderer->getParticleSystem();
		SmokeEmitter& emitter = particleSystem->getEmitter(m_particleEmitterID);
		return &emitter;
	}

	void ModelInstance::updateParticleEmitter() noexcept
	{
		ENGI_ASSERT(this->hasParticleEmitter());
		// Nothing to do here, maybe for future optimizations
	}


	void ModelInstance::addDecal(Texture2D* normalMap, const math::Vec3& albedo, const math::Mat4x4& decalToWorld) noexcept
	{
		DecalManager* decalManager = m_sceneRenderer->getDecalManager();
		uint32_t decalID = decalManager->createDecal(this->getInstanceID(), normalMap, decalToWorld, math::Vec3(), albedo, 0.5f, 0.5f);
		m_decalIDs.push_back(decalID);
	}

	void ModelInstance::removeAllDecals() noexcept
	{
		DecalManager* decalManager = m_sceneRenderer->getDecalManager();
		for (uint32_t decalID : m_decalIDs)
		{
			decalManager->removeDecal(decalID);
			ENGI_LOG_TRACE("ModelInstance {}: Removed Decal {}", this->getName(), decalID);
		}

		m_decalIDs.clear();
	}

	InstanceIntersection ModelInstance::intersect(const math::Ray& ray) const noexcept
	{
		StaticMeshTriangleOctree::IntersectionType intersection;
		intersection.reset();
		uint32_t intersectedMeshIndex = uint32_t(-1);

		uint32_t numMeshes = m_model->getNumStaticMeshes();
		ENGI_ASSERT(numMeshes > 0);
		const InstanceData& data = getData();
		math::Vec3 modelPos = ray.origin * data.worldToModel;
		math::Vec4 modelDir = math::Vec4(ray.direction, 0.0f) * data.worldToModel;
		math::Ray modelRay(modelPos, math::Vec3(&modelDir.x));
		for (uint32_t meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
		{
			const StaticMeshEntry& entry = m_model->getStaticMeshEntries()[meshIndex];
			math::Vec3 meshPos = modelRay.origin * entry.mesh.getModelToMesh();
			math::Vec4 meshDir = math::Vec4(modelRay.direction, 0.0f) * entry.mesh.getModelToMesh();
			math::Ray meshRay(meshPos, math::Vec3(&meshDir.x));
			StaticMeshTriangleOctree::IntersectionType i = entry.bvh.intersect(meshRay);
			if (i < intersection)
			{
				intersection = i;
				intersectedMeshIndex = meshIndex;
			}
		}

		// cast StaticMeshIntersection info from mesh space to world space
		if (intersection.isValid())
		{
			StaticMeshEntry& entry = m_model->getStaticMeshEntries()[intersectedMeshIndex];
			math::Vec3 worldHitpos = (intersection.hitpos * entry.mesh.getMeshToModel()) * data.modelToWorld;
			float t = math::Vec3::distance(ray.origin, worldHitpos);

			intersection.t = t;
			intersection.hitpos = worldHitpos;
		}

		InstanceIntersection result;
		result.meshIndex = intersectedMeshIndex;
		result.meshIntersection = intersection;
		return result;
	}

}; // engi namespace