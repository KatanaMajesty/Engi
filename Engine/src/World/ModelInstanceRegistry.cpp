#include "World/ModelInstanceRegistry.h"

#include <algorithm>
#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "World/SceneRenderer.h"

namespace engi
{

	ModelInstanceRegistry::ModelInstanceRegistry(SceneRenderer* sceneRenderer)
		: m_sceneRenderer(sceneRenderer)
	{
		ENGI_ASSERT(sceneRenderer);
	}

	uint32_t ModelInstanceRegistry::addInstance(const std::string& name, const SharedHandle<Model>& model, ArrayView<const MaterialInstance> materials, const InstanceData& data) noexcept
	{
		uint32_t modelInstanceID = uint32_t(-1);
		if (!model)
		{
			ENGI_LOG_WARN("Failed to init model instance {}", name);
			return modelInstanceID;
		}

		ModelInstance* mi = new ModelInstance(m_sceneRenderer, name);
		if (!mi->init(model, materials, data))
		{
			ENGI_LOG_WARN("Failed to init model instance {}", name);
			delete mi;
			return modelInstanceID;
		}

		modelInstanceID = m_modelInstances.insert(std::move(makeUnique<ModelInstance>(mi)));
		return modelInstanceID;
	}

	void ModelInstanceRegistry::removeInstance(uint32_t modelInstanceID) noexcept
	{
		if (!this->isValidID(modelInstanceID))
			return;

		m_modelInstances.erase(modelInstanceID);
	}

	ModelInstance* ModelInstanceRegistry::getModelInstance(uint32_t modelInstanceID) noexcept
	{
		if (!this->isValidID(modelInstanceID))
			return nullptr;

		return m_modelInstances[modelInstanceID].get();
	}

}; // engi namespace