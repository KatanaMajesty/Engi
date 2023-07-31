#pragma once

#include <limits>
#include <numbers>

namespace engi::math
{

	struct Numeric
	{
		static constexpr float infinity() noexcept { return std::numeric_limits<float>::infinity(); }
		static constexpr float epsilon() noexcept { return std::numeric_limits<float>::epsilon(); }
		static constexpr float pi() noexcept { return std::numbers::pi_v<float>; }
		static constexpr float pi2() noexcept { return 2.0f * pi(); }
		static constexpr float phi() noexcept { return std::numbers::phi_v<float>; }

		static constexpr bool isFinite(float f) noexcept { return (f < infinity() && f > -infinity()); }
	}; // Numeric struct

}; // engi::math namespace