#pragma once

#include "World/LightSystem/SpotLight.h"
#include "World/Camera.h"

namespace engi
{

	class SceneRenderer;
	class Texture2D;

	class Flashlight
	{
	public:
		Flashlight(SceneRenderer* sceneRenderer);
		Flashlight(const Flashlight&) = delete;
		Flashlight& operator=(const Flashlight&) = delete;
		~Flashlight();

		bool init(const Camera* camera, const math::Vec3& color, float intensity, float radius, Texture2D* lightMask) noexcept;
		void reset() noexcept;
		void update() noexcept;
		SpotLight& getSpotlight() noexcept;

		inline constexpr bool isValid() const noexcept { return m_camera != nullptr; }
		inline constexpr bool hasCameraID() const noexcept { return m_cameraInstanceID != uint32_t(-1); }
		inline constexpr bool hasSpotlightID() const noexcept { return m_spotLightID != uint32_t(-1); }

	private:
		SceneRenderer* m_sceneRenderer;
		const Camera* m_camera = nullptr;
		uint32_t m_cameraInstanceID = uint32_t(-1);
		uint32_t m_spotLightID = uint32_t(-1);
	};

}; // engi namespace