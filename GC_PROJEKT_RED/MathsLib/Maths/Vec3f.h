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
    class Vec3f
    {
    public:
        __m128 reg;

        Vec3f(float x, float y, float z) : reg(_mm_setr_ps(x, y, z, 0.0f)) {}

        Vec3f() : reg(_mm_setzero_ps()) {}

        Vec3f(__m128 val) : reg(val) {}

        inline Vec3f operator+(const Vec3f& rhs) const;
        inline Vec3f operator-(const Vec3f& rhs) const;
        inline Vec3f operator*(const Vec3f& rhs) const;
        inline Vec3f operator/(const Vec3f& rhs) const;
        inline Vec3f operator-() const;

        template <std::floating_point T = float>
        inline Vec3f operator*(T scalar) const;
        template <std::floating_point T = float>
        inline Vec3f operator/(T scalar) const;
        inline Vec3f& operator+=(const Vec3f& rhs);
        inline Vec3f& operator-=(const Vec3f& rhs);
        inline Vec3f& operator*=(const Vec3f& rhs);
        inline Vec3f& operator/=(const Vec3f& rhs);
        template <std::floating_point T = float>
        inline Vec3f& operator*=(T scalar);
        template <std::floating_point T = float>
        inline Vec3f& operator/=(T scalar);
        inline bool operator==(const Vec3f& rhs) const;
        inline bool operator!=(const Vec3f& rhs) const;
        /*
        inline float Dot(const Vec3f& rhs) const;
        inline float MagnitudeSquared() const;
        inline float Magnitude() const;

        inline Vec3f Normalize() const;
        inline float DistanceSquared(const Vec3f& rhs) const;
        inline float Distance(const Vec3f& rhs) const;
        inline float Angle(const Vec3f& rhs) const;

        inline template <std::floating_point T = float>
        inline static Vec3f Lerp(const Vec3f& a, const Vec3f& b, T t);
        inline static Vec3f Min(const Vec3f& a, const Vec3f& b);
        inline static Vec3f Max(const Vec3f& a, const Vec3f& b);

        inline static const Vec3f Zero;
        inline static const Vec3f One;
        inline static const Vec3f UnitX;
        inline static const Vec3f UnitY;
        inline static const Vec3f UnitZ;
        inline static const Vec3f UnitW;*/
    };


    inline Vec3f Vec3f::operator+(const Vec3f& rhs) const
    {
        //float res[4];
        //_mm_store_ps(res,_mm_add_ps(reg, rhs.reg));
        //Vec3f result = { res[0],res[1] ,res[2] };     //Old version of the operator, still here for the records / SLOWER than new version (~10% slower);
        //return result;
        return _mm_add_ps(reg, rhs.reg);
    }

    inline Vec3f Vec3f::operator-(const Vec3f& rhs) const
    {
        return _mm_sub_ps(reg, rhs.reg);
    }

    inline Vec3f Vec3f::operator*(const Vec3f& rhs) const
    {
        return _mm_mul_ps(reg, rhs.reg);
    }

    inline Vec3f Vec3f::operator/(const Vec3f& rhs) const
    {
        assert(rhs.reg.m128_f32[0] != 0.f and rhs.reg.m128_f32[1] != 0.f and rhs.reg.m128_f32[2] != 0.f);
        return _mm_div_ps(reg, rhs.reg);
    }

    inline Vec3f Vec3f::operator-() const
    {
        __m128 e = _mm_set_ss(-0.0f);
        return _mm_xor_ps(reg, _mm_shuffle_ps(e, e, 0x00));
    }

    template <std::floating_point T>
    inline Vec3f Vec3f::operator*(T scalar) const
    {
        return _mm_mul_ps(reg, _mm_set1_ps(static_cast<float>(scalar)));
    }

    template <std::floating_point T>
    inline Vec3f Vec3f::operator/(T scalar) const
    {
        assert(scalar != 0.f);
        return _mm_div_ps(reg, _mm_set1_ps(static_cast<float>(scalar)));
    }

    inline Vec3f& Vec3f::operator+=(const Vec3f& rhs)
    {
        reg = _mm_add_ps(reg, rhs.reg);
        return *this;
    }

    inline Vec3f& Vec3f::operator-=(const Vec3f& rhs)
    {
        reg = _mm_sub_ps(reg, rhs.reg);
        return *this;
    }

    inline Vec3f& Vec3f::operator*=(const Vec3f& rhs)
    {
        reg = _mm_mul_ps(reg, rhs.reg);
        return *this;
    }

    inline Vec3f& Vec3f::operator/=(const Vec3f& rhs)
    {
        assert(rhs.reg.m128_f32[0] != 0.f and rhs.reg.m128_f32[1] != 0.f and rhs.reg.m128_f32[2] != 0.f);
        reg = _mm_div_ps(reg, rhs.reg);
        return *this;
    }

    template <std::floating_point T>
    inline Vec3f& Vec3f::operator*=(T scalar)
    {
        reg = _mm_mul_ps(reg, _mm_set1_ps(static_cast<float>(scalar)));
        return *this;
    }

    template <std::floating_point T>
    inline Vec3f& Vec3f::operator/=(T scalar)
    {
        assert(scalar != 0.f);
        reg = _mm_div_ps(reg, _mm_set1_ps(static_cast<float>(scalar)));
        return *this;
    }

    inline bool Vec3f::operator==(const Vec3f& rhs) const
    {
        return ((_mm_movemask_ps(_mm_cmpeq_ps(reg, rhs.reg)) == 0xF));
        //return _mm_movemask_ps(_mm_cmpeq_ps(reg, rhs.reg));
    }

    inline bool Vec3f::operator!=(const Vec3f& rhs) const
    {
        return ((_mm_movemask_ps(_mm_cmpeq_ps(reg, rhs.reg)) != 0xF));
        //return !(_mm_movemask_ps(_mm_cmpeq_ps(reg, rhs.reg)));
    }
    /*
    float Vec3f::Dot(const Vec3f& rhs) const
    {
    }

    float Vec3f::MagnitudeSquared() const
    {
        return Dot(*this);
    }

    float Vec3f::Magnitude() const
    {
    }

    Vec3f Vec3f::Normalize() const
    {
    }

    float Vec3f::DistanceSquared(const Vec3f& rhs) const
    {
        return (*this - rhs).MagnitudeSquared();
    }

    float Vec3f::Distance(const Vec3f& rhs) const
    {
        return (*this - rhs).Magnitude();
    }

    float Vec3f::Angle(const Vec3f& rhs) const
    {
    }

    template <std::floating_point T>
    Vec3f Vec3f::Lerp(const Vec3f& a, const Vec3f& b, T t)
    {
    }

    Vec3f Vec3f::Min(const Vec3f& a, const Vec3f& b)
    {
    }

    Vec3f Vec3f::Max(const Vec3f& a, const Vec3f& b)
    {
    }

    const Vec3f Vec3f::Zero{0, 0, 0, 0};

    const Vec3f Vec3f::One{1, 1, 1, 1};

    const Vec3f Vec3f::UnitX{1, 0, 0, 0};

    const Vec3f Vec3f::UnitY{0, 1, 0, 0};

    const Vec3f Vec3f::UnitZ{0, 0, 1, 0};

    const Vec3f Vec3f::UnitW{0, 0, 0, 1};

    template <std::floating_point T>
    Vec3f operator*(T scalar, const Vec3f& vector)
    {
        return vector * scalar;
    }*/

}
