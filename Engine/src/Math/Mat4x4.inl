#pragma once

#include "Math/Mat4x4.h"

namespace engi::math
{

	inline bool Mat4x4::operator==(const Mat4x4& other) const noexcept
	{
		Vec4 lhs0(&this->_11);
		Vec4 lhs1(&this->_21);
		Vec4 lhs2(&this->_31);
		Vec4 lhs3(&this->_41);

		Vec4 rhs0(&other._11);
		Vec4 rhs1(&other._21);
		Vec4 rhs2(&other._31);
		Vec4 rhs3(&other._41);

		return (lhs0 == rhs0)
			&& (lhs1 == rhs1)
			&& (lhs2 == rhs2)
			&& (lhs3 == rhs3);
	}

	inline bool Mat4x4::operator!=(const Mat4x4& other) const noexcept
	{
		Vec4 lhs0(&this->_11);
		Vec4 lhs1(&this->_21);
		Vec4 lhs2(&this->_31);
		Vec4 lhs3(&this->_41);

		Vec4 rhs0(&other._11);
		Vec4 rhs1(&other._21);
		Vec4 rhs2(&other._31);
		Vec4 rhs3(&other._41);

		// Prefer using operator!= due to XMVectorNotEqual
		return (lhs0 != rhs0)
			|| (lhs1 != rhs1)
			|| (lhs2 != rhs2)
			|| (lhs3 != rhs3);
	}

	inline Mat4x4& Mat4x4::operator+=(const Mat4x4& other) noexcept
	{
		const XMMATRIX m1 = (XMMATRIX)*this;
		const XMMATRIX m2 = (XMMATRIX)other;
		return (*this = Mat4x4(m1 + m2));
	}

	inline Mat4x4& Mat4x4::operator-=(const Mat4x4& other) noexcept
	{
		const XMMATRIX m1 = (XMMATRIX)*this;
		const XMMATRIX m2 = (XMMATRIX)other;
		return (*this = Mat4x4(m1 - m2));
	}

	inline Mat4x4& Mat4x4::operator*=(const Mat4x4& other) noexcept
	{
		const XMMATRIX m1 = (XMMATRIX)*this;
		const XMMATRIX m2 = (XMMATRIX)other;
		return (*this = Mat4x4(XMMatrixMultiply(m1, m2)));
	}

	inline Mat4x4& Mat4x4::operator*=(float scalar) noexcept
	{
		const XMMATRIX m = (XMMATRIX)*this;
		return (*this = Mat4x4(m * scalar));
	}

	inline Mat4x4& Mat4x4::operator/=(const Mat4x4& other) noexcept
	{
		Vec4 lhs0(&this->_11);
		Vec4 lhs1(&this->_21);
		Vec4 lhs2(&this->_31);
		Vec4 lhs3(&this->_41);

		Vec4 rhs0(&other._11);
		Vec4 rhs1(&other._21);
		Vec4 rhs2(&other._31);
		Vec4 rhs3(&other._41);

		return (*this = Mat4x4(
			lhs0 / rhs0, 
			lhs1 / rhs1, 
			lhs2 / rhs2, 
			lhs3 / rhs3));
	}

	inline Mat4x4& Mat4x4::operator/=(float scalar) noexcept
	{
		return (this->operator*=(1.0f / scalar));
	}

	inline Mat4x4 Mat4x4::operator-() const noexcept
	{
		Vec4 r0(&this->_11);
		Vec4 r1(&this->_21);
		Vec4 r2(&this->_31);
		Vec4 r3(&this->_41);
		return Mat4x4(-r0, -r1, -r2, -r3);
	}

	inline Mat4x4 Mat4x4::transpose() const noexcept
	{
		const XMMATRIX m = (XMMATRIX)*this;
		return Mat4x4(XMMatrixTranspose(m));
	}

	inline Mat4x4 Mat4x4::inverse() const noexcept
	{
		const XMMATRIX m = (XMMATRIX)*this;
		return Mat4x4(XMMatrixInverse(nullptr, m));
	}

