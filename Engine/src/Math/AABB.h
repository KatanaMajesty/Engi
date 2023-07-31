#pragma once

#include "Math/Vec3.h"
#include "Math/Mat4x4.h"
#include "GFX/WinAPIUndef.h"

namespace engi::math
{

	struct AABB
	{
		AABB(const Vec3& min = Vec3(-1.0f), const Vec3& max = Vec3(1.0f))
			: min(min)
			, max(max)
		{
		}

		AABB(const AABB&) = default;
		AABB& operator=(const AABB&) = default;

		AABB(AABB&&) = default;
		AABB& operator=(AABB&&) = default;

		// AABB::size() effectively returns the vector of the diagonal
		// thus AABB::size().length() would give the diagonal length of the AABB
		Vec3 size() const noexcept;
		Vec3 center() const noexcept;
		constexpr bool contains(const Vec3& point) const noexcept;

		AABB applyMatrix(const Mat4x4& matrix) const noexcept;

		Vec3 min;
		Vec3 max;
	}; // AABB struct

}; // engi::math namespace

#include "Math/AABB.inl"