#include "World/CameraManager.h"

#include "Math/Math.h"

namespace engi
{

	CameraManager::CameraManager(uint32_t width, uint32_t height)
		: m_width(width)
		, m_height(height)
	{
	}

	bool CameraManager::init(const math::Vec3& pos, const math::Vec3& dir) noexcept
	{
		m_camera = Camera();
		m_camera.setView(pos, dir, math::Vec3(0.0f, 1.0f, 0.0f));
		m_camera.setProjection(m_width, m_height, math::toRadians(m_gpuCamProps.fov), m_gpuCamProps.nearPlane, m_gpuCamProps.farPlane);
		m_camera.setProjectionShadow(m_width, m_height, math::toRadians(m_gpuCamProps.fov), m_gpuCamProps.nearPlane, m_gpuCamProps.shadowPlane);
		if (!m_cameraController.init(&m_camera))
			return false;

		m_isInitialized = true;
		return true;
	}

	void CameraManager::update(float timestep) noexcept
	{
		m_cameraController.update(timestep, m_gpuCamProps.translationSpeed, m_gpuCamProps.rotationSpeed);
	}

	void CameraManager::updateProjection() noexcept
	{
		m_camera.setProjection(m_width, m_height, math::toRadians(m_gpuCamProps.fov), m_gpuCamProps.nearPlane, m_gpuCamProps.farPlane);
		m_camera.setProjectionShadow(m_width, m_height, math::toRadians(m_gpuCamProps.fov), m_gpuCamProps.nearPlane, m_gpuCamProps.shadowPlane);
	}

	void CameraManager::resize(uint32_t width, uint32_t height) noexcept
	{
		m_width = width;
		m_height = height;
		updateProjection();
	}

}; // engi namespace