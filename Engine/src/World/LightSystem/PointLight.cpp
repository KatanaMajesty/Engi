#include "PointLight.h"

namespace engi
{

	PointLight::PointLight(uint32_t arraySlice, const math::Vec3& color, float intensity, float radius, const math::Vec3& position)
		: LightBase(color, intensity, radius)
		, m_position(position)
		, m_depthmapArrayslice(arraySlice)
	{
	}

    math::Vec3 PointLight::getPosition() const noexcept
    {
        if (!this->hasEntity())
            return this->getRelativePosition();

        return this->getRelativePosition() * this->getEntityToLight();
    }

    void PointLight::setEntityID(InstanceTable* dataTable, uint32_t entityID) noexcept
    {
        m_dataTable = dataTable;
        m_entityID = entityID;
    }

    math::Mat4x4 PointLight::getView(uint32_t index) const noexcept
    {
		static math::Vec3 directionTable[] =
        {
             math::Vec3(1.0f, 0.0f, 0.0f),
             math::Vec3(-1.0f, 0.0f, 0.0f),
             math::Vec3(0.0f, 1.0f, 0.0f),
             math::Vec3(0.0f, -1.0f, 0.0f),
             math::Vec3(0.0f, 0.0f, 1.0f),
             math::Vec3(0.0f, 0.0f, -1.0f),
        };

        static math::Vec3 upTable[] =
        {
             math::Vec3(0.0f, 1.0f,  0.0f),
             math::Vec3(0.0f, 1.0f,  0.0f),
             math::Vec3(0.0f, 0.0f, -1.0f),
             math::Vec3(0.0f, 0.0f,  1.0f),
             math::Vec3(0.0f, 1.0f,  0.0f),
             math::Vec3(0.0f, 1.0f,  0.0f),
        };

        math::Vec3 pos = this->getPosition();
        return math::Mat4x4::lookAtLH(pos, pos + directionTable[index], upTable[index]);
    }

    math::Mat4x4 PointLight::getProjection() const noexcept
    {
        return math::Mat4x4::perspectiveProjectionLH(math::toRadians(90.0f), 1.0f, 30.0f, 0.01f);
    }

    const math::Mat4x4& PointLight::getEntityToLight() const noexcept
    {
        ENGI_ASSERT(m_dataTable);
        return m_dataTable->getInstanceData(m_entityID).modelToWorld;
    }

}; // engi namespace