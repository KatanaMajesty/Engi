#pragma once

#include <DirectXMath.h>
#include "Core/CommonDefinitions.h"
#include "GFX/WinAPIUndef.h"

using namespace DirectX;

namespace engi::math
{

	float ceil(float val) noexcept;
	float floor(float val) noexcept;
	float round(float val) noexcept;

	class Vec2 : public XMFLOAT2
	{
	public:
		constexpr Vec2() noexcept : XMFLOAT2(0.0f, 0.0f) {}
		constexpr explicit Vec2(float xy) noexcept : XMFLOAT2(xy, xy) {}
		constexpr explicit Vec2(float x, float y) noexcept : XMFLOAT2(x, y) {}
		explicit Vec2(const float* xy) noexcept : XMFLOAT2(xy) {}
		explicit Vec2(FXMVECTOR v) noexcept { XMStoreFloat2(this, v); }
		Vec2& operator=(FXMVECTOR v) noexcept { return (*this = Vec2(v)); }
		explicit Vec2(const XMFLOAT2& v) noexcept : Vec2(v.x, v.y) {}

		Vec2(const Vec2&) = default;
		Vec2& operator=(const Vec2&) = default;

		Vec2(Vec2&&) = default;
		Vec2& operator=(Vec2&&) = default;

		explicit operator XMVECTOR() const noexcept { return XMLoadFloat2(this); }

		bool operator==(const Vec2& other) const noexcept;
		bool operator!=(const Vec2& other) const noexcept;
        Vec2& operator+= (const Vec2& other) noexcept;
        Vec2& operator-= (const Vec2& other) noexcept;
        Vec2& operator*= (const Vec2& other) noexcept;
        Vec2& operator*= (float scalar) noexcept;
        Vec2& operator/= (float scalar) noexcept;
        Vec2 operator+() const noexcept { return *this; }
        Vec2 operator-() const noexcept { return Vec2(XMVectorNegate((XMVECTOR)*this)); }
		float& operator[](size_t i) noexcept { ENGI_ASSERT(i < 2); return *(&x + i); }
		const float& operator[](size_t i) const noexcept { ENGI_ASSERT(i < 2); return *(&x + i); }

        // Vector operations
		float dot(const Vec2& other) const noexcept;
		Vec2 cross(const Vec2& other) const noexcept;

        float length() const noexcept;
        float length2() const noexcept;

        void normalize() noexcept;
        void normalize(Vec2* result) const noexcept;

        void clamp(const Vec2& min, const Vec2& max) noexcept;
        void clamp(const Vec2& min, const Vec2& max, Vec2* result) const noexcept;

		float getMin() const noexcept;
		float getMax() const noexcept;

		static Vec2 min(const Vec2& lhs, const Vec2& rhs) noexcept;
		static Vec2 max(const Vec2& lhs, const Vec2& rhs) noexcept;

		static Vec2 ceil(const Vec2& v) noexcept;
		static Vec2 floor(const Vec2& v) noexcept;
		static Vec2 round(const Vec2& v) noexcept;

		static float distance(const Vec2& lhs, const Vec2& rhs) noexcept;
		static float distance2(const Vec2& lhs, const Vec2& rhs) noexcept;
	}; // Vec2 class

	// Binary operators
	inline Vec2 operator+(const Vec2& lhs, const Vec2& rhs) noexcept;
	inline Vec2 operator-(const Vec2& lhs, const Vec2& rhs) noexcept;
	inline Vec2 operator*(const Vec2& lhs, const Vec2& rhs) noexcept;
	inline Vec2 operator*(const Vec2& v, float scalar) noexcept;
	inline Vec2 operator/(const Vec2& lhs, const Vec2& rhs) noexcept;
	inline Vec2 operator/(const Vec2& v, float scalar) noexcept;
	inline Vec2 operator*(float scalar, const Vec2& v) noexcept;
	inline Vec2 operator/(float scalar, const Vec2& v) noexcept;

}; // engi::math namespace

#include "Math/Vec2.inl"