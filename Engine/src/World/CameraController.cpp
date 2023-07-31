#include "World/CameraController.h"

#include "Core/CommonDefinitions.h"
#include "Core/Input.h"

namespace engi
{

	bool CameraController::init(Camera* camera) noexcept
	{
		ENGI_ASSERT(camera && "Camera cannot be nullptr");
		m_camera = camera;
		return true;
	}

	void CameraController::update(float timestep, float ts, float rs) noexcept
	{
		if (!m_camera)
		{
			return;
		}

		EngiIO& io = EngiIO::get();
		math::Vec3 relativeOffset(0.0f);
		math::Vec3 relativeAngles(0.0f);

		math::Vec2 scrollDelta = io.getMouse().getScrollDelta();
		scrollDelta.clamp(math::Vec2(0.0f), math::Vec2(20.0f));
		float acceleration = io.isPressed(Keycode::SHIFT) ? CameraControllerTraits::acceleration() : 0.2f;
		acceleration += CameraControllerTraits::minorAcceleration() * scrollDelta.y;

		if (io.isPressed(Keycode::W))
		{
			relativeOffset.z += ts * acceleration;
		}
		if (io.isPressed(Keycode::A))
		{
			relativeOffset.x -= ts * acceleration;
		}
		if (io.isPressed(Keycode::S))
		{
			relativeOffset.z -= ts * acceleration;
		}
		if (io.isPressed(Keycode::D))
		{
			relativeOffset.x += ts * acceleration;
		}
		if (io.isPressed(Keycode::CTRL))
		{
			relativeOffset.y -= ts * acceleration;
		}
		if (io.isPressed(Keycode::SPACE))
		{
			relativeOffset.y += ts * acceleration;
		}

		// If mouse offset length is a half width of the window, then rotation speed should be 180 deg / sec
		if (io.isPressed(Keycode::LMB))
		{
			math::Vec2 cursorPos = io.getCursorPosNDC();
			float degree = rs * timestep;
			relativeAngles.x -= cursorPos.y * degree;
			relativeAngles.y -= cursorPos.x * degree;
		}

		// avoid calling this each frame, because this will force to update projection and basis,
		// despite the fact, that no changes occured
		if (relativeOffset != math::Vec3(0.0f))
		{
			m_camera->addRelativePosition(relativeOffset * timestep);
		}
		if (relativeAngles != math::Vec3(0.0f))
		{
			m_camera->addWorldAngles(relativeAngles);
		}

		// Camera update
		m_camera->updateProjection();
		m_camera->updateBasis();
	}

};