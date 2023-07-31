#pragma once

#include "World/ModelInstanceRegistry.h"
#include "Math/Math.h"

namespace engi
{

	class InstanceDragger
	{
	public:
		void init(ModelInstanceRegistry* registry) noexcept;
		void update(const math::Ray& ray);

	private:
		void performIntersection(const math::Ray& ray);
		void intersected(uint32_t modelInstanceID, const InstanceIntersection& intersection);
		void performDragging(const math::Ray& ray);
		void performRelease();
		
		ModelInstanceRegistry* m_registry = nullptr;
		uint32_t m_modelInstanceID = uint32_t(-1);
		math::Vec3 m_delta;
		math::Vec3 m_hitpos;
		float m_t = -1.0f;
		bool m_active = false;
	};

}; // engi namespace