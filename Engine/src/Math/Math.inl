#pragma once

#include "Math/Math.h"

namespace engi::math
{

	using namespace DirectX;

	inline float cos(float radians) noexcept { return std::cos(radians); }
	inline float sin(float radians) noexcept { return std::sin(radians); }
	inline float log2(float n) noexcept { return std::log2f(n); }
	inline float floor(float val) noexcept { return std::floor(val); }
	inline float ceil(float val) noexcept { return std::ceil(val); }
	inline float round(float val) noexcept { return std::round(val); }

	inline Vec3 fibonacciHemispherePoint(float& outNdotL, uint32_t i, uint32_t n) noexcept
	{
		float theta = Numeric::pi2() * i / Numeric::phi();
		float phiCos = outNdotL = 1.0f - (i + 0.5f) / n;
		float phiSin = sqrt(1.0f - phiCos * phiCos);
		float thetaCos = cos(theta);
		float thetaSin = sin(theta);
		return Vec3(thetaCos * phiSin, thetaSin * phiSin, phiCos);
	}

	inline constexpr float toRadians(float degree) noexcept { return degree * Numeric::pi() / 180.0f; }

	inline constexpr float clamp(float val, float min, float max) noexcept
	{
		float r = val < min ? min : val;
		return r > max ? max : r;
	}

	constexpr float lerp(float from, float to, float s) noexcept
	{
		return std::lerp(from, to, s);
	}

	inline bool Ray::intersects(const Vec3& tri0, const Vec3& tri1, const Vec3& tri2, float* t) const noexcept
	{
		const XMVECTOR v0 = (XMVECTOR)tri0;
		const XMVECTOR v1 = (XMVECTOR)tri1;
		const XMVECTOR v2 = (XMVECTOR)tri2;
		const XMVECTOR o = (XMVECTOR)origin;
		const XMVECTOR d = (XMVECTOR)direction;
		float dummy; // to avoid nullptr dereference
		return TriangleTests::Intersects(o, d, v0, v1, v2, t == nullptr ? dummy : *t);
	}

	inline bool Ray::intersects(const AABB& aabb, float* t) const noexcept
	{
		Vec3 invDir = Vec3(1.0f) / direction;
		Vec3 tMin = (aabb.min - origin) * invDir;
		Vec3 tMax = (aabb.max - origin) * invDir;
		Vec3 t1 = Vec3::min(tMin, tMax);
		Vec3 t2 = Vec3::max(tMin, tMax);
		float tNear = t1.getMax();
		float tFar = t2.getMin();
		if (tNear > tFar)
		{
			return false;
		}

		if (t)
		{
			*t = tNear;
		}
		return true;
	}

}; // engi::math namespace