#pragma once

#include "Math/Math.h"
#include "World/LightSystem/LightBase.h"
#include "Renderer/TextureCube.h"
#include "Renderer/InstanceTable.h"

namespace engi
{

	class PointLight : public LightBase
	{
	public:
		PointLight() = default;
		PointLight(uint32_t arraySlice, const math::Vec3& color, float intensity, float radius, const math::Vec3& position = math::Vec3());

		math::Vec3& getRelativePosition() noexcept { return m_position; }
		const math::Vec3& getRelativePosition() const noexcept { return m_position; }
		math::Vec3 getPosition() const noexcept;

		void setEntityID(InstanceTable* dataTable, uint32_t entityID) noexcept;
		uint32_t getEntityID() const noexcept { return m_entityID; }
		bool hasEntity() const noexcept { return m_dataTable != nullptr && m_dataTable->isOccupied(m_entityID); }

		void setDepthmapArrayslice(uint32_t arrayslice) noexcept { m_depthmapArrayslice = arrayslice; }
		uint32_t getDepthmapArrayslice() const noexcept { return m_depthmapArrayslice; }

		math::Mat4x4 getView(uint32_t index) const noexcept;
		math::Mat4x4 getProjection() const noexcept;

	private:
		const math::Mat4x4& getEntityToLight() const noexcept;

		InstanceTable* m_dataTable = nullptr;
		uint32_t m_entityID = uint32_t(-1);
		math::Vec3 m_position = math::Vec3();
		uint32_t m_depthmapArrayslice = uint32_t(-1);
	};

}; // engi namespace