#include "World/InstanceDragger.h"

#include "Core/Application.h"
#include "Core/CommonDefinitions.h"
#include "Core/Input.h"

namespace engi
{

	void InstanceDragger::init(ModelInstanceRegistry* registry) noexcept
	{
		m_registry = registry;
	}

	void InstanceDragger::update(const math::Ray& ray)
	{
		if (!m_registry)
			return;

		if (EngiIO::get().isPressed(Keycode::RMB))
		{
			(m_active ? performDragging(ray) : performIntersection(ray));
		}
		else if (m_active)
		{
			performRelease();
		}
	}

	void InstanceDragger::performIntersection(const math::Ray& ray)
	{
		ENGI_ASSERT(m_registry);

		InstanceIntersection result;
		result.meshIntersection.reset();
		uint32_t modelInstanceID = uint32_t(-1);

		// TODO: Right now instance dragger stores ModelInstance* to drag an instnace. It might become a dangling pointer, thus it needs to be changed!
		for (uint32_t currentModelInstanceID : m_registry->getAllModelInstanceIDs())
		{
			ModelInstance* modelInstance = m_registry->getModelInstance(currentModelInstanceID);
			InstanceIntersection i = modelInstance->intersect(ray);
			if (result > i)
			{
				result = i;
				modelInstanceID = currentModelInstanceID;
			}
		}

		if (result.isValid())
		{
			m_active = true;
			intersected(modelInstanceID, result);
		}
	}

	void InstanceDragger::intersected(uint32_t modelInstanceID, const InstanceIntersection& intersection)
	{
		ENGI_ASSERT(modelInstanceID != uint32_t(-1));

		ModelInstance* modelInstance = m_registry->getModelInstance(modelInstanceID);
		InstanceData& data = modelInstance->getData();

		m_t = intersection.meshIntersection.t;
		m_modelInstanceID = modelInstanceID;
		math::Vec3 instancePos = data.modelToWorld.getTranslation();
		m_delta = instancePos - intersection.meshIntersection.hitpos;
	}

	void InstanceDragger::performDragging(const math::Ray& ray)
	{
		ModelInstance* modelInstance = m_registry->getModelInstance(m_modelInstanceID);
		if (!modelInstance)
		{
			this->performRelease();
			return;
		}

		InstanceData& data = modelInstance->getData();

		math::Vec3 current = ray.origin + (ray.direction * m_t);
		math::Vec3 instancePos = data.modelToWorld.getTranslation();
		math::Vec3 offset = current - instancePos + m_delta;
		data.modelToWorld.addTranslation(offset);
		data.worldToModel = data.modelToWorld.inverse();
		modelInstance->updateMeshData();
	}

	void InstanceDragger::performRelease()
	{
		//ENGI_ASSERT(m_instance != && "Instance cannot be nullptr");
		m_active = false;
	}

}; // engi namespace