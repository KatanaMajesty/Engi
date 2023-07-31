#pragma once

#include "Math/Math.h"
#include "World/LightSystem/LightBase.h"
#include "World/Camera.h"

namespace engi
{

	class DirectionalLight : public LightBase
	{
	public:
		DirectionalLight() = default;
		DirectionalLight(const math::Vec3& ambient, const math::Vec3& direction, float radius, uint32_t arrayslice, uint32_t cubeSize);

		math::Vec3& getDirection() noexcept { return m_direction; }
		const math::Vec3& getDirection() const noexcept { return m_direction; }

		// void setDepthmapMipslice(uint32_t mipslice) noexcept; // we only use 1 mipslice
		void setDepthAnchor(const Camera& camera, float shadowMargin = 10.0f) noexcept;
		void setDepthmapArrayslice(uint32_t arrayslice) noexcept { m_depthmapArrayslice = arrayslice; }
		uint32_t getDepthmapArrayslice() const noexcept { return m_depthmapArrayslice; }

		const math::Mat4x4& getView() const noexcept { return m_viewMatrix; }
		const math::Mat4x4& getProjection() const noexcept { return m_clipMatrix; }

	private:
		math::Vec3 m_direction = math::Vec3(0.0f, 0.0f, 1.0f); // may be not normalized
		math::Mat4x4 m_clipMatrix = math::Mat4x4();
		math::Mat4x4 m_viewMatrix = math::Mat4x4();
		uint32_t m_depthmapArrayslice = uint32_t(-1);
		uint32_t m_cubeSize;
	};

}; // engi namespace