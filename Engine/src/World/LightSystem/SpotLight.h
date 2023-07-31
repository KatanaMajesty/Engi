#pragma once

#include "Math/Math.h"
#include "World/LightSystem/LightBase.h"
#include "Renderer/Texture2D.h"
#include "Renderer/InstanceTable.h"

namespace engi
{

	class SpotLight : public LightBase
	{
	public:
		SpotLight() = default;
		SpotLight(uint32_t arraySlice, const math::Vec3& color, float intensity, float radius,
			float cutoff = 15.0f, float smoothing = 3.0f, const math::Vec3& direction = math::Vec3(0.0f, 0.0f, 1.0f), const math::Vec3& position = math::Vec3());

		void setCookie(Texture2D* cookie) noexcept { m_cookie = cookie; }
		Texture2D* getCookie() noexcept { return m_cookie; }
		const Texture2D* getCookie() const noexcept { return m_cookie; }

		math::Vec3& getRelativePosition() noexcept { return m_position; }
		const math::Vec3& getRelativePosition() const noexcept { return m_position; }
		math::Vec3 getPosition() const noexcept;

		math::Vec3& getDirection() noexcept { return m_direction; }
		const math::Vec3& getDirection() const noexcept { return m_direction; }

		float& getCutoff() noexcept { return m_cutoff; }
		float getCutoff() const noexcept { return m_cutoff; }

		float& getSmoothing() noexcept { return m_smoothing; }
		float getSmoothing() const noexcept { return m_smoothing; }

		void setEntityID(InstanceTable* dataTable, uint32_t entityID) noexcept;
		uint32_t getEntityID() const noexcept { return m_entityID; }
		bool hasEntity() const noexcept { return m_dataTable != nullptr && m_dataTable->isOccupied(m_entityID); }

		void setDepthmapArrayslice(uint32_t arrayslice) noexcept { m_depthmapArrayslice = arrayslice; }
		uint32_t getDepthmapArrayslice() const noexcept { return m_depthmapArrayslice; }

		math::Mat4x4 getView() const noexcept;
		math::Mat4x4 getProjection() const noexcept;

	private:
		const math::Mat4x4& getEntityToLight() const noexcept;

		InstanceTable* m_dataTable = nullptr;
		uint32_t m_entityID = uint32_t(-1);
		Texture2D* m_cookie = nullptr;
		math::Vec3 m_position = math::Vec3();
		math::Vec3 m_direction = math::Vec3(0.0f, 0.0f, 1.0f);
		float m_cutoff = 15.0f;
		float m_smoothing = 3.0f;
		uint32_t m_depthmapArrayslice = uint32_t(-1);
	};

}; // engi namespace