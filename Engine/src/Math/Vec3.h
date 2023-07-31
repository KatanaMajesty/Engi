#pragma once

#include <DirectXMath.h>
#include "Math/Vec2.h"
#include "GFX/WinAPIUndef.h"

using namespace DirectX;

namespace engi::math
{

	float ceil(float val) noexcept;
	float floor(float val) noexcept;
	float round(float val) noexcept;

	class Vec3 : public XMFLOAT3
	{
	public:
		constexpr Vec3() noexcept : XMFLOAT3(0.0f, 0.0f, 0.0f) {}
		constexpr explicit Vec3(float xyz) noexcept : XMFLOAT3(xyz, xyz, xyz) {}
		constexpr explicit Vec3(float x, float y, float z) noexcept : XMFLOAT3(x, y, z) {}
		constexpr explicit Vec3(const Vec2& xy, float z) noexcept : XMFLOAT3(xy.x, xy.y, z) {}
		explicit Vec3(const float* xyz) noexcept : XMFLOAT3(xyz) {}
		explicit Vec3(FXMVECTOR v) noexcept { XMStoreFloat3(this, v); }
		Vec3& operator=(FXMVECTOR v) noexcept { return (*this = Vec3(v)); }
		explicit Vec3(const XMFLOAT3& v) noexcept : Vec3(v.x, v.y, v.z) {}

		Vec3(const Vec3&) = default;
		Vec3& operator=(const Vec3&) = default;

		Vec3(Vec3&&) = default;
		Vec3& operator=(Vec3&&) = default;

		explicit operator XMVECTOR() const noexcept { return XMLoadFloat3(this); }

		bool operator==(const Vec3& other) const noexcept;
		bool operator!=(const Vec3& other) const noexcept;
		Vec3& operator+= (const Vec3& other) noexcept;
		Vec3& operator-= (const Vec3& other) noexcept;
		Vec3& operator*= (const Vec3& other) noexcept;
		Vec3& operator*= (float scalar) noexcept;
		Vec3& operator/= (float scalar) noexcept;
		Vec3 operator+() const noexcept { return *this; }
		Vec3 operator-() const noexcept { return Vec3(XMVectorNegate((XMVECTOR)*this)); }
		float& operator[](size_t i) noexcept { ENGI_ASSERT(i < 3); return *(&x + i); }
		const float& operator[](size_t i) const noexcept { ENGI_ASSERT(i < 3); return *(&x + i); }

		// Vector operations
		float dot(const Vec3& other) const noexcept;
		Vec3 cross(const Vec3& other) const noexcept;

		float length() const noexcept;
		float length2() const noexcept;

		void normalize() noexcept;
		void normalize(Vec3* result) const noexcept;

		void clamp(const Vec3& min, const Vec3& max) noexcept;
		void clamp(const Vec3& min, const Vec3& max, Vec3* result) const noexcept;

		float getMin() const noexcept;
		float getMax() const noexcept;

		static Vec3 min(const Vec3& lhs, const Vec3& rhs) noexcept;
		static Vec3 max(const Vec3& lhs, const Vec3& rhs) noexcept;

		static Vec3 ceil(const Vec3& v) noexcept;
		static Vec3 floor(const Vec3& v) noexcept;
		static Vec3 round(const Vec3& v) noexcept;

		static float distance(const Vec3& lhs, const Vec3& rhs) noexcept;
		static float distance2(const Vec3& lhs, const Vec3& rhs) noexcept;
	}; // Vec3 class

	// Binary operators
	inline Vec3 operator+(const Vec3& lhs, const Vec3& rhs) noexcept;
	inline Vec3 operator-(const Vec3& lhs, const Vec3& rhs) noexcept;
	inline Vec3 operator*(const Vec3& lhs, const Vec3& rhs) noexcept;
	inline Vec3 operator*(const Vec3& v, float scalar) noexcept;
	inline Vec3 operator/(const Vec3& lhs, const Vec3& rhs) noexcept;
	inline Vec3 operator/(const Vec3& v, float scalar) noexcept;
	inline Vec3 operator*(float scalar, const Vec3& v) noexcept;
	inline Vec3 operator/(float scalar, const Vec3& v) noexcept;

}; // engi::math namespace

#include "Math/Vec3.inl"