	inline Mat4x4 Mat4x4::translation(float x, float y, float z) noexcept
	{
		return Mat4x4(XMMatrixTranslation(x, y, z));
	}

	inline Mat4x4 Mat4x4::translation(const Vec3& xyz) noexcept
	{
		return Mat4x4::translation(xyz.x, xyz.y, xyz.z);
	}

	inline Mat4x4 Mat4x4::rotationX(float radians) noexcept
	{
		return Mat4x4(XMMatrixRotationX(radians));
	}

	inline Mat4x4 Mat4x4::rotationY(float radians) noexcept
	{
		return Mat4x4(XMMatrixRotationY(radians));
	}

	inline Mat4x4 Mat4x4::rotationZ(float radians) noexcept
	{
		return Mat4x4(XMMatrixRotationZ(radians));
	}

	inline Mat4x4 Mat4x4::rotationRPY(float pitch, float yaw, float roll)
	{
		return Mat4x4(XMMatrixRotationRollPitchYaw(pitch, yaw, roll));
	}

	inline Mat4x4 Mat4x4::rotationRPY(const Vec3& angles)
	{
		return Mat4x4::rotationRPY(angles.x, angles.y, angles.z);
	}

	inline Mat4x4 Mat4x4::scale(float x, float y, float z) noexcept
	{
		return Mat4x4(XMMatrixScaling(x, y, z));
	}

	inline Mat4x4 Mat4x4::scale(float xyz) noexcept
	{
		return Mat4x4::scale(xyz, xyz, xyz);
	}

	inline Mat4x4 Mat4x4::scale(const Vec3& xyz) noexcept
	{
		return Mat4x4::scale(xyz.x, xyz.y, xyz.z);
	}

