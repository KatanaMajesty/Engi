#pragma once

#include "Math/Vec3.h"

namespace engi
{

	class LightBase
	{
	public:
		LightBase() = default;
		LightBase(const math::Vec3& color, float intensity, float radius)
			: m_color(color)
			, m_intensity(intensity)
			, m_radius(radius)
		{
		}

		math::Vec3& getColor() noexcept { return m_color; }
		const math::Vec3& getColor() const noexcept { return m_color; }

		float& getIntensity() noexcept { return m_intensity; }
		float getIntensity() const noexcept { return m_intensity; }

		float& getRadius() noexcept { return m_radius; }
		float getRadius() const noexcept { return m_radius; }

	private:
		math::Vec3 m_color = math::Vec3(1.0f);
		float m_intensity = 1.0f;
		float m_radius = 1.0f;
	};

}; // engi namespace