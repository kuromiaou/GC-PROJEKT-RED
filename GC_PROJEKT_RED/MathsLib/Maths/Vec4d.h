//#pragma once
//
//#include <immintrin.h>
//#include <type_traits>
//#include <algorithm>
//#include <cmath>
//#include <concepts>
//#include <iostream>
//#include <stdexcept>
//#include <cassert>
//
//namespace Maths
//{
//    class Vec4d
//    {
//    public:
//        __m128d regA;
//        __m128d regB;
//
//        Vec4d(double x, double y, double z, double w) : regA(_mm_setr_pd(x, y)), regB(_mm_setr_pd(z, w)) {}
//        
//        Vec4d() : regA(_mm_setzero_pd()), regB(_mm_setzero_pd()) {}
//
//        Vec4d(__m128d val) : regA(val.m128d_f64[0]), regB(val.m128d_f64[1]) {}
//
//        inline Vec4d operator+(const Vec4d& rhs) const;
//        inline Vec4d operator-(const Vec4d& rhs) const;
//        inline Vec4d operator*(const Vec4d& rhs) const;
//        inline Vec4d operator/(const Vec4d& rhs) const;
//        inline Vec4d operator-() const;
//
//        template <std::floating_point T= double >
//        Vec4d operator*(T scalar) const;
//        template <std::floating_point T = double>
//        Vec4d operator/(T scalar) const;
//        Vec4d operator+=(const Vec4d& rhs);
//        Vec4d operator-=(const Vec4d& rhs);
//        Vec4d operator*=(const Vec4d& rhs);
//        Vec4d operator/=(const Vec4d& rhs);
//        template <std::floating_point T = double>
//        Vec4d operator*=(T scalar);
//        template <std::floating_point T = double>
//        Vec4d operator/=(T scalar);
//        bool operator==(const Vec4d& rhs) const;
//        bool operator!=(const Vec4d& rhs) const;
//        /*double Dot(const Vec4d& rhs) const;
//        double MagnitudeSquared() const;
//        double Magnitude() const;
//
//        Vec4d Normalize() const;
//        double DistanceSquared(const Vec4d& rhs) const;
//        double Distance(const Vec4d& rhs) const;
//        double Angle(const Vec4d& rhs) const;
//
//        template <std::doubleing_point T = double>
//        static Vec4d Lerp(const Vec4d& a, const Vec4d& b, T t);
//        static Vec4d Min(const Vec4d& a, const Vec4d& b);
//        static Vec4d Max(const Vec4d& a, const Vec4d& b);
//
//        static const Vec4d Zero;
//        static const Vec4d One;
//        static const Vec4d UnitX;
//        static const Vec4d UnitY;
//        static const Vec4d UnitZ;
//        static const Vec4d UnitW;*/
//    };
//
//
//    inline Vec4d Vec4d::operator+(const Vec4d& rhs) const
//    {
//        //double res[4];
//        //_mm_store_ps(res,_mm_add_ps(reg, rhs.reg));
//        //Vec4d result = { res[0],res[1] ,res[2] ,res[3] };
//        //return result;
//    }
//
//    inline Vec4d Vec4d::operator-(const Vec4d& rhs) const
//    {
//    }
//
//    inline Vec4d Vec4d::operator*(const Vec4d& rhs) const
//    {
//    }
//
//    inline Vec4d Vec4d::operator/(const Vec4d& rhs) const
//    {
//
//    }
//
//    inline Vec4d Vec4d::operator-() const
//    {
//    }
//
//    template <std::floating_point T>
//    Vec4d Vec4d::operator*(T scalar) const
//    {
//    }
//
//    template <std::floating_point T>
//    Vec4d Vec4d::operator/(T scalar) const
//    {
//    }
//
//    Vec4d Vec4d::operator+=(const Vec4d& rhs)
//    {
//    }
//
//    Vec4d Vec4d::operator-=(const Vec4d& rhs)
//    {
//    }
//
//    Vec4d Vec4d::operator*=(const Vec4d& rhs)
//    {
//    }
//
//    Vec4d Vec4d::operator/=(const Vec4d& rhs)
//    {
//    }
//
//    template <std::floating_point T>
//    Vec4d Vec4d::operator*=(T scalar)
//    {
//    }
//
//    template <std::floating_point T>
//    Vec4d Vec4d::operator/=(T scalar)
//    {
//    }
//
//    bool Vec4d::operator==(const Vec4d& rhs) const
//    {
//    }
//
//    bool Vec4d::operator!=(const Vec4d& rhs) const
//    {
//    }
//    /*
//    double Vec4d::Dot(const Vec4d& rhs) const
//    {
//    }
//
//    double Vec4d::MagnitudeSquared() const
//    {
//        return Dot(*this);
//    }
//
//    double Vec4d::Magnitude() const
//    {
//    }
//
//    Vec4d Vec4d::Normalize() const
//    {
//    }
//
//    double Vec4d::DistanceSquared(const Vec4d& rhs) const
//    {
//        return (*this - rhs).MagnitudeSquared();
//    }
//
//    double Vec4d::Distance(const Vec4d& rhs) const
//    {
//        return (*this - rhs).Magnitude();
//    }
//
//    double Vec4d::Angle(const Vec4d& rhs) const
//    {
//    }
//
//    template <std::doubleing_point T>
//    Vec4d Vec4d::Lerp(const Vec4d& a, const Vec4d& b, T t)
//    {
//    }
//
//    Vec4d Vec4d::Min(const Vec4d& a, const Vec4d& b)
//    {
//    }
//
//    Vec4d Vec4d::Max(const Vec4d& a, const Vec4d& b)
//    {
//    }
//
//    const Vec4d Vec4d::Zero{0, 0, 0, 0};
//
//    const Vec4d Vec4d::One{1, 1, 1, 1};
//
//    const Vec4d Vec4d::UnitX{1, 0, 0, 0};
//
//    const Vec4d Vec4d::UnitY{0, 1, 0, 0};
//
//    const Vec4d Vec4d::UnitZ{0, 0, 1, 0};
//
//    const Vec4d Vec4d::UnitW{0, 0, 0, 1};
//
//    template <std::doubleing_point T>
//    Vec4d operator*(T scalar, const Vec4d& vector)
//    {
//        return vector * scalar;
//    }*/
//
//}











