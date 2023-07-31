#pragma once

#include <array>
#include "Math/Math.h"
#include "GFX/Definitions.h"

namespace engi
{

	struct alignas(16) InstanceData
	{
		InstanceData() = default;
		InstanceData(const math::Transformation& transform, const math::Vec3& color = math::Vec3(), const math::Vec3& emission = math::Vec3(), float emissionPow = 0.0f);

		static std::array<gfx::GpuInputAttributeDesc, 16> getInputAttributes(uint32_t inputSlot) noexcept;

		math::Mat4x4 modelToWorld = math::Mat4x4();
		math::Mat4x4 worldToModel = math::Mat4x4();
		math::Vec3 color = math::Vec3();

		float time = 0.0f;
		float timestep = 0.0f; // timestep added to time at the current frame

		math::Vec3 emission = math::Vec3();
		float emissionPower = 0.0f;

		math::Vec3 sphereOrigin = math::Vec3();
		float sphereRadiusMax = 0.0f;

		uint32_t instanceID = uint32_t(-1);
	};

}; // engi namespace