#pragma once

#include <immintrin.h>
#include <type_traits>
#include <algorithm>
#include <cmath>
#include <concepts>
#include <iostream>
#include <stdexcept>
#include <cassert>

namespace Maths
{
    class Vec4f
    {
    public:
        __m128 reg;

        Vec4f(float x, float y, float z, float w) : reg(_mm_setr_ps(x, y, z, w)) {}

        Vec4f() : reg(_mm_setzero_ps()) {}

        Vec4f(__m128 val) : reg(val) {}

        Vec4f operator+(const Vec4f& rhs) const;
        Vec4f operator-(const Vec4f& rhs) const;
        Vec4f operator*(const Vec4f& rhs) const;
        Vec4f operator/(const Vec4f& rhs) const;
        /*
        Vec4f operator-() const;
        template <std::floating_point T = float>
        Vec4f operator*(T scalar) const;
        template <std::floating_point T = float>
        Vec4f operator/(T scalar) const;
        Vec4f& operator+=(const Vec4f& rhs);
        Vec4f& operator-=(const Vec4f& rhs);
        Vec4f& operator*=(const Vec4f& rhs);
        Vec4f& operator/=(const Vec4f& rhs);
        template <std::floating_point T = float>
        Vec4f& operator*=(T scalar);
        template <std::floating_point T = float>
        Vec4f& operator/=(T scalar);
        bool operator==(const Vec4f& rhs) const;
        bool operator!=(const Vec4f& rhs) const;

        float Dot(const Vec4f& rhs) const;
        float MagnitudeSquared() const;
        float Magnitude() const;

        Vec4f Normalize() const;
        float DistanceSquared(const Vec4f& rhs) const;
        float Distance(const Vec4f& rhs) const;
        float Angle(const Vec4f& rhs) const;

        template <std::floating_point T = float>
        static Vec4f Lerp(const Vec4f& a, const Vec4f& b, T t);
        static Vec4f Min(const Vec4f& a, const Vec4f& b);
        static Vec4f Max(const Vec4f& a, const Vec4f& b);

        static const Vec4f Zero;
        static const Vec4f One;
        static const Vec4f UnitX;
        static const Vec4f UnitY;        
        static const Vec4f UnitZ;
        static const Vec4f UnitW;*/
    };

    // ---------- float ----------

    Vec4f Vec4f::operator+(const Vec4f& rhs) const
    {
        //float res[4];
        //_mm_store_ps(res,_mm_add_ps(reg, rhs.reg));
        //Vec4f result = { res[0],res[1] ,res[2] ,res[3] };
        //return result;
        return _mm_add_ps(reg, rhs.reg);
    }

    Vec4f Vec4f::operator-(const Vec4f& rhs) const
    {
        return _mm_sub_ps(reg, rhs.reg);
    }


    Vec4f Vec4f::operator*(const Vec4f& rhs) const
    {
        return _mm_mul_ps(reg, rhs.reg);
    }

    Vec4f Vec4f::operator/(const Vec4f& rhs) const
    {
        assert(rhs.reg.m128_f32[0] != 0.f and rhs.reg.m128_f32[1] != 0.f and rhs.reg.m128_f32[2] != 0.f and rhs.reg.m128_f32[3] != 0.f);
        return _mm_div_ps(reg, rhs.reg);

    }

    /*Vec4f Vec4f::operator-() const
    {
        
    }

    template <std::floating_point T>
    Vec4f Vec4f::operator*(T scalar) const
    {
    }

    template <std::floating_point T>
    Vec4f Vec4f::operator/(T scalar) const
    {
    }

    Vec4f& Vec4f::operator+=(const Vec4f& rhs)
    {
        *this = *this + rhs;
        return *this;
    }

    Vec4f& Vec4f::operator-=(const Vec4f& rhs)
    {
        *this = *this - rhs;
        return *this;
    }

    Vec4f& Vec4f::operator*=(const Vec4f& rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    Vec4f& Vec4f::operator/=(const Vec4f& rhs)
    {
        *this = *this / rhs;
        return *this;
    }

    template <std::floating_point T>
    Vec4f& Vec4f::operator*=(T scalar)
    {
        *this = *this * scalar;
        return *this;
    }

    template <std::floating_point T>
    Vec4f& Vec4f::operator/=(T scalar)
    {
        *this = *this / scalar;
        return *this;
    }

    bool Vec4f::operator==(const Vec4f& rhs) const
    {
    }

    bool Vec4f::operator!=(const Vec4f& rhs) const
    {
        return !(*this == rhs);
    }

    float Vec4f::Dot(const Vec4f& rhs) const
    {
    }

    float Vec4f::MagnitudeSquared() const
    {
        return Dot(*this);
    }

    float Vec4f::Magnitude() const
    {
    }

    Vec4f Vec4f::Normalize() const
    {
    }

    float Vec4f::DistanceSquared(const Vec4f& rhs) const
    {
        return (*this - rhs).MagnitudeSquared();
    }

    float Vec4f::Distance(const Vec4f& rhs) const
    {
        return (*this - rhs).Magnitude();
    }

    float Vec4f::Angle(const Vec4f& rhs) const
    {
    }

    template <std::floating_point T>
    Vec4f Vec4f::Lerp(const Vec4f& a, const Vec4f& b, T t)
    {
    }

    Vec4f Vec4f::Min(const Vec4f& a, const Vec4f& b)
    {
    }

    Vec4f Vec4f::Max(const Vec4f& a, const Vec4f& b)
    {
    }

    const Vec4f Vec4f::Zero{0, 0, 0, 0};

    const Vec4f Vec4f::One{1, 1, 1, 1};

    const Vec4f Vec4f::UnitX{1, 0, 0, 0};

    const Vec4f Vec4f::UnitY{0, 1, 0, 0};

    const Vec4f Vec4f::UnitZ{0, 0, 1, 0};

    const Vec4f Vec4f::UnitW{0, 0, 0, 1};

    template <std::floating_point T>
    Vec4f operator*(T scalar, const Vec4f& vector)
    {
        return vector * scalar;
    }*/

}
