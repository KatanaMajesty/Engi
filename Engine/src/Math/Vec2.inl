#pragma once

#include "Math/Vec2.h"

namespace engi::math
{

	inline bool Vec2::operator==(const Vec2& other) const noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return XMVector2Equal(v1, v2);
	}

	inline bool Vec2::operator!=(const Vec2& other) const noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return XMVector2NotEqual(v1, v2);
	}

	inline Vec2& Vec2::operator+=(const Vec2& other) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return (*this = XMVectorAdd(v1, v2));
	}

	inline Vec2& Vec2::operator-=(const Vec2& other) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return (*this = XMVectorSubtract(v1, v2));
	}

	inline Vec2& Vec2::operator*=(const Vec2& other) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return (*this = XMVectorMultiply(v1, v2));
	}

	inline Vec2& Vec2::operator*=(float scalar) noexcept
	{
		const XMVECTOR v = (XMVECTOR)*this;
		return (*this = XMVectorScale(v, scalar));
	}

	inline Vec2& Vec2::operator/=(float scalar) noexcept
	{
		return (this->operator*=(1.0f / scalar));
	}

	inline float Vec2::dot(const Vec2& other) const noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return XMVectorGetX(XMVector2Dot(v1, v2));
	}

	inline Vec2 Vec2::cross(const Vec2& other) const noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return Vec2(XMVector2Cross(v1, v2));
	}

	inline float Vec2::length() const noexcept
	{
		const XMVECTOR v = (XMVECTOR)*this;
		return XMVectorGetX(XMVector2Length(v));
	}

	inline float Vec2::length2() const noexcept
	{
		const XMVECTOR v = (XMVECTOR)*this;
		return XMVectorGetX(XMVector2LengthSq(v));
	}

	inline void Vec2::normalize() noexcept
	{
		*this = XMVector2Normalize((XMVECTOR)*this);
	}

	inline void Vec2::normalize(Vec2* result) const noexcept
	{
		*result = XMVector2Normalize((XMVECTOR)*this);
	}

	inline void Vec2::clamp(const Vec2& min, const Vec2& max) noexcept
	{
		const XMVECTOR v = (XMVECTOR)*this;
		const XMVECTOR rawmin = (XMVECTOR)min;
		const XMVECTOR rawmax = (XMVECTOR)max;
		*this = XMVectorClamp(v, rawmin, rawmax);
	}

	inline void Vec2::clamp(const Vec2& min, const Vec2& max, Vec2* result) const noexcept
	{
		const XMVECTOR v = (XMVECTOR)*this;
		const XMVECTOR rawmin = (XMVECTOR)min;
		const XMVECTOR rawmax = (XMVECTOR)max;
		*result = XMVectorClamp(v, rawmin, rawmax);
	}

	inline float Vec2::getMin() const noexcept
	{
		return this->x < this->y ? this->x : this->y;
	}

	inline float Vec2::getMax() const noexcept
	{
		return this->x > this->y ? this->x : this->y;
	}

	inline Vec2 Vec2::min(const Vec2& lhs, const Vec2& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec2(XMVectorMin(v1, v2));
	}

	inline Vec2 Vec2::max(const Vec2& lhs, const Vec2& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec2(XMVectorMax(v1, v2));
	}

	inline Vec2 Vec2::ceil(const Vec2& v) noexcept
	{
		return Vec2(math::ceil(v.x), math::ceil(v.y));
	}

	inline Vec2 Vec2::floor(const Vec2& v) noexcept
	{
		return Vec2(math::floor(v.x), math::floor(v.y));
	}

	inline Vec2 Vec2::round(const Vec2& v) noexcept
	{
		return Vec2(math::round(v.x), math::round(v.y));
	}

	inline float Vec2::distance(const Vec2& lhs, const Vec2& rhs) noexcept
	{
		return (rhs - lhs).length();
	}

	inline float Vec2::distance2(const Vec2& lhs, const Vec2& rhs) noexcept
	{
		return (rhs - lhs).length2();
	}

	inline Vec2 operator+(const Vec2& lhs, const Vec2& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec2(XMVectorAdd(v1, v2));
	}

	inline Vec2 operator-(const Vec2& lhs, const Vec2& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec2(XMVectorSubtract(v1, v2));
	}

	inline Vec2 operator*(const Vec2& lhs, const Vec2& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec2(XMVectorMultiply(v1, v2));
	}

	inline Vec2 operator*(const Vec2& v, float scalar) noexcept
	{
		const XMVECTOR rawV = (XMVECTOR)v;
		return Vec2(XMVectorScale(rawV, scalar));
	}

	inline Vec2 operator/(const Vec2& lhs, const Vec2& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec2(XMVectorDivide(v1, v2));
	}

	inline Vec2 operator/(const Vec2& v, float scalar) noexcept
	{
		return v * (1.0f / scalar);
	}

	Vec2 operator*(float scalar, const Vec2& v) noexcept
	{
		return v * scalar;
	}

	Vec2 operator/(float scalar, const Vec2& v) noexcept
	{
		return (1.0f / scalar) * v;
	}

}; // engi::math namespace