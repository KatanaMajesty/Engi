#pragma once

#include "World/Camera.h"

namespace engi
{

	struct CameraControllerTraits
	{
		static inline constexpr float acceleration() { return 1.0f; }
		static inline constexpr float minorAcceleration() { return acceleration() * 0.05f; }
	};

	class CameraController
	{
	public:
		bool init(Camera* camera) noexcept;
		void update(float timestep, float ts, float rs) noexcept;

	private:
		Camera* m_camera = nullptr;
	};

}; // engi namespace