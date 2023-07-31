#pragma once

#include <cmath>
#include <DirectXMath.h>
// TODO: Remove this when ray-tri collisions are replaced
#include <DirectXCollision.h>

#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include "Math/Vec4.h"
#include "Math/Mat4x4.h"
#include "Math/Numeric.h"
#include "Math/AABB.h"

namespace engi::math
{
	using namespace DirectX;

	float cos(float radians) noexcept;
	float sin(float radians) noexcept;
	float log2(float n) noexcept;
	float floor(float val) noexcept;
	float ceil(float val) noexcept;
	float round(float val) noexcept;
	Vec3 fibonacciHemispherePoint(float& outNdotL, uint32_t i, uint32_t n) noexcept;

	constexpr float toRadians(float degree) noexcept;
	constexpr float clamp(float val, float min, float max) noexcept;
	constexpr float lerp(float from, float to, float s) noexcept;

	struct Transformation
	{
		Transformation(const Vec3& t = Vec3(0.0f), const Vec3& r = Vec3(0.0f), const Vec3& s = Vec3(1.0f))
			: translation(t)
			, rotation(toRadians(r.x), toRadians(r.y), toRadians(r.z))
			, scale(s)
		{
		}

		Vec3 translation;
		Vec3 rotation;
		Vec3 scale;
	}; // Transformation struct

	struct Ray
	{
		Ray(const Vec3& origin = Vec3(0.0f), const Vec3& direction = Vec3(0.0f, 0.0f, 1.0f))
			: origin(origin)
			, direction(direction)
		{
			this->direction.normalize();
		}

		Ray(const Ray&) = default;
		Ray& operator=(const Ray&) = default;

		Ray(Ray&&) = default;
		Ray& operator=(Ray&&) = default;

		bool intersects(const Vec3& tri0, const Vec3& tri1, const Vec3& tri2, float* t) const noexcept;
		bool intersects(const AABB& aabb, float* t) const noexcept;

		Vec3 origin;
		Vec3 direction;
	}; // Ray struct

}; // engi::math namespace

#include "Math/Math.inl"
