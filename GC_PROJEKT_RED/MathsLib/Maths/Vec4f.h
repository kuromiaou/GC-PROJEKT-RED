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

        inline Vec4f operator+(const Vec4f& rhs) const;
        inline Vec4f operator-(const Vec4f& rhs) const;
        inline Vec4f operator*(const Vec4f& rhs) const;
        inline Vec4f operator/(const Vec4f& rhs) const;        
        inline Vec4f operator-() const;

        template <std::floating_point T = float>
        inline Vec4f operator*(T scalar) const;
        template <std::floating_point T = float>
        inline Vec4f operator/(T scalar) const;
        inline Vec4f& operator+=(const Vec4f& rhs);
        inline Vec4f& operator-=(const Vec4f& rhs);
        inline Vec4f& operator*=(const Vec4f& rhs);
        inline Vec4f& operator/=(const Vec4f& rhs);
        template <std::floating_point T = float>
        inline Vec4f& operator*=(T scalar);
        template <std::floating_point T = float>
        inline Vec4f& operator/=(T scalar);
        inline bool operator==(const Vec4f& rhs) const;
        inline bool operator!=(const Vec4f& rhs) const;
        
        inline float Dot(const Vec4f& rhs) const;
        /*inline float MagnitudeSquared() const;
        inline float Magnitude() const;

        inline Vec4f Normalize() const;
        inline float DistanceSquared(const Vec4f& rhs) const;
        inline float Distance(const Vec4f& rhs) const;
        inline float Angle(const Vec4f& rhs) const;

        template <std::floating_point T = float>
        inline static Vec4f Lerp(const Vec4f& a, const Vec4f& b, T t);
        inline static Vec4f Min(const Vec4f& a, const Vec4f& b);
        inline static Vec4f Max(const Vec4f& a, const Vec4f& b);
        */
        static const Vec4f Zero;
        static const Vec4f One;
        static const Vec4f UnitX;
        static const Vec4f UnitY;        
        static const Vec4f UnitZ;
        static const Vec4f UnitW;
    };


    inline Vec4f Vec4f::operator+(const Vec4f& rhs) const
    {
        //float res[4];
        //_mm_store_ps(res,_mm_add_ps(reg, rhs.reg));
        //Vec4f result = { res[0],res[1] ,res[2] ,res[3] };
        //return result;
        return _mm_add_ps(reg, rhs.reg);
    }

    inline Vec4f Vec4f::operator-(const Vec4f& rhs) const
    {
        return _mm_sub_ps(reg, rhs.reg);
    }

    inline Vec4f Vec4f::operator*(const Vec4f& rhs) const
    {
        return _mm_mul_ps(reg, rhs.reg);
    }

    inline Vec4f Vec4f::operator/(const Vec4f& rhs) const
    {
        assert(rhs.reg.m128_f32[0] != 0.f and rhs.reg.m128_f32[1] != 0.f and rhs.reg.m128_f32[2] != 0.f and rhs.reg.m128_f32[3] != 0.f);
        return _mm_div_ps(reg, rhs.reg);
    }

    inline Vec4f Vec4f::operator-() const
    {
        __m128 e = _mm_set_ss(-0.0f);
		return _mm_xor_ps(reg, _mm_shuffle_ps(e, e, 0x00));
    }

    template <std::floating_point T>
    inline Vec4f Vec4f::operator*(T scalar) const
    {
		return _mm_mul_ps(reg, _mm_set1_ps(static_cast<float>(scalar)));
    }
    
    template <std::floating_point T>
    inline Vec4f Vec4f::operator/(T scalar) const
    {
		assert(scalar != 0.f);
        return _mm_div_ps(reg, _mm_set1_ps(static_cast<float>(scalar)));
    }
    
    inline Vec4f& Vec4f::operator+=(const Vec4f& rhs)
    {
		reg = _mm_add_ps(reg, rhs.reg);
        return *this;
    }

    inline Vec4f& Vec4f::operator-=(const Vec4f& rhs)
    {
        reg = _mm_sub_ps(reg, rhs.reg);
        return *this;
    }

    inline Vec4f& Vec4f::operator*=(const Vec4f& rhs)
    {
        reg = _mm_mul_ps(reg, rhs.reg);
        return *this;
    }

    inline Vec4f& Vec4f::operator/=(const Vec4f& rhs)
    {
        assert(rhs.reg.m128_f32[0] != 0.f and rhs.reg.m128_f32[1] != 0.f and rhs.reg.m128_f32[2] != 0.f and rhs.reg.m128_f32[3] != 0.f);
        reg = _mm_div_ps(reg, rhs.reg);
        return *this;
    }

    template <std::floating_point T>
    inline Vec4f& Vec4f::operator*=(T scalar)
    {
        reg = _mm_mul_ps(reg, _mm_set1_ps(static_cast<float>(scalar)));
        return *this;
    }

    template <std::floating_point T>
    inline Vec4f& Vec4f::operator/=(T scalar)
    {
        assert(scalar != 0.f);
        reg = _mm_div_ps(reg, _mm_set1_ps(static_cast<float>(scalar)));
        return *this;
    }

    inline bool Vec4f::operator==(const Vec4f& rhs) const
    {
        return ((_mm_movemask_ps(_mm_cmpeq_ps(reg, rhs.reg)) == 0xF));
        //return _mm_movemask_ps(_mm_cmpeq_ps(reg, rhs.reg));
    }

    inline bool Vec4f::operator!=(const Vec4f& rhs) const
    {
        return ((_mm_movemask_ps(_mm_cmpeq_ps(reg, rhs.reg)) != 0xF));
        //return !(_mm_movemask_ps(_mm_cmpeq_ps(reg, rhs.reg)));
    }
    
    float Vec4f::Dot(const Vec4f& rhs) const
    {

    }
    /*
    inline float Vec4f::MagnitudeSquared() const
    {
        
    }

    inline float Vec4f::Magnitude() const
    {
    }

    inline Vec4f Vec4f::Normalize() const
    {
    }

    inline float Vec4f::DistanceSquared(const Vec4f& rhs) const
    {
        
    }

    inline float Vec4f::Distance(const Vec4f& rhs) const
    {
        
    }

    inline float Vec4f::Angle(const Vec4f& rhs) const
    {
    }

    template <std::floating_point T>
    inline Vec4f Vec4f::Lerp(const Vec4f& a, const Vec4f& b, T t)
    {
    }

    inline Vec4f Vec4f::Min(const Vec4f& a, const Vec4f& b)
    {
    }

    inline Vec4f Vec4f::Max(const Vec4f& a, const Vec4f& b)
    {
    }
    template <std::floating_point T>
    inline Vec4f operator*(T scalar, const Vec4f& vector)
    {
        return vector * scalar;
    }*/

    const Vec4f Vec4f::Zero{0, 0, 0, 0};

    const Vec4f Vec4f::One{1, 1, 1, 1};

    const Vec4f Vec4f::UnitX{1, 0, 0, 0};

    const Vec4f Vec4f::UnitY{0, 1, 0, 0};

    const Vec4f Vec4f::UnitZ{0, 0, 1, 0};

    const Vec4f Vec4f::UnitW{0, 0, 0, 1};


}
