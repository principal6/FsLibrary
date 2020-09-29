﻿#pragma once


#ifndef FS_FLOAT4X4_H
#define FS_FLOAT4X4_H


#include <CommonDefinitions.h>
#include <Math/Float4.h>


namespace fs
{
#pragma region Forward declaration
	class Float3x3;
#pragma endregion


	// right-handed coordinate system
	class Float4x4 final
	{
#pragma region Static Functions
	public:
		static const Float4		mul(const Float4x4& m, const Float4& v) noexcept;
		static const Float4x4	mul(const Float4x4& l, const Float4x4& r) noexcept;
			   
		static const Float4x4	translationMatrix(const float x, const float y, const float z) noexcept;
		static const Float4x4	scalingMatrix(const float x, const float y, const float z) noexcept;
		static const Float4x4	rotationMatrixX(const float angle) noexcept;
		static const Float4x4	rotationMatrixY(const float angle) noexcept;
		static const Float4x4	rotationMatrixZ(const float angle) noexcept;

		// Rodrigues' rotation formula
		static const Float4x4	rotationMatrixAxisAngle(const Float4& axis, const float angle) noexcept;
		static const Float4x4	projectionMatrixPerspective(const float Fov, const float nearZ, const float farZ, const float ratio) noexcept;
#pragma endregion

	public:
								Float4x4();
		explicit				Float4x4(const float s);
		explicit				Float4x4(
									const float m00, const float m01, const float m02, const float m03,
									const float m10, const float m11, const float m12, const float m13,
									const float m20, const float m21, const float m22, const float m23,
									const float m30, const float m31, const float m32, const float m33);
								Float4x4(const Float4x4& rhs)																= default;
								Float4x4(Float4x4&& rhs) noexcept															= default;
								~Float4x4()																					= default;

	public:
		Float4x4&				operator=(const Float4x4& rhs)																= default;
		Float4x4&				operator=(Float4x4&& rhs) noexcept															= default;

	public:
		const Float4x4			operator+(const Float4x4& rhs) const noexcept;
		const Float4x4			operator-(const Float4x4& rhs) const noexcept;

		const Float4x4			operator*(const float s) const noexcept;
		const Float4x4			operator/(const float s) const noexcept;
		// matrix(lhs) * matrix(rhs)
		const Float4x4			operator*(const Float4x4& rhs) const noexcept;
		// matrix * vector (=> right-handed coordinate system)
		const Float4			operator*(const Float4& v) const noexcept;

	public:
		void					set(
									float m00, float m01, float m02, float m03,
									float m10, float m11, float m12, float m13,
									float m20, float m21, float m22, float m23,
									float m30, float m31, float m32, float m33) noexcept;
		void					setZero() noexcept;
		void					setIdentity() noexcept;

	public:
		const Float3x3			minor(const uint32 row, const uint32 col) const noexcept;
		const float				determinant() const noexcept;
		const Float4x4			transpose() const noexcept;
		const Float4x4			cofactor() const noexcept;
		const Float4x4			adjugate() const noexcept;
		const Float4x4			inverse() const noexcept;

	public:
		const Float4x4			mul(const Float4x4& rhs) const noexcept;
		const Float4			mul(const Float4& v) const noexcept;

	private:
		float					_m[4][4];
	};
}


#endif // !FS_FLOAT4X4_H