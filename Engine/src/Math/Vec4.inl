#pragma once

#include "Math/Vec4.h"

namespace engi::math
{

	inline bool Vec4::operator==(const Vec4& other) const noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return XMVector4Equal(v1, v2);
	}

	inline bool Vec4::operator!=(const Vec4& other) const noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return XMVector4NotEqual(v1, v2);
	}

	inline Vec4& Vec4::operator+=(const Vec4& other) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return (*this = XMVectorAdd(v1, v2));
	}

	inline Vec4& Vec4::operator-=(const Vec4& other) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return (*this = XMVectorSubtract(v1, v2));
	}

	inline Vec4& Vec4::operator*=(const Vec4& other) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return (*this = XMVectorMultiply(v1, v2));
	}

	inline Vec4& Vec4::operator*=(float scalar) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		return (*this = XMVectorScale(v1, scalar));
	}

	inline Vec4& Vec4::operator/=(float scalar) noexcept
	{
		return (this->operator*=(1.0f / scalar));
	}

	inline float Vec4::dot(const Vec4& other) const noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)*this;
		const XMVECTOR v2 = (XMVECTOR)other;
		return XMVectorGetX(XMVector4Dot(v1, v2));
	}

	inline float Vec4::length() const noexcept
	{
		const XMVECTOR v = (XMVECTOR)*this;
		return XMVectorGetX(XMVector4Length(v));
	}

	inline float Vec4::length2() const noexcept
	{
		const XMVECTOR v = (XMVECTOR)*this;
		return XMVectorGetX(XMVector4LengthSq(v));
	}

	inline void Vec4::normalize() noexcept
	{
		*this = XMVector4Normalize((XMVECTOR)*this);
	}

	inline void Vec4::normalize(Vec4* result) const noexcept
	{
		*result = XMVector4Normalize((XMVECTOR)*this);
	}

	inline void Vec4::clamp(const Vec4& min, const Vec4& max) noexcept
	{
		const XMVECTOR v = (XMVECTOR)*this;
		const XMVECTOR rawmin = (XMVECTOR)min;
		const XMVECTOR rawmax = (XMVECTOR)max;
		*this = XMVectorClamp(v, rawmin, rawmax);
	}

	inline void Vec4::clamp(const Vec4& min, const Vec4& max, Vec4* result) const noexcept
	{
		const XMVECTOR v = (XMVECTOR)*this;
		const XMVECTOR rawmin = (XMVECTOR)min;
		const XMVECTOR rawmax = (XMVECTOR)max;
		*result = XMVectorClamp(v, rawmin, rawmax);
	}

	inline float Vec4::getMin() const noexcept
	{
		float result = this->x;
		result = this->y < result ? this->y : result;
		result = this->z < result ? this->z : result;
		result = this->w < result ? this->w : result;
		return result;
	}

	inline float Vec4::getMax() const noexcept
	{
		float result = this->x;
		result = this->y > result ? this->y : result;
		result = this->z > result ? this->z : result;
		result = this->w > result ? this->w : result;
		return result;
	}

	inline Vec4 Vec4::ceil(const Vec4& v) noexcept
	{
		return Vec4(math::ceil(v.x), math::ceil(v.y), math::ceil(v.z), math::ceil(v.w));
	}

	inline Vec4 Vec4::floor(const Vec4& v) noexcept
	{
		return Vec4(math::floor(v.x), math::floor(v.y), math::floor(v.z), math::floor(v.w));
	}

	inline Vec4 Vec4::round(const Vec4& v) noexcept
	{
		return Vec4(math::round(v.x), math::round(v.y), math::round(v.z), math::round(v.w));
	}

	inline Vec4 Vec4::min(const Vec4& lhs, const Vec4& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec4(XMVectorMin(v1, v2));
	}

	inline Vec4 Vec4::max(const Vec4& lhs, const Vec4& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec4(XMVectorMax(v1, v2));
	}

	inline Vec4 operator+(const Vec4& lhs, const Vec4& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec4(XMVectorAdd(v1, v2));
	}

	inline Vec4 operator-(const Vec4& lhs, const Vec4& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec4(XMVectorSubtract(v1, v2));
	}

	inline Vec4 operator*(const Vec4& lhs, const Vec4& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec4(XMVectorMultiply(v1, v2));
	}

	inline Vec4 operator*(const Vec4& v, float scalar) noexcept
	{
		const XMVECTOR rawV = (XMVECTOR)v;
		return Vec4(XMVectorScale(rawV, scalar));
	}

	inline Vec4 operator/(const Vec4& lhs, const Vec4& rhs) noexcept
	{
		const XMVECTOR v1 = (XMVECTOR)lhs;
		const XMVECTOR v2 = (XMVECTOR)rhs;
		return Vec4(XMVectorDivide(v1, v2));
	}

	inline Vec4 operator/(const Vec4& v, float scalar) noexcept
	{
		return v * (1.0f / scalar);
	}

	Vec4 operator*(float scalar, const Vec4& v) noexcept
	{
		return v * scalar;
	}

	Vec4 operator/(float scalar, const Vec4& v) noexcept
	{
		return (1.0f / scalar) * v;
	}

}; // engi::math namespace