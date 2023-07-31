#include "World/Camera.h"

#include <array>

namespace engi
{
	using namespace math;

	void Camera::setView(const Vec3& pos, const Vec3& viewDir, const Vec3& upDir)
	{
		setWorldPosition(pos);
		setWorldAngles(Vec3(0.0f, 90.0f, 0.0f));
		setViewComponent(viewDir, upDir);
		m_updatedBasis = false;
	}

	void Camera::setProjection(int32_t width, int32_t height, float fov, float nearPlane, float farPlane)
	{
		m_projComp.fov = fov;
		m_projComp.width = width;
		m_projComp.height = height;
		m_projComp.nearPlane = nearPlane;
		m_projComp.farPlane = farPlane;
		m_updatedProj = false;
	}

	void Camera::setProjectionShadow(int32_t width, int32_t height, float fov, float nearPlane, float farPlane)
	{
		m_shadowProjComp.fov = fov;
		m_shadowProjComp.width = width;
		m_shadowProjComp.height = height;
		m_shadowProjComp.nearPlane = nearPlane;
		m_shadowProjComp.farPlane = farPlane;
		m_updatedProj = false;
	}

	void Camera::setWorldPosition(const Vec3& worldOffset)
	{
		m_viewComp.eyePos = worldOffset;
		m_updatedBasis = false;
	}

	void Camera::addWorldPosition(const Vec3& worldOffset)
	{
		m_viewComp.eyePos += worldOffset;
		m_updatedBasis = false;
	}

	void Camera::addRelativePosition(const Vec3& offset)
	{
		Vec3 right = m_viewComp.rightDir * offset.x;
		Vec3 up = m_viewComp.upDir * offset.y;
		Vec3 front = m_viewComp.eyeDir * offset.z;
		m_viewComp.eyePos += (right + up + front);
		m_updatedBasis = false;
	}

	void Camera::setWorldAngles(const Vec3& angles)
	{
		m_angles.x = math::clamp(angles.x, -89.0f, 89.0f);
		m_angles.z = angles.z; // Ignored
		m_angles.y = angles.y;
		m_updatedBasis = false;
	}

	void Camera::addWorldAngles(const Vec3& angles)
	{
		m_angles.x = math::clamp(m_angles.x + angles.x, -89.0f, 89.0f);
		m_angles.z += angles.z; // Ignored
		m_angles.y += angles.y;
		m_updatedBasis = false;
	}

	void Camera::setViewComponent(const math::Vec3& viewDir, const math::Vec3& upDir)
	{
		viewDir.normalize(&m_viewComp.eyeDir);
		upDir.normalize(&m_viewComp.upDir);
		upDir.cross(viewDir).normalize(&m_viewComp.rightDir);
	}

	void Camera::calculateView()
	{
		Vec3 dir;
		dir.x = math::cos(toRadians(m_angles.y)) * math::cos(toRadians(m_angles.x));
		dir.y = math::sin(toRadians(m_angles.x));
		dir.z = math::sin(toRadians(m_angles.y)) * math::cos(toRadians(m_angles.x));
		setViewComponent(dir, m_viewComp.upDir);
	}

