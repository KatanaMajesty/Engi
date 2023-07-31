#pragma once

#include "Math/AABB.h"

namespace engi::math
{

	inline Vec3 AABB::size() const noexcept
	{
		return max - min;
	}

	inline Vec3 AABB::center() const noexcept
	{
		return (min + max) * 0.5f;
	}

	inline constexpr bool AABB::contains(const Vec3& point) const noexcept
	{
		return min.x <= point.x && point.x <= max.x
			&& min.y <= point.y && point.y <= max.y
			&& min.z <= point.z && point.z <= max.z;
	}

	inline AABB AABB::applyMatrix(const Mat4x4& matrix) const noexcept
	{
		Vec3 pos[8];
		pos[0] = min;
		pos[1] = Vec3(min.x, max.y, min.z);
		pos[2] = Vec3(max.x, max.y, min.z);
		pos[3] = Vec3(max.x, min.y, min.z);
		pos[4] = max;
		pos[5] = Vec3(max.x, min.y, max.z);
		pos[6] = Vec3(min.x, min.y, max.z);
		pos[7] = Vec3(min.x, max.y, max.z);

		Vec3 tMin = pos[0] * matrix;
		Vec3 tMax = tMin;
		for (uint32_t i = 1; i < 8; ++i)
		{
			Vec3 t = pos[i] * matrix;
			tMin = Vec3::min(tMin, t);
			tMax = Vec3::max(tMax, t);
		}

		return AABB(tMin, tMax);
	}

}; // engi::math namespace