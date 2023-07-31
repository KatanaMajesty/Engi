#include "SpotLight.h"

namespace engi
{

	SpotLight::SpotLight(uint32_t arraySlice, const math::Vec3& color, float intensity, float radius, float cutoff, float smoothing, const math::Vec3& direction, const math::Vec3& position)
		: LightBase(color, intensity, radius)
		, m_position(position)
		, m_direction(direction)
		, m_cutoff(cutoff)
		, m_smoothing(smoothing)
		, m_depthmapArrayslice(arraySlice)
	{
	}

	math::Vec3 SpotLight::getPosition() const noexcept
	{
		if (!this->hasEntity())
			return this->getRelativePosition();

		return this->getRelativePosition() * this->getEntityToLight();
	}

	void SpotLight::setEntityID(InstanceTable* dataTable, uint32_t entityID) noexcept
	{
		m_dataTable = dataTable;
		m_entityID = entityID;
	}

	math::Mat4x4 SpotLight::getView() const noexcept
	{
		math::Vec3 pos = this->getPosition();
		return math::Mat4x4::lookToLH(pos, this->getDirection());
	}

	math::Mat4x4 SpotLight::getProjection() const noexcept
	{
		float fov = math::toRadians((getCutoff() + getSmoothing()) * 2.0f); // smoothing is multiplied by 2 to add minor offset and avoid artifacts under big angles
		return math::Mat4x4::perspectiveProjectionLH(fov, 1.0f, 30.0f, 0.01f);
	}

	const math::Mat4x4& SpotLight::getEntityToLight() const noexcept
	{
		ENGI_ASSERT(m_dataTable);
		return m_dataTable->getInstanceData(m_entityID).modelToWorld;
	}

}; // engi namespace