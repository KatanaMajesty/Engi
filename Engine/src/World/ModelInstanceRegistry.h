#pragma once

#include <vector>
#include "Utility/SolidVector.h"
#include "Utility/Memory.h"
#include "Utility/ArrayView.h"
#include "Renderer/Model.h"
#include "Renderer/MaterialInstance.h"
#include "Renderer/InstanceData.h"
#include "World/ModelInstance.h"

namespace engi
{

	class SceneRenderer;

	class ModelInstanceRegistry
	{
	public:
		ModelInstanceRegistry(SceneRenderer* sceneRenderer);
		ModelInstanceRegistry(const ModelInstanceRegistry&) = delete;
		ModelInstanceRegistry& operator=(const ModelInstanceRegistry&) = delete;
		~ModelInstanceRegistry() = default;

		uint32_t addInstance(const std::string& name, const SharedHandle<Model>& model, ArrayView<const MaterialInstance> materials, const InstanceData& data) noexcept;

		// Remark: modelInstanceID != ModelInstance::getInstanceID(). Those are different IDs for different registries
		// Latter is used to access InstanceData information of the instance
		// ModelInstance itself does not know what modelInstanceID it is assigned to
		void removeInstance(uint32_t modelInstanceID) noexcept;

		// Remark: read remarks for ModelInstanceRegistry::removeInstance()
		ModelInstance* getModelInstance(uint32_t modelInstanceID) noexcept;

		// Returns true if there is a ModelInstance that is associated with this ID
		inline constexpr bool isValidID(uint32_t modelInstanceID) const noexcept { return modelInstanceID != uint32_t(-1) && m_modelInstances.isOccupied(modelInstanceID); }

		const auto& getAllModelInstanceIDs() const noexcept { return m_modelInstances.getAllIDs(); }

	private:
		SceneRenderer* m_sceneRenderer;
		SolidVector<UniqueHandle<ModelInstance>> m_modelInstances;
	};

}; // engi namespace