#pragma once

#include "Utility/Memory.h"
#include "Core/Event.h"
#include "Math/Math.h"
#include "World/ModelInstance.h"

namespace engi
{

	namespace editor
	{

		struct EvInstanceSelected : Event
		{
			EvInstanceSelected(uint32_t modelInstanceID, const InstanceIntersection& instanceIntersection, const math::Ray& ray)
				: modelInstanceID(modelInstanceID)
				, instanceIntersection(instanceIntersection)
				, ray(ray)
			{
			}

			uint32_t modelInstanceID;
			InstanceIntersection instanceIntersection;
			math::Ray ray;
		};

	};

}; // engi namespace