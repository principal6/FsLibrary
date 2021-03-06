﻿#pragma once


#ifndef MINT_FLOAT3_H
#define MINT_FLOAT3_H


#include <MintMath/Include/Float2.h>


namespace mint
{
    // No SIMD
    class Float3 final
    {
    public:
        static const Float3         kUnitScale;

    public:
        constexpr                   Float3();
        constexpr explicit          Float3(const float s);
        constexpr explicit          Float3(const float x, const float y, const float z);
        constexpr explicit          Float3(const Float2& rhs);
        constexpr                   Float3(const Float3& rhs)           = default;
        constexpr                   Float3(Float3&& rhs) noexcept       = default;
                                    ~Float3()                           = default;

    public:
        Float3&                     operator=(const Float3& rhs)        = default;
        Float3&                     operator=(Float3&& rhs) noexcept    = default;

    public:
        Float3&                     operator+=(const Float3& rhs);
        Float3&                     operator-=(const Float3& rhs);
        Float3&                     operator*=(const float s);
        Float3&                     operator/=(const float s);

    public:
        const Float3&               operator+() const noexcept;
        Float3                      operator-() const noexcept;

    public:
        Float3                      operator+(const Float3& rhs) const noexcept;
        Float3                      operator-(const Float3& rhs) const noexcept;
        Float3                      operator*(const float s) const noexcept;
        Float3                      operator/(const float s) const noexcept;

    public:
        float&                      operator[](const uint32 index) noexcept;
        const float&                operator[](const uint32 index) const noexcept;

    public:
        const bool                  operator==(const Float3& rhs) const noexcept;
        const bool                  operator!=(const Float3& rhs) const noexcept;

    public:
        static const float          dotProductRaw(const float* const a, const float* const b) noexcept;

        // for matrix multiplication
        static const float          dotProductRaw(const float (&a)[3], const float bX, const float bY, const float bZ) noexcept;

    public:
        static const float          dot(const Float3& lhs, const Float3& rhs) noexcept;
        static Float3               cross(const Float3& lhs, const Float3& rhs) noexcept;
        static Float3               crossAndNormalize(const Float3& lhs, const Float3& rhs) noexcept;
        static Float3               normalize(const Float3& float3) noexcept;
    
    public:
        void                        normalize() noexcept;

    public:
        const float                 lengthSqaure() const noexcept;
        const float                 length() const noexcept;

    public:
        void                        set(const float x, const float y, const float z) noexcept;
        Float2                      xy() const noexcept;
        
    public:
        float                       _x;
        float                       _y;
        float                       _z;
    };
}


#include <MintMath/Include/Float3.inl>


#endif // !MINT_FLOAT3_H