	void Camera::updateBasis()
	{
		if (m_updatedBasis)
		{
			return;
		}

		m_updatedBasis = true;

		calculateView();
		m_view = m_viewComp.getLHView();
		m_viewInv = m_view.inverse(); // TODO: Optimize inverse operation

		// ClipToWorld resize
		std::array<Vec4, 4> nearPlane = {
			Vec4(-1.0f, -1.0f, 1.0f, 1.0f),
			Vec4(-1.0f, 1.0f, 1.0f, 1.0f),
			Vec4(1.0f, 1.0f, 1.0f, 1.0f),
			Vec4(1.0f, -1.0f, 1.0f, 1.0f),
		};

		//std::array<Vec4, 4> farPlane =
		//{
		//	Vec4(-1.0f, -1.0f, 0.0f, 1.0f),
		//	Vec4(-1.0f, 1.0f, 0.0f, 1.0f),
		//	Vec4(1.0f, 1.0f, 0.0f, 1.0f),
		//	Vec4(1.0f, -1.0f, 0.0f, 1.0f),
		//};

		auto clipToWorld = m_projInv * m_viewInv;
		for (Vec4& vertex : nearPlane)
		{
			vertex = vertex * clipToWorld;
			vertex = vertex / vertex.w;
		}

		//for (Vec4& vertex : farPlane)
		//{
		//	vertex = vertex * clipToWorld;
		//	vertex = vertex / vertex.w;
		//}

		FrustumComponent& fc = m_frustumComp;
		fc.nearBottomLeft = Vec3(&nearPlane[0].x);
		fc.nearTopLeft = Vec3(&nearPlane[1].x);
		fc.nearTopRight = Vec3(&nearPlane[2].x);
		fc.nearBottomRight = Vec3(&nearPlane[3].x);
		//fc.farBottomLeft = Vec3(&farPlane[0].x);
		//fc.farTopLeft = Vec3(&farPlane[1].x);
		//fc.farTopRight = Vec3(&farPlane[2].x);
		//fc.farBottomRight = Vec3(&farPlane[3].x);
		fc.nearTopLeftDir = fc.nearTopLeft - fc.nearBottomLeft;
		fc.nearBottomRightDir = fc.nearBottomRight - fc.nearBottomLeft;
		fc.pixelWidth = fc.nearBottomRightDir.length() / m_projComp.width;
		fc.pixelHeight = fc.nearTopLeftDir.length() / m_projComp.height;
		fc.nearBottomRightDir.normalize();
		fc.nearTopLeftDir.normalize();
	}

	void Camera::updateProjection()
	{
		if (m_updatedProj)
		{
			return;
		}

		m_updatedProj = true;

		m_proj = m_projComp.getLHReversedDepthProjection();
		m_projInv = m_proj.inverse();

		Mat4x4 shadowProj = m_shadowProjComp.getLHReversedDepthProjection();
		m_shadowProjInv = shadowProj.inverse();
	}

	Ray Camera::castRay(const Vec2& windowPos) const
	{
		float scalarX = (m_frustumComp.pixelWidth * windowPos.x) + m_frustumComp.pixelWidth / 2.0f;
		float scalarY = (m_frustumComp.pixelHeight * windowPos.y) + m_frustumComp.pixelHeight / 2.0f;
		Vec3 pixelPos = m_frustumComp.nearBottomLeft
			+ (m_frustumComp.nearBottomRightDir * scalarX)
			+ (m_frustumComp.nearTopLeftDir * scalarY);
		const Vec3& cameraPos = getPosition();
		Vec3 direction = (pixelPos - cameraPos);
		direction.normalize();
		return Ray(cameraPos, direction);
	}

	Vec3 Camera::getPosition() const
	{
		//return Vec3(&m_viewComp.eyePos.x);
		return Vec3(m_viewInv._41, m_viewInv._42, m_viewInv._43);
	}

	Vec3 Camera::getDirection() const
	{
		return Vec3(m_view._13, m_view._23, m_view._33);
	}

	std::array<math::Vec3, 8> Camera::getShadowFrustumCorners() const noexcept
	{
		static const std::array<Vec4, 8> frustum = {
			Vec4(-1.0f, -1.0f, 1.0f, 1.0f),
			Vec4(-1.0f, 1.0f, 1.0f, 1.0f),
			Vec4(1.0f, 1.0f, 1.0f, 1.0f),
			Vec4(1.0f, -1.0f, 1.0f, 1.0f),
			Vec4(-1.0f, -1.0f, 0.0f, 1.0f),
			Vec4(-1.0f, 1.0f, 0.0f, 1.0f),
			Vec4(1.0f, 1.0f, 0.0f, 1.0f),
			Vec4(1.0f, -1.0f, 0.0f, 1.0f),
		};

		auto clipToWorld = m_shadowProjInv * m_viewInv;
		std::array<Vec3, 8> Ret;
		for (uint32_t i = 0; i < frustum.size(); ++i)
		{
			Vec4 corner = frustum[i] * clipToWorld;
			corner /= corner.w;
			Ret[i] = Vec3(&corner.x);
		}

		return Ret;
	}

}; // engi namespace