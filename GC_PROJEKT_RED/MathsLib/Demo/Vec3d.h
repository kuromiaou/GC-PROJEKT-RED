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
    class Vec3d
    {
    public:
        __m128d regA;
        __m128d regB;

        Vec3d(double x, double y, double z) : regA(_mm_setr_pd(x, y)), regB(_mm_setr_pd(z, 0.0)) {}

        Vec3d() : regA(_mm_setzero_pd()), regB(_mm_setzero_pd()) {}

        Vec3d(__m128d val) : regA(val.m128d_f64[0]), regB(val.m128d_f64[1]) {}

        inline Vec3d operator+(const Vec3d& rhs) const;
        inline Vec3d operator-(const Vec3d& rhs) const;
        inline Vec3d operator*(const Vec3d& rhs) const;
        inline Vec3d operator/(const Vec3d& rhs) const;
        inline Vec3d operator-() const;

        template <std::floating_point T = double >
        Vec3d operator*(T scalar) const;
        template <std::floating_point T = double>
        Vec3d operator/(T scalar) const;
        Vec3d operator+=(const Vec3d& rhs);
        Vec3d operator-=(const Vec3d& rhs);
        Vec3d operator*=(const Vec3d& rhs);
        Vec3d operator/=(const Vec3d& rhs);
        template <std::floating_point T = double>
        Vec3d operator*=(T scalar);
        template <std::floating_point T = double>
        Vec3d operator/=(T scalar);
        bool operator==(const Vec3d& rhs) const;
        bool operator!=(const Vec3d& rhs) const;
        /*double Dot(const Vec3d& rhs) const;
        double MagnitudeSquared() const;
        double Magnitude() const;

        Vec3d Normalize() const;
        double DistanceSquared(const Vec3d& rhs) const;
        double Distance(const Vec3d& rhs) const;
        double Angle(const Vec3d& rhs) const;

        template <std::doubleing_point T = double>
        static Vec3d Lerp(const Vec3d& a, const Vec3d& b, T t);
        static Vec3d Min(const Vec3d& a, const Vec3d& b);
        static Vec3d Max(const Vec3d& a, const Vec3d& b);

        static const Vec3d Zero;
        static const Vec3d One;
        static const Vec3d UnitX;
        static const Vec3d UnitY;
        static const Vec3d UnitZ;
        static const Vec3d UnitW;*/
    };


    inline Vec3d Vec3d::operator+(const Vec3d& rhs) const
    {
        //double res[4];
        //_mm_store_ps(res,_mm_add_ps(reg, rhs.reg));
        //Vec3d result = { res[0],res[1] ,res[2] ,res[3] };
        //return result;
    }

    inline Vec3d Vec3d::operator-(const Vec3d& rhs) const
    {}

    inline Vec3d Vec3d::operator*(const Vec3d& rhs) const
    {}

    inline Vec3d Vec3d::operator/(const Vec3d& rhs) const
    {

    }

    inline Vec3d Vec3d::operator-() const
    {}

    template <std::floating_point T>
    Vec3d Vec3d::operator*(T scalar) const
    {}

    template <std::floating_point T>
    Vec3d Vec3d::operator/(T scalar) const
    {}

    Vec3d Vec3d::operator+=(const Vec3d& rhs)
    {}

    Vec3d Vec3d::operator-=(const Vec3d& rhs)
    {}

    Vec3d Vec3d::operator*=(const Vec3d& rhs)
    {}

    Vec3d Vec3d::operator/=(const Vec3d& rhs)
    {}

    template <std::floating_point T>
    Vec3d Vec3d::operator*=(T scalar)
    {}

    template <std::floating_point T>
    Vec3d Vec3d::operator/=(T scalar)
    {}

    bool Vec3d::operator==(const Vec3d& rhs) const
    {}

    bool Vec3d::operator!=(const Vec3d& rhs) const
    {}
    /*
    double Vec3d::Dot(const Vec3d& rhs) const
    {
    }

    double Vec3d::MagnitudeSquared() const
    {
        return Dot(*this);
    }

    double Vec3d::Magnitude() const
    {
    }

    Vec3d Vec3d::Normalize() const
    {
    }

    double Vec3d::DistanceSquared(const Vec3d& rhs) const
    {
        return (*this - rhs).MagnitudeSquared();
    }

    double Vec3d::Distance(const Vec3d& rhs) const
    {
        return (*this - rhs).Magnitude();
    }

    double Vec3d::Angle(const Vec3d& rhs) const
    {
    }

    template <std::doubleing_point T>
    Vec3d Vec3d::Lerp(const Vec3d& a, const Vec3d& b, T t)
    {
    }

    Vec3d Vec3d::Min(const Vec3d& a, const Vec3d& b)
    {
    }

    Vec3d Vec3d::Max(const Vec3d& a, const Vec3d& b)
    {
    }

    const Vec3d Vec3d::Zero{0, 0, 0, 0};

    const Vec3d Vec3d::One{1, 1, 1, 1};

    const Vec3d Vec3d::UnitX{1, 0, 0, 0};

    const Vec3d Vec3d::UnitY{0, 1, 0, 0};

    const Vec3d Vec3d::UnitZ{0, 0, 1, 0};

    const Vec3d Vec3d::UnitW{0, 0, 0, 1};

    template <std::doubleing_point T>
    Vec3d operator*(T scalar, const Vec3d& vector)
    {
        return vector * scalar;
    }*/

}
