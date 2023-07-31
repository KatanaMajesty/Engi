#include "DirectionalLight.h"

#include <array>
#include "Core/Logger.h"
#include "Core/Application.h"
#include "Core/CommonDefinitions.h"

namespace engi
{

	DirectionalLight::DirectionalLight(const math::Vec3& ambient, const math::Vec3& direction, float radius, uint32_t arrayslice, uint32_t cubeSize)
		: LightBase(ambient, 1.0f, radius) // TODO: intensity is temporarily 1
		, m_direction(direction)
		, m_clipMatrix()
		, m_viewMatrix()
		, m_depthmapArrayslice(arrayslice)
		, m_cubeSize(cubeSize)
	{
	}

	void DirectionalLight::setDepthAnchor(const Camera& camera, float shadowMargin) noexcept
	{
		using namespace math;

		auto frustumCorners = camera.getShadowFrustumCorners();
		Vec3 center;
		for (const Vec3& corner : frustumCorners)
			center += corner;

		float texelSize = 1.0f / m_cubeSize;
		center /= static_cast<float>(frustumCorners.size());
		center = Vec3::round(center / texelSize) * texelSize; // discrete steps to minimize shadow swimming
		m_viewMatrix = Mat4x4::lookAtLH(center - m_direction, center);

		Vec3 min = frustumCorners[0] * m_viewMatrix;
		Vec3 max = frustumCorners[0] * m_viewMatrix;
		for (const Vec3& corner : frustumCorners)
		{
			Vec3 lightCorner = corner * m_viewMatrix;
			min = Vec3::min(lightCorner, min);
			max = Vec3::max(lightCorner, max);
		}

		if (min.z < 0.0f) min.z -= shadowMargin;
		else min.z += shadowMargin;

		m_clipMatrix = Mat4x4::orthographicProjectionLH(min.x, max.x, min.y, max.y, max.z, min.z);
	}

}; // engi namespace