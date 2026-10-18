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

        inline Vec3f operator*(float scalar) const;
        inline Vec3f operator/(float scalar) const;
        inline Vec3f& operator+=(const Vec3f& rhs);
        inline Vec3f& operator-=(const Vec3f& rhs);
        inline Vec3f& operator*=(const Vec3f& rhs);
        inline Vec3f& operator/=(const Vec3f& rhs);
        inline Vec3f& operator*=(float scalar);
        inline Vec3f& operator/=(float scalar);
        inline bool operator==(const Vec3f& rhs) const;
        inline bool operator!=(const Vec3f& rhs) const;
        
        inline float Dot(const Vec3f& rhs) const;
        inline float MagnitudeSquared() const;
        inline float Magnitude() const;

        inline Vec3f Normalize() const;
        inline float DistanceSquared(const Vec3f& rhs) const;
        inline float Distance(const Vec3f& rhs) const;
        inline float Angle(const Vec3f& rhs) const;

        //template <std::floating_point T = float>
        inline static Vec3f Lerp(const Vec3f& a, const Vec3f& b, float t);
        inline static Vec3f Min(const Vec3f& a, const Vec3f& b);
        inline static Vec3f Max(const Vec3f& a, const Vec3f& b);
        

        static const Vec3f Zero;
        static const Vec3f One;
        static const Vec3f UnitX;
        static const Vec3f UnitY;
        static const Vec3f UnitZ;
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
        const __m128 d = _mm_blend_ps(rhs.reg, _mm_set1_ps(1.f), 0x8);
        return _mm_div_ps(reg, d);
    }

    inline Vec3f Vec3f::operator-() const
    {
        //__m128 e = _mm_set_ss(-0.0f);
        //return _mm_xor_ps(reg, _mm_shuffle_ps(e, e, 0x00));
        return _mm_xor_ps(reg, _mm_set1_ps(-0.0f));
    }

    inline Vec3f Vec3f::operator*(float scalar) const
    {
        return _mm_mul_ps(reg, _mm_set1_ps(static_cast<float>(scalar)));
    }

    inline Vec3f Vec3f::operator/(float scalar) const
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

    inline Vec3f& Vec3f::operator*=(float scalar)
    {
        reg = _mm_mul_ps(reg, _mm_set1_ps(static_cast<float>(scalar)));
        return *this;
    }

    inline Vec3f& Vec3f::operator/=(float scalar)
    {
        assert(scalar != 0.f);
        reg = _mm_div_ps(reg, _mm_set1_ps(static_cast<float>(scalar)));
        return *this;
    }

    inline bool Vec3f::operator==(const Vec3f& rhs) const
    {
        return (_mm_movemask_ps(_mm_cmpeq_ps(reg, rhs.reg)) & 0x7) == 0x7;
    }

    inline bool Vec3f::operator!=(const Vec3f& rhs) const
    {
        return (_mm_movemask_ps(_mm_cmpeq_ps(reg, rhs.reg)) & 0x7) != 0x7;
    }
    
    inline float Vec3f::Dot(const Vec3f& rhs) const
    {
        return _mm_cvtss_f32(_mm_dp_ps(reg, rhs.reg, 0x71));
    }

    inline float Vec3f::MagnitudeSquared() const
    {
        return Dot(*this);
    }

    inline float Vec3f::Magnitude() const
    {
        return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(reg, reg, 0x71)));
    }

    inline Vec3f Vec3f::Normalize() const
    {
        __m128 lenSq = _mm_dp_ps(reg, reg, 0x7F);
        assert(_mm_cvtss_f32(lenSq) > 0.f);
        return _mm_div_ps(reg, _mm_sqrt_ps(lenSq));
    }

    inline float Vec3f::DistanceSquared(const Vec3f& rhs) const
    {
        return (*this - rhs).MagnitudeSquared();
    }

    inline float Vec3f::Distance(const Vec3f& rhs) const
    {
        return (*this - rhs).Magnitude();
    }

    inline float Vec3f::Angle(const Vec3f& rhs) const
    {
        const float denom = Magnitude() * rhs.Magnitude();
        assert(denom != 0.0f);
        const float cosi = Dot(rhs) / denom;
        return std::acos(std::clamp(cosi, -1.f, 1.f));
    }

    inline Vec3f Vec3f::Lerp(const Vec3f& a, const Vec3f& b, float t)
    {
        __m128 result = _mm_sub_ps(b.reg, a.reg);
        result = _mm_mul_ps(result, _mm_set1_ps(t));
        result = _mm_add_ps(a.reg, result);
        return result;
    }

    inline Vec3f Vec3f::Min(const Vec3f& a, const Vec3f& b)
    {
        return _mm_min_ps(a.reg, b.reg);
    }

    inline Vec3f Vec3f::Max(const Vec3f& a, const Vec3f& b)
    {
        return _mm_max_ps(a.reg, b.reg);
    }
    inline Vec3f operator*(float scalar, const Vec3f& vector)
    {
        return _mm_mul_ps(_mm_set1_ps(static_cast<float>(scalar)), vector.reg);
    }

    inline const Vec3f Vec3f::Zero{ 0, 0, 0 };

    inline const Vec3f Vec3f::One{ 1, 1, 1 };

    inline const Vec3f Vec3f::UnitX{ 1, 0, 0 };

    inline const Vec3f Vec3f::UnitY{ 0, 1, 0 };

    inline const Vec3f Vec3f::UnitZ{0, 0, 1};


}
