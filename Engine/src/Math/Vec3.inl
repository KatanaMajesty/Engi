#pragma once

#include "Math/Vec3.h"

namespace engi::math
{

	inline bool Vec3::operator==(const Vec3& other) const noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return XMVector3Equal(v1, v2);
	}

	inline bool Vec3::operator!=(const Vec3& other) const noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return XMVector3NotEqual(v1, v2);
	}

	inline Vec3& Vec3::operator+=(const Vec3& other) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return (*this = XMVectorAdd(v1, v2));
	}

	inline Vec3& Vec3::operator-=(const Vec3& other) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return (*this = XMVectorSubtract(v1, v2));
	}

	inline Vec3& Vec3::operator*=(const Vec3& other) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return (*this = XMVectorMultiply(v1, v2));
	}

	inline Vec3& Vec3::operator*=(float scalar) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		return (*this = XMVectorScale(v1, scalar));
	}
	
	inline Vec3& Vec3::operator/=(float scalar) noexcept
	{
		return (this->operator*=(1.0f / scalar));
	}

	inline float Vec3::dot(const Vec3& other) const noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return XMVectorGetX(XMVector3Dot(v1, v2));
	}

	inline Vec3 Vec3::cross(const Vec3& other) const noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return Vec3(XMVector3Cross(v1, v2));
	}

	inline float Vec3::length() const noexcept
	{
		const XMVECTOR v = (XMVECTOR)*this;
		return XMVectorGetX(XMVector3Length(v));
	}

	inline float Vec3::length2() const noexcept
	{
		const XMVECTOR v = (XMVECTOR)*this;
		return XMVectorGetX(XMVector3LengthSq(v));
	}

	inline void Vec3::normalize() noexcept
	{
		*this = XMVector3Normalize((XMVECTOR)*this);
	}

	inline void Vec3::normalize(Vec3* result) const noexcept
	{
		*result = XMVector3Normalize((XMVECTOR)*this);
	}

	inline void Vec3::clamp(const Vec3& min, const Vec3& max) noexcept
	{
		const XMVECTOR v = (XMVECTOR)*this;
		const XMVECTOR rawmin = (XMVECTOR)min;
		const XMVECTOR rawmax = (XMVECTOR)max;
		*this = XMVectorClamp(v, rawmin, rawmax);
	}

	inline void Vec3::clamp(const Vec3& min, const Vec3& max, Vec3* result) const noexcept
	{
		const XMVECTOR v = (XMVECTOR)*this;
		const XMVECTOR rawmin = (XMVECTOR)min;
		const XMVECTOR rawmax = (XMVECTOR)max;
		*result = XMVectorClamp(v, rawmin, rawmax);
	}

	inline float Vec3::getMin() const noexcept
	{
		float result = this->x;
		result = this->y < result ? this->y : result;
		result = this->z < result ? this->z : result;
		return result;
	}

	inline float Vec3::getMax() const noexcept
	{
		float result = this->x;
		result = this->y > result ? this->y : result;
		result = this->z > result ? this->z : result;
		return result;
	}

	inline Vec3 Vec3::min(const Vec3& lhs, const Vec3& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec3(XMVectorMin(v1, v2));
	}

	inline Vec3 Vec3::max(const Vec3& lhs, const Vec3& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec3(XMVectorMax(v1, v2));
	}

	inline Vec3 Vec3::ceil(const Vec3& v) noexcept
	{
		return Vec3(math::ceil(v.x), math::ceil(v.y), math::ceil(v.z));
	}

	inline Vec3 Vec3::floor(const Vec3& v) noexcept
	{
		return Vec3(math::floor(v.x), math::floor(v.y), math::floor(v.z));
	}

	inline Vec3 Vec3::round(const Vec3& v) noexcept
	{
		return Vec3(math::round(v.x), math::round(v.y), math::round(v.z));
	}

	inline float Vec3::distance(const Vec3& lhs, const Vec3& rhs) noexcept
	{
		return (rhs - lhs).length();
	}

	inline float Vec3::distance2(const Vec3& lhs, const Vec3& rhs) noexcept
	{
		return (rhs - lhs).length2();
	}

	inline Vec3 operator+(const Vec3& lhs, const Vec3& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec3(XMVectorAdd(v1, v2));
	}

	inline Vec3 operator-(const Vec3& lhs, const Vec3& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec3(XMVectorSubtract(v1, v2));
	}

	inline Vec3 operator*(const Vec3& lhs, const Vec3& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec3(XMVectorMultiply(v1, v2));
	}

	inline Vec3 operator*(const Vec3& v, float scalar) noexcept
	{
		const XMVECTOR rawV = (XMVECTOR)v;
		return Vec3(XMVectorScale(rawV, scalar));
	}

	inline Vec3 operator/(const Vec3& lhs, const Vec3& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec3(XMVectorDivide(v1, v2));
	}

	inline Vec3 operator/(const Vec3& v, float scalar) noexcept
	{
		return v * (1.0f / scalar);
	}

	Vec3 operator*(float scalar, const Vec3& v) noexcept
	{
		return v * scalar;
	}

	Vec3 operator/(float scalar, const Vec3& v) noexcept
	{
		return (1.0f / scalar) * v;
	}
	
}; // engi::math namespace