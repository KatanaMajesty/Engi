#pragma once

#include "Utility/Memory.h"
#include "World/Camera.h"
#include "World/CameraController.h"

namespace engi
{

	struct CameraPhysicalProperties
	{
		float exposureValue = 0.0f;
		float gammaCorrection = 2.2f;
	};

	struct CameraGPUProperties
	{
		float fov = 60.0f;
		float nearPlane = 0.1f;
		float farPlane = 100.0f;
		float shadowPlane = 10.0f;
		float rotationSpeed = 180.0f;
		float translationSpeed = 10.0f;
	};

	class CameraManager
	{
	public:
		CameraManager(uint32_t width, uint32_t height);
		~CameraManager() = default;

		bool init(const math::Vec3& pos, const math::Vec3& dir) noexcept;
		void update(float timestep) noexcept;
		void updateProjection() noexcept;
		void resize(uint32_t width, uint32_t height) noexcept;
		CameraPhysicalProperties& getPhysicalCameraProperties() noexcept { return m_physCamProps; }
		CameraGPUProperties& getGPUCameraProperties() noexcept { return m_gpuCamProps; }
		Camera& getCamera() noexcept { return m_camera; }

	private:
		uint32_t m_width;
		uint32_t m_height;
		bool m_isInitialized = false;
		Camera m_camera{};
		CameraController m_cameraController{};
		CameraPhysicalProperties m_physCamProps{};
		CameraGPUProperties m_gpuCamProps{};
	};

}; // engi namespace