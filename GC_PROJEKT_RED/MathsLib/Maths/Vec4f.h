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
        ///Division by zero yields inf/NaN, asserted in Debug.
        ///Note : the assert only catches exact 0, not 8.1e-22f, for example.
        inline Vec4f operator/(const Vec4f& rhs) const;
		/// Turns all factors negative. If they are already negative, they stay negative.
        inline Vec4f operator-() const;

        template <std::floating_point T = float>
        inline Vec4f operator*(T scalar) const;
        /// Same as operator/: division by zero yields inf/NaN, asserted in Debug
        template <std::floating_point T = float>
        inline Vec4f operator/(T scalar) const;
        inline Vec4f& operator+=(const Vec4f& rhs);
        inline Vec4f& operator-=(const Vec4f& rhs);
        inline Vec4f& operator*=(const Vec4f& rhs);
        /// Same as operator/: division by zero yields inf/NaN, asserted in Debug
        inline Vec4f& operator/=(const Vec4f& rhs);
        template <std::floating_point T = float>
        inline Vec4f& operator*=(T scalar);
        /// Same as operator/: division by zero yields inf/NaN, asserted in Debug
        template <std::floating_point T = float>
        inline Vec4f& operator/=(T scalar);
        inline bool operator==(const Vec4f& rhs) const;
        inline bool operator!=(const Vec4f& rhs) const;
		/// Left usable for benchmarking, but not needed in the final implementation.
        ///|              ns / op |                       op / s |      err % |        total | benchmark
        ///|                  2.11 |       473, 241, 945.37 |      0.8 % |         2.19 | `DotDP`
        ///|                  2.28 |         438,937,720.35 |       0.3% |         2.20 | `DotManual`
        inline float DotDP(const Vec4f& rhs) const;
        /// Left usable for benchmarking, but not needed in the final implementation.
        ///|              ns / op |                       op / s |      err % |        total | benchmark
        ///|                  2.11 |       473, 241, 945.37 |      0.8 % |         2.19 | `DotDP`
        ///|                  2.28 |         438,937,720.35 |       0.3% |         2.20 | `DotManual`
        inline float DotManual(const Vec4f& rhs) const;
		/// Uses SSE4.1 _mm_dp_ps instruction (same as DotDP). 
        /// We chose to keep _mm_dp_ps as the main Dot product implementation, as it was ~8% faster in our benchmarks.
        ///   Benchmark     ns/op
        ///   DotDP             2.11 
        ///   DotManual     2.28
        inline float Dot(const Vec4f& rhs) const;
        inline float MagnitudeSquared() const;
		/// SIMD Magnitude is ~70% / 3.3x faster than Base_Magnitude (with hypot) in our benchmarks.
		/// SIMD Magnitude is ~15% / 1.18x faster than Base_Magnitude (without hypot) in our benchmarks.
        ///   Benchmark            ns/op
        ///   SIMD Magnitude       2.09
        ///   Scalar, std::sqrt()        2.47 ~15% / 1.18x slower
		///   Scalar, std::hypot()     6.99 ~70% / 3.3x slower
        inline float Magnitude() const;
        
        inline Vec4f Normalize() const;
        inline float DistanceSquared(const Vec4f& rhs) const;
        inline float Distance(const Vec4f& rhs) const;
        inline float Angle(const Vec4f& rhs) const;

        template <std::floating_point T = float>
        inline static Vec4f Lerp(const Vec4f& a, const Vec4f& b, T t);
        inline static Vec4f Min(const Vec4f& a, const Vec4f& b);
        inline static Vec4f Max(const Vec4f& a, const Vec4f& b);
        
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
        return _mm_add_ps(reg, rhs.reg);  //benchmark to operator to see the difference.
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
        //__m128 e = _mm_set_ss(-0.0f);
		//return _mm_xor_ps(reg, _mm_shuffle_ps(e, e, 0x00));
		return _mm_xor_ps(reg, _mm_set1_ps(-0.0f));     //benchmark to operator to see the difference.
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
        
    }

    inline bool Vec4f::operator!=(const Vec4f& rhs) const
    {
        return ((_mm_movemask_ps(_mm_cmpeq_ps(reg, rhs.reg)) != 0xF));
        
    }
    

    inline float Vec4f::DotDP(const Vec4f& rhs) const
    {
        return _mm_cvtss_f32(_mm_dp_ps(reg, rhs.reg, 0xF1));
    }

    inline float Vec4f::DotManual(const Vec4f& rhs) const
    {
        __m128 m = _mm_mul_ps(reg, rhs.reg);
        __m128 s = _mm_add_ps(m, _mm_movehl_ps(m, m));
        s = _mm_add_ss(s, _mm_shuffle_ps(s, s, 1));
        return _mm_cvtss_f32(s);
    }

    inline float Vec4f::MagnitudeSquared() const
    {
        return Dot(*this);
    }
  
    inline float Vec4f::Dot(const Vec4f& rhs) const
    {
        return _mm_cvtss_f32(_mm_dp_ps(reg, rhs.reg, 0xF1));
    }

    inline float Vec4f::Magnitude() const
    {
        return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(reg, reg, 0xF1)));
    }
    
    inline Vec4f Vec4f::Normalize() const
    {
        __m128 lenSq = _mm_dp_ps(reg, reg, 0xFF);
        assert(_mm_cvtss_f32(lenSq) > 0.f);
        return _mm_div_ps(reg, _mm_sqrt_ps(lenSq));
    }

    inline float Vec4f::DistanceSquared(const Vec4f& rhs) const
    {
        return (*this - rhs).MagnitudeSquared();
    }

    inline float Vec4f::Distance(const Vec4f& rhs) const
    {
        return (*this - rhs).Magnitude();
    }

    inline float Vec4f::Angle(const Vec4f& rhs) const
    {
        const float denom = Magnitude() * rhs.Magnitude();
        assert(denom != 0.0f);
		const float cosi = Dot(rhs) / denom;
        return std::acos(std::clamp(cosi, -1.f, 1.f));
    }

    template <std::floating_point T>
    inline Vec4f Vec4f::Lerp(const Vec4f& a, const Vec4f& b, T t)
    {
        __m128 result = _mm_sub_ps( b.reg, a.reg);
        result = _mm_mul_ps(result, _mm_set1_ps(static_cast<float>(t)));
        result = _mm_add_ps(a.reg, result);
        return result;
    }

    inline Vec4f Vec4f::Min(const Vec4f& a, const Vec4f& b)
    {
        return _mm_min_ps(a.reg, b.reg);
    }

    inline Vec4f Vec4f::Max(const Vec4f& a, const Vec4f& b)
    {
        return _mm_max_ps(a.reg, b.reg);
    }

    template <std::floating_point T>
    inline Vec4f operator*(T scalar, const Vec4f& vector)
    {
        return _mm_mul_ps(_mm_set1_ps(static_cast<float>(scalar)), vector.reg);
    }

    inline const Vec4f Vec4f::Zero{0, 0, 0, 0};

    inline const Vec4f Vec4f::One{1, 1, 1, 1};

    inline const Vec4f Vec4f::UnitX{1, 0, 0, 0};

    inline const Vec4f Vec4f::UnitY{0, 1, 0, 0};

    inline const Vec4f Vec4f::UnitZ{0, 0, 1, 0};

    inline const Vec4f Vec4f::UnitW{0, 0, 0, 1};


}
