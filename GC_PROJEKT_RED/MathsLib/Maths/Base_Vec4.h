#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <stdexcept>

namespace Base_Maths
{
    // Scalar teaching baseline. No SIMD, custom alignment or forced inlining.
    template <std::floating_point T = float>
    class BVec4
    {
    public:
        T x;
        T y;
        T z;
        T w;

        BVec4();
        BVec4(T _x, T _y, T _z, T _w);
        template <std::floating_point U>
        explicit BVec4(const BVec4<U>& other);

        BVec4 operator+(const BVec4& rhs) const;
        BVec4 operator-(const BVec4& rhs) const;
        BVec4 operator*(const BVec4& rhs) const;
        BVec4 operator/(const BVec4& rhs) const;
        BVec4 operator-() const;
        BVec4 operator*(T scalar) const;
        BVec4 operator/(T scalar) const;
        BVec4& operator+=(const BVec4& rhs);
        BVec4& operator-=(const BVec4& rhs);
        BVec4& operator*=(const BVec4& rhs);
        BVec4& operator/=(const BVec4& rhs);
        BVec4& operator*=(T scalar);
        BVec4& operator/=(T scalar);
        bool operator==(const BVec4& rhs) const;
        bool operator!=(const BVec4& rhs) const;

        T Dot(const BVec4& rhs) const;
        T MagnitudeSquared() const;
        T Magnitude() const;
        // Returns a new vector. Throws domain_error for zero or non-finite input.
        BVec4 Normalize() const;
        T DistanceSquared(const BVec4& rhs) const;
        T Distance(const BVec4& rhs) const;
        T Angle(const BVec4& rhs) const;
        static BVec4 Lerp(const BVec4& a, const BVec4& b, T t);
        static BVec4 Min(const BVec4& a, const BVec4& b);
        static BVec4 Max(const BVec4& a, const BVec4& b);

        static const BVec4 Zero;
        static const BVec4 One;
        static const BVec4 UnitX;
        static const BVec4 UnitY;
        static const BVec4 UnitZ;
        static const BVec4 UnitW;
    };

    template <std::floating_point T>
    BVec4<T>::BVec4() : x(0), y(0), z(0), w(0)
    {

    }

