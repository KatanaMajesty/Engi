#pragma once

#include <cstdint>
#include "Math/Math.h"

namespace engi
{

	struct Random
	{
		// Generates float in range [0, 1)
		static float GenerateFloat() noexcept;

		// Generates float in range [L, R)
		static float GenerateFloat(float L, float R) noexcept;

		static math::Vec3 GenerateFloat3(const math::Vec3& L, const math::Vec3& R) noexcept;

		// Generates uint32_t in range [L, R)
		static uint32_t GenerateUnsigned(uint32_t L, uint32_t R) noexcept;
	};

}