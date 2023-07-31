#pragma once

#include <cstring>
#include <DirectXMath.h>
#include "Math/Vec3.h"
#include "Math/Vec4.h"

using namespace DirectX;

namespace engi::math
{

	class Mat4x4 : public XMFLOAT4X4
	{
	public:
		constexpr Mat4x4() noexcept : XMFLOAT4X4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		) {}
		explicit constexpr Mat4x4(
			float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33) noexcept : XMFLOAT4X4(
				m00, m01, m02, m03,
				m10, m11, m12, m13,
				m20, m21, m22, m23,
				m30, m31, m32, m33
			) {}
		explicit constexpr Mat4x4(const Vec3& r0, const Vec3& r1, const Vec3& r2) noexcept : XMFLOAT4X4(
				r0.x, r0.y, r0.z, 0.0f,	
				r1.x, r1.y, r1.z, 0.0f,	
				r2.x, r2.y, r2.z, 0.0f,	
				0.0f, 0.0f, 0.0f, 1.0f
			) {}
		explicit constexpr Mat4x4(const Vec4& r0, const Vec4& r1, const Vec4& r2, const Vec4& r3) noexcept : XMFLOAT4X4(
				r0.x, r0.y, r0.z, r0.w,
				r1.x, r1.y, r1.z, r1.w,
				r2.x, r2.y, r2.z, r2.w,
				r3.x, r3.y, r3.z, r3.w
			) {}
		explicit Mat4x4(const float* m) noexcept : XMFLOAT4X4(m) {}
		explicit Mat4x4(CXMMATRIX m) noexcept { XMStoreFloat4x4(this, m); }
		Mat4x4& operator=(CXMMATRIX m) noexcept { return (*this = Mat4x4(m)); }
		Mat4x4(const XMFLOAT4X4& m) noexcept { std::memcpy(this, &m, sizeof(Mat4x4)); }

		Mat4x4(const Mat4x4&) = default;
		Mat4x4& operator=(const Mat4x4&) = default;

		Mat4x4(Mat4x4&&) = default;
		Mat4x4& operator=(Mat4x4&&) = default;

		explicit operator XMMATRIX() const noexcept { return XMLoadFloat4x4(this); }

		bool operator==(const Mat4x4& other) const noexcept;
		bool operator!=(const Mat4x4& other) const noexcept;
		Mat4x4& operator+=(const Mat4x4& other) noexcept;
		Mat4x4& operator-=(const Mat4x4& other) noexcept;
		Mat4x4& operator*=(const Mat4x4& other) noexcept;
		Mat4x4& operator*=(float scalar) noexcept;
		Mat4x4& operator/=(const Mat4x4& other) noexcept;
		Mat4x4& operator/=(float scalar) noexcept;
		Mat4x4 operator+() const noexcept { return *this; }
		Mat4x4 operator-() const noexcept;

		void setScale(const Vec3& v) noexcept { _11 *= v.x; _22 *= v.y; _33 *= v.z; }

		Vec3 getTranslation() const noexcept { return Vec3(_41, _42, _43); }
		void setTranslation(const Vec3& v) noexcept { _41 = v.x, _42 = v.y, _43 = v.z; }
		void addTranslation(const Vec3& v) noexcept { _41 += v.x, _42 += v.y, _43 += v.z; }

		Mat4x4 transpose() const noexcept;
		Mat4x4 inverse() const noexcept;

		static Mat4x4 translation(float x, float y, float z) noexcept;
		static Mat4x4 translation(const Vec3& xyz) noexcept;

		static Mat4x4 rotationX(float radians) noexcept;
		static Mat4x4 rotationY(float radians) noexcept;
		static Mat4x4 rotationZ(float radians) noexcept;
		static Mat4x4 rotationRPY(float pitch, float yaw, float roll); // (xyz)
		static Mat4x4 rotationRPY(const Vec3& angles);

		static Mat4x4 scale(float x, float y, float z) noexcept;
		static Mat4x4 scale(float xyz) noexcept;
		static Mat4x4 scale(const Vec3& xyz) noexcept;

		static Mat4x4 toWorld(const Vec3& translation, const Vec3& rotation, const Vec3& scale) noexcept;
		static Mat4x4 toWorld(const Vec3& translation, const Vec3& right, const Vec3& up, const Vec3& forward, const Vec3& scale) noexcept;
		static Mat4x4 perspectiveProjectionLH(float fov, float aspectRatio, float znear, float zfar) noexcept;
		static Mat4x4 perspectiveProjectionRH(float fov, float aspectRatio, float znear, float zfar) noexcept;
		static Mat4x4 orthographicProjectionLH(float left, float right, float bottom, float top, float znear, float zfar) noexcept;
		static Mat4x4 orthographicProjectionRH(float left, float right, float bottom, float top, float znear, float zfar) noexcept;
		static Mat4x4 lookToLH(const Vec3& eyepos, const Vec3& eyedir, const Vec3& updir = Vec3(0.0f, 1.0f, 0.0f)) noexcept;
		static Mat4x4 lookAtLH(const Vec3& eyepos, const Vec3& focus, const Vec3& updir = Vec3(0.0f, 1.0f, 0.0f)) noexcept;
		static Mat4x4 lookToRH(const Vec3& eyepos, const Vec3& eyedir, const Vec3& updir = Vec3(0.0f, 1.0f, 0.0f)) noexcept;
		static Mat4x4 lookAtRH(const Vec3& eyepos, const Vec3& focus, const Vec3& updir = Vec3(0.0f, 1.0f, 0.0f)) noexcept;
	}; // Mat4x4 class

	inline Mat4x4 operator+(const Mat4x4& lhs, const Mat4x4& rhs) noexcept;
	inline Mat4x4 operator-(const Mat4x4& lhs, const Mat4x4& rhs) noexcept;
	inline Mat4x4 operator*(const Mat4x4& lhs, const Mat4x4& rhs) noexcept;
	inline Mat4x4 operator*(const Mat4x4& mat, float scalar) noexcept;
	inline Mat4x4 operator/(const Mat4x4& lhs, const Mat4x4& rhs) noexcept;
	inline Mat4x4 operator/(const Mat4x4& mat, float scalar) noexcept;

	inline Vec4 operator*(const Vec4& vec, const Mat4x4& mat) noexcept;
	inline Vec3 operator*(const Vec3& vec, const Mat4x4& mat) noexcept; // assumes vec.w = 1

}; // engi::math namespace

#include "Math/Mat4x4.inl"