    template <std::floating_point T>
    BVec4<T>::BVec4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w)
    {

    }

    template <std::floating_point T>
    template <std::floating_point U>
    BVec4<T>::BVec4(const BVec4<U>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)), z(static_cast<T>(other.z)), w(static_cast<T>(other.w))
    {

    }

    template <std::floating_point T>
    BVec4<T> BVec4<T>::operator+(const BVec4& rhs) const
    {
        return {x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w};
    }

    template <std::floating_point T>
    BVec4<T> BVec4<T>::operator-(const BVec4& rhs) const
    {
        return {x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w};
    }

    template <std::floating_point T>
    BVec4<T> BVec4<T>::operator*(const BVec4& rhs) const
    {
        return {x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w};
    }

    template <std::floating_point T>
    BVec4<T> BVec4<T>::operator/(const BVec4& rhs) const
    {
        if (rhs.x == T{0} || rhs.y == T{0} || rhs.z == T{0} || rhs.w == T{0})
        {
            throw std::domain_error("Cannot divide by a zero component");
        }

        return {x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w};
    }

    template <std::floating_point T>
    BVec4<T> BVec4<T>::operator-() const
    {
        return {-x, -y, -z, -w};
    }

    template <std::floating_point T>
    BVec4<T> BVec4<T>::operator*(T scalar) const
    {
        return {x * scalar, y * scalar, z * scalar, w * scalar};
    }

    template <std::floating_point T>
    BVec4<T> BVec4<T>::operator/(T scalar) const
    {
        if (scalar == T{0})
        {
            throw std::domain_error("Cannot divide by zero");
        }

        return {x / scalar, y / scalar, z / scalar, w / scalar};
    }

    template <std::floating_point T>
    BVec4<T>& BVec4<T>::operator+=(const BVec4& rhs)
    {
        *this = *this + rhs;
        return *this;
    }

    template <std::floating_point T>
    BVec4<T>& BVec4<T>::operator-=(const BVec4& rhs)
    {
        *this = *this - rhs;
        return *this;
    }

    template <std::floating_point T>
    BVec4<T>& BVec4<T>::operator*=(const BVec4& rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    template <std::floating_point T>
    BVec4<T>& BVec4<T>::operator/=(const BVec4& rhs)
    {
        *this = *this / rhs;
        return *this;
    }

    template <std::floating_point T>
    BVec4<T>& BVec4<T>::operator*=(T scalar)
    {
        *this = *this * scalar;
        return *this;
    }

    template <std::floating_point T>
    BVec4<T>& BVec4<T>::operator/=(T scalar)
    {
        *this = *this / scalar;
        return *this;
    }

    template <std::floating_point T>
    bool BVec4<T>::operator==(const BVec4& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    }

    template <std::floating_point T>
    bool BVec4<T>::operator!=(const BVec4& rhs) const
    {
        return !(*this == rhs);
    }

    template <std::floating_point T>
    T BVec4<T>::Dot(const BVec4& rhs) const
    {
        return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
    }

    template <std::floating_point T>
    T BVec4<T>::MagnitudeSquared() const
    {
        return Dot(*this);
    }

    template <std::floating_point T>
    T BVec4<T>::Magnitude() const
    {
        //return std::hypot(std::hypot(x, y), std::hypot(z, w));
        return std::sqrt(x * x + y * y + z * z + w * w);

    }

    template <std::floating_point T>
    BVec4<T> BVec4<T>::Normalize() const
    {
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) || !std::isfinite(w))
        {
            throw std::domain_error("Cannot normalize non-finite components");
        }

        // Scaling avoids overflowing/underflowing the squared magnitude.
        const T scale = std::max({std::abs(x), std::abs(y), std::abs(z), std::abs(w)});
        if (scale == T{0})
        {
            throw std::domain_error("Cannot normalize the zero vector");
        }

        const BVec4 scaled = *this / scale;
        return scaled / scaled.Magnitude();
    }

    template <std::floating_point T>
    T BVec4<T>::DistanceSquared(const BVec4& rhs) const
    {
        return (*this - rhs).MagnitudeSquared();
    }

    template <std::floating_point T>
    T BVec4<T>::Distance(const BVec4& rhs) const
    {
        return (*this - rhs).Magnitude();
    }

    template <std::floating_point T>
    T BVec4<T>::Angle(const BVec4& rhs) const
    {
        const T cosine = Normalize().Dot(rhs.Normalize());
        return std::acos(std::clamp(cosine, T{-1}, T{1}));
    }

    template <std::floating_point T>
    BVec4<T> BVec4<T>::Lerp(const BVec4& a, const BVec4& b, T t)
    {
        return a * (T{1} - t) + b * t;
    }

    template <std::floating_point T>
    BVec4<T> BVec4<T>::Min(const BVec4& a, const BVec4& b)
    {
        return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w)};
    }

    template <std::floating_point T>
    BVec4<T> BVec4<T>::Max(const BVec4& a, const BVec4& b)
    {
        return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w)};
    }

    template <std::floating_point T>
    const BVec4<T> BVec4<T>::Zero{0, 0, 0, 0};

    template <std::floating_point T>
    const BVec4<T> BVec4<T>::One{1, 1, 1, 1};

    template <std::floating_point T>
    const BVec4<T> BVec4<T>::UnitX{1, 0, 0, 0};

    template <std::floating_point T>
    const BVec4<T> BVec4<T>::UnitY{0, 1, 0, 0};

    template <std::floating_point T>
    const BVec4<T> BVec4<T>::UnitZ{0, 0, 1, 0};

    template <std::floating_point T>
    const BVec4<T> BVec4<T>::UnitW{0, 0, 0, 1};

    template <std::floating_point T>
    BVec4<T> operator*(T scalar, const BVec4<T>& vector)
    {
        return vector * scalar;
    }

    using BVec4f = BVec4<float>;
    using BVec4d = BVec4<double>;
}
