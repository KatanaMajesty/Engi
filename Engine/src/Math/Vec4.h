#pragma once

#include <DirectXMath.h>
#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include "GFX/WinAPIUndef.h"

using namespace DirectX;

namespace engi::math
{

	float ceil(float val) noexcept;
	float floor(float val) noexcept;
	float round(float val) noexcept;

	class Vec4 : public XMFLOAT4
	{
	public:
		constexpr Vec4() noexcept : XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) {}
		constexpr explicit Vec4(float xyzw) noexcept : XMFLOAT4(xyzw, xyzw, xyzw, xyzw) {}
		constexpr explicit Vec4(float x, float y, float z, float w = 1.0f) noexcept : XMFLOAT4(x, y, z, w) {}
		constexpr explicit Vec4(const Vec2& xy, float z, float w = 1.0f) noexcept : Vec4(xy.x, xy.y, z, w) {}
		constexpr explicit Vec4(const Vec3& xyz, float w = 1.0f) noexcept : Vec4(xyz.x, xyz.y, xyz.z, w) {}
		explicit Vec4(const float* xyzw) noexcept : XMFLOAT4(xyzw) {}
		explicit Vec4(FXMVECTOR v) noexcept { XMStoreFloat4(this, v); }
		Vec4& operator=(FXMVECTOR v) noexcept { return (*this = Vec4(v)); }
		explicit Vec4(const XMFLOAT4& v) noexcept : Vec4(v.x, v.y, v.z, v.w) {}

		Vec4(const Vec4&) = default;
		Vec4& operator=(const Vec4&) = default;

		Vec4(Vec4&&) = default;
		Vec4& operator=(Vec4&&) = default;

		explicit operator XMVECTOR() const noexcept { return XMLoadFloat4(this); }

		bool operator==(const Vec4& other) const noexcept;
		bool operator!=(const Vec4& other) const noexcept;
		Vec4& operator+=(const Vec4& other) noexcept;
		Vec4& operator-=(const Vec4& other) noexcept;
		Vec4& operator*=(const Vec4& other) noexcept;
		Vec4& operator*=(float scalar) noexcept;
		Vec4& operator/=(float scalar) noexcept;
		Vec4 operator+() const noexcept { return *this; }
		Vec4 operator-() const noexcept { return Vec4(XMVectorNegate((XMVECTOR)*this)); }
		float& operator[](size_t i) noexcept { ENGI_ASSERT(i < 4); return *(&x + i); }
		const float& operator[](size_t i) const noexcept { ENGI_ASSERT(i < 4); return *(&x + i); }

		// Vector operations
		float dot(const Vec4& other) const noexcept;

		float length() const noexcept;
		float length2() const noexcept;

		void normalize() noexcept;
		void normalize(Vec4* result) const noexcept;

		void clamp(const Vec4& min, const Vec4& max) noexcept;
		void clamp(const Vec4& min, const Vec4& max, Vec4* result) const noexcept;

		float getMin() const noexcept;
		float getMax() const noexcept;

		static Vec4 ceil(const Vec4& v) noexcept;
		static Vec4 floor(const Vec4& v) noexcept;
		static Vec4 round(const Vec4& v) noexcept;

		static Vec4 min(const Vec4& lhs, const Vec4& rhs) noexcept;
		static Vec4 max(const Vec4& lhs, const Vec4& rhs) noexcept;
	}; // Vec4 class

	// Binary operators
	inline Vec4 operator+(const Vec4& lhs, const Vec4& rhs) noexcept;
	inline Vec4 operator-(const Vec4& lhs, const Vec4& rhs) noexcept;
	inline Vec4 operator*(const Vec4& lhs, const Vec4& rhs) noexcept;
	inline Vec4 operator*(const Vec4& v, float scalar) noexcept;
	inline Vec4 operator/(const Vec4& lhs, const Vec4& rhs) noexcept;
	inline Vec4 operator/(const Vec4& v, float scalar) noexcept;
	inline Vec4 operator*(float scalar, const Vec4& v) noexcept;
	inline Vec4 operator/(float scalar, const Vec4& v) noexcept;

}; // engi::math namespace

#include "Math/Vec4.inl"