	inline Mat4x4 Mat4x4::toWorld(const Vec3& translation, const Vec3& rotation, const Vec3& scale) noexcept
	{
		const XMVECTOR t = (XMVECTOR)translation;
		const XMVECTOR r = (XMVECTOR)rotation;
		const XMVECTOR rq = XMQuaternionRotationRollPitchYawFromVector(r);
		const XMVECTOR s = (XMVECTOR)scale;
		return Mat4x4(XMMatrixAffineTransformation(s, XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), rq, t));
	}

	inline Mat4x4 Mat4x4::toWorld(const Vec3& translation, const Vec3& right, const Vec3& up, const Vec3& forward, const Vec3& scale) noexcept
	{
		Mat4x4 result;
		result._11 = right.x;
		result._12 = right.y;
		result._13 = right.z;
		result._21 = up.x;
		result._22 = up.y;
		result._23 = up.z;
		result._31 = forward.x;
		result._32 = forward.y;
		result._33 = forward.z;
		result.setTranslation(translation);
		result.setScale(scale);
		result._44 = 1.0f;
		return result;
	}

	inline Mat4x4 Mat4x4::perspectiveProjectionLH(float fov, float aspectRatio, float znear, float zfar) noexcept
	{
		return Mat4x4(XMMatrixPerspectiveFovLH(fov, aspectRatio, znear, zfar));
	}

	inline Mat4x4 Mat4x4::perspectiveProjectionRH(float fov, float aspectRatio, float znear, float zfar) noexcept
	{
		return Mat4x4(XMMatrixPerspectiveFovRH(fov, aspectRatio, znear, zfar));
	}

	inline Mat4x4 Mat4x4::orthographicProjectionLH(float left, float right, float bottom, float top, float znear, float zfar) noexcept
	{
		return Mat4x4(XMMatrixOrthographicOffCenterLH(left, right, bottom, top, znear, zfar));
	}

	inline Mat4x4 Mat4x4::orthographicProjectionRH(float left, float right, float bottom, float top, float znear, float zfar) noexcept
	{
		return Mat4x4(XMMatrixOrthographicOffCenterRH(left, right, bottom, top, znear, zfar));
	}

	inline Mat4x4 Mat4x4::lookToLH(const Vec3& eyepos, const Vec3& eyedir, const Vec3& updir) noexcept
	{
		const XMVECTOR ep = (XMVECTOR)eyepos;
		const XMVECTOR ed = (XMVECTOR)eyedir;
		const XMVECTOR ud = (XMVECTOR)updir;
		return Mat4x4(XMMatrixLookToLH(ep, ed, ud));
	}

	inline Mat4x4 Mat4x4::lookAtLH(const Vec3& eyepos, const Vec3& focus, const Vec3& updir) noexcept
	{
		const XMVECTOR ep = (XMVECTOR)eyepos;
		const XMVECTOR fp = (XMVECTOR)focus;
		const XMVECTOR ud = (XMVECTOR)updir;
		return Mat4x4(XMMatrixLookAtLH(ep, fp, ud));
	}

	inline Mat4x4 Mat4x4::lookToRH(const Vec3& eyepos, const Vec3& eyedir, const Vec3& updir) noexcept
	{
		const XMVECTOR ep = (XMVECTOR)eyepos;
		const XMVECTOR ed = (XMVECTOR)eyedir;
		const XMVECTOR ud = (XMVECTOR)updir;
		return Mat4x4(XMMatrixLookToRH(ep, ed, ud));
	}

	inline Mat4x4 Mat4x4::lookAtRH(const Vec3& eyepos, const Vec3& focus, const Vec3& updir) noexcept
	{
		const XMVECTOR ep = (XMVECTOR)eyepos;
		const XMVECTOR fp = (XMVECTOR)focus;
		const XMVECTOR ud = (XMVECTOR)updir;
		return Mat4x4(XMMatrixLookAtRH(ep, fp, ud));
	}

	inline Mat4x4 operator+(const Mat4x4& lhs, const Mat4x4& rhs) noexcept
	{
		const XMMATRIX m1 = (XMMATRIX)lhs;
		const XMMATRIX m2 = (XMMATRIX)rhs;
		return Mat4x4(m1 + m2);
	}

	inline Mat4x4 operator-(const Mat4x4& lhs, const Mat4x4& rhs) noexcept
	{
		const XMMATRIX m1 = (XMMATRIX)lhs;
		const XMMATRIX m2 = (XMMATRIX)rhs;
		return Mat4x4(m1 - m2);
	}

	inline Mat4x4 operator*(const Mat4x4& lhs, const Mat4x4& rhs) noexcept
	{
		const XMMATRIX m1 = (XMMATRIX)lhs;
		const XMMATRIX m2 = (XMMATRIX)rhs;
		return Mat4x4(XMMatrixMultiply(m1, m2));
	}

	inline Mat4x4 operator*(const Mat4x4& mat, float scalar) noexcept
	{
		const XMMATRIX m = (XMMATRIX)mat;
		return Mat4x4(m * scalar);
	}

	inline Mat4x4 operator/(const Mat4x4& lhs, const Mat4x4& rhs) noexcept
	{
		Vec4 lhs0(&lhs._11);
		Vec4 lhs1(&lhs._21);
		Vec4 lhs2(&lhs._31);
		Vec4 lhs3(&lhs._41);

		Vec4 rhs0(&rhs._11);
		Vec4 rhs1(&rhs._21);
		Vec4 rhs2(&rhs._31);
		Vec4 rhs3(&rhs._41);

		return Mat4x4(
			lhs0 / rhs0,
			lhs1 / rhs1,
			lhs2 / rhs2,
			lhs3 / rhs3
		);
	}

	inline Mat4x4 operator/(const Mat4x4& mat, float scalar) noexcept
	{
		return mat * (1.0f / scalar);
	}

	inline Vec4 operator*(const Vec4& vec, const Mat4x4& mat) noexcept
	{
		const XMVECTOR v = (XMVECTOR)vec;
		const XMMATRIX m = (XMMATRIX)mat;
		return Vec4(XMVector4Transform(v, m));
	}

	inline Vec3 operator*(const Vec3& vec, const Mat4x4& mat) noexcept
	{
		const XMVECTOR v = (XMVECTOR)vec;
		const XMMATRIX m = (XMMATRIX)mat;
		return Vec3(XMVector3Transform(v, m));
	}

}; // engi::math namespace