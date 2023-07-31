#pragma once

#include <array>
#include "Math/Math.h"

namespace engi
{

	struct ViewComponent
	{
		math::Vec3 eyePos;
		math::Vec3 eyeDir;
		math::Vec3 upDir;
		math::Vec3 rightDir; // This is used for camera movement

		math::Mat4x4 getLHView() const
		{
			return math::Mat4x4::lookToLH(eyePos, eyeDir, upDir);
		}
	}; // ViewComponent struct

	struct ProjectionComponent
	{
		float fov;
		uint32_t width;
		uint32_t height;
		float nearPlane;
		float farPlane;

		math::Mat4x4 getLHProjection() const
		{
			return math::Mat4x4::perspectiveProjectionLH(fov, static_cast<float>(width) / height, nearPlane, farPlane);
		}

		math::Mat4x4 getLHReversedDepthProjection() const
		{
			return math::Mat4x4::perspectiveProjectionLH(fov, static_cast<float>(width) / height, farPlane, nearPlane);
		}
	}; // ProjectionComponent struct

	struct FrustumComponent
	{
		math::Vec3 nearBottomLeft;
		math::Vec3 nearBottomRight;
		math::Vec3 nearTopLeft;
		math::Vec3 nearTopRight;
		math::Vec3 nearTopLeftDir; // normalized
		math::Vec3 nearBottomRightDir; // normalized
		//math::Vec3 farBottomLeft;
		//math::Vec3 farBottomRight;
		//math::Vec3 farTopLeft;
		//math::Vec3 farTopRight;
		float pixelWidth;
		float pixelHeight;
	}; // FrustumComponent struct

	class Camera
	{
	public:
		void setView(const math::Vec3& pos, const math::Vec3& viewDir, const math::Vec3& upDir);
		void setProjection(int32_t width, int32_t height, float fov, float nearPlane, float farPlane);
		void setProjectionShadow(int32_t width, int32_t height, float fov, float nearPlane, float farPlane);
		void setWorldPosition(const math::Vec3& worldOffset);
		void addWorldPosition(const math::Vec3& worldOffset);
		void addRelativePosition(const math::Vec3& offset);
		void setWorldAngles(const math::Vec3& angles);
		void addWorldAngles(const math::Vec3& angles);
		void setViewComponent(const math::Vec3& viewDir, const math::Vec3& upDir);
		void calculateView();
		void updateBasis();
		void updateProjection();
		constexpr const math::Mat4x4& getView() const { return m_view; }
		constexpr const math::Mat4x4& getViewInv() const { return m_viewInv; }
		constexpr const math::Mat4x4& getProj() const { return m_proj; }
		constexpr const math::Mat4x4& getProjInv() const { return m_projInv; }
		math::Ray castRay(const math::Vec2& windowPos) const;
		math::Vec3 getPosition() const;
		math::Vec3 getDirection() const;
		const FrustumComponent& getFrustumComponent() const noexcept { return m_frustumComp; }
		std::array<math::Vec3, 8> getShadowFrustumCorners() const noexcept;

	private:
		bool m_updatedBasis = false;
		bool m_updatedProj = false;
		math::Mat4x4 m_view;
		math::Mat4x4 m_viewInv;
		math::Mat4x4 m_proj;
		math::Mat4x4 m_projInv;
		math::Mat4x4 m_shadowProjInv; // far-z is closer
		math::Vec3 m_angles;
		ViewComponent m_viewComp;
		ProjectionComponent m_projComp;
		ProjectionComponent m_shadowProjComp;
		FrustumComponent m_frustumComp;
	}; // EulersCamera class

}; // engi namespace