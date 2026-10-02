#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <stdexcept>

namespace Base_Maths
{
    // Scalar teaching baseline. No SIMD, custom alignment or forced inlining.
    template <std::floating_point T = float>
    class BVec3
    {
    public:
        T x;
        T y;
        T z;

        BVec3();
        BVec3(T _x, T _y, T _z);
        template <std::floating_point U>
        explicit BVec3(const BVec3<U>& other);

        BVec3 operator+(const BVec3& rhs) const;
        BVec3 operator-(const BVec3& rhs) const;
        BVec3 operator*(const BVec3& rhs) const;
        BVec3 operator/(const BVec3& rhs) const;
        BVec3 operator-() const;
        BVec3 operator*(T scalar) const;
        BVec3 operator/(T scalar) const;
        BVec3& operator+=(const BVec3& rhs);
        BVec3& operator-=(const BVec3& rhs);
        BVec3& operator*=(const BVec3& rhs);
        BVec3& operator/=(const BVec3& rhs);
        BVec3& operator*=(T scalar);
        BVec3& operator/=(T scalar);
        bool operator==(const BVec3& rhs) const;
        bool operator!=(const BVec3& rhs) const;

        T Dot(const BVec3& rhs) const;
        BVec3 Cross(const BVec3& rhs) const;
        T MagnitudeSquared() const;
        T Magnitude() const;
        // Returns a new vector. Throws domain_error for zero or non-finite input.
        BVec3 Normalize() const;
        T DistanceSquared(const BVec3& rhs) const;
        T Distance(const BVec3& rhs) const;
        T Angle(const BVec3& rhs) const;
        static BVec3 Lerp(const BVec3& a, const BVec3& b, T t);
        static BVec3 Min(const BVec3& a, const BVec3& b);
        static BVec3 Max(const BVec3& a, const BVec3& b);

        static const BVec3 Zero;
        static const BVec3 One;
        static const BVec3 UnitX;
        static const BVec3 UnitY;
        static const BVec3 UnitZ;
    };

    template <std::floating_point T>
    BVec3<T>::BVec3() : x(0), y(0), z(0)
    {

    }

    template <std::floating_point T>
    BVec3<T>::BVec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z)
    {

    }

    template <std::floating_point T>
    template <std::floating_point U>
    BVec3<T>::BVec3(const BVec3<U>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)), z(static_cast<T>(other.z))
    {

    }

    template <std::floating_point T>
    BVec3<T> BVec3<T>::operator+(const BVec3& rhs) const
    {
        return {x + rhs.x, y + rhs.y, z + rhs.z};
    }

    template <std::floating_point T>
    BVec3<T> BVec3<T>::operator-(const BVec3& rhs) const
    {
        return {x - rhs.x, y - rhs.y, z - rhs.z};
    }

    template <std::floating_point T>
    BVec3<T> BVec3<T>::operator*(const BVec3& rhs) const
    {
        return {x * rhs.x, y * rhs.y, z * rhs.z};
    }

    template <std::floating_point T>
    BVec3<T> BVec3<T>::operator/(const BVec3& rhs) const
    {
        if (rhs.x == T{0} || rhs.y == T{0} || rhs.z == T{0})
        {
            throw std::domain_error("Cannot divide by a zero component");
        }

        return {x / rhs.x, y / rhs.y, z / rhs.z};
    }

    template <std::floating_point T>
    BVec3<T> BVec3<T>::operator-() const
    {
        return {-x, -y, -z};
    }

    template <std::floating_point T>
    BVec3<T> BVec3<T>::operator*(T scalar) const
    {
        return {x * scalar, y * scalar, z * scalar};
    }

    template <std::floating_point T>
    BVec3<T> BVec3<T>::operator/(T scalar) const
    {
        if (scalar == T{0})
        {
            throw std::domain_error("Cannot divide by zero");
        }

        return {x / scalar, y / scalar, z / scalar};
    }

    template <std::floating_point T>
    BVec3<T>& BVec3<T>::operator+=(const BVec3& rhs)
    {
        *this = *this + rhs;
        return *this;
    }

    template <std::floating_point T>
    BVec3<T>& BVec3<T>::operator-=(const BVec3& rhs)
    {
        *this = *this - rhs;
        return *this;
    }

    template <std::floating_point T>
    BVec3<T>& BVec3<T>::operator*=(const BVec3& rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    template <std::floating_point T>
    BVec3<T>& BVec3<T>::operator/=(const BVec3& rhs)
    {
        *this = *this / rhs;
        return *this;
    }

    template <std::floating_point T>
    BVec3<T>& BVec3<T>::operator*=(T scalar)
    {
        *this = *this * scalar;
        return *this;
    }

    template <std::floating_point T>
    BVec3<T>& BVec3<T>::operator/=(T scalar)
    {
        *this = *this / scalar;
        return *this;
    }

    template <std::floating_point T>
    bool BVec3<T>::operator==(const BVec3& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    template <std::floating_point T>
    bool BVec3<T>::operator!=(const BVec3& rhs) const
    {
        return !(*this == rhs);
    }

    template <std::floating_point T>
    T BVec3<T>::Dot(const BVec3& rhs) const
    {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    template <std::floating_point T>
    BVec3<T> BVec3<T>::Cross(const BVec3& rhs) const
    {
        return {y * rhs.z - z * rhs.y,
                z * rhs.x - x * rhs.z,
                x * rhs.y - y * rhs.x};
    }

    template <std::floating_point T>
    T BVec3<T>::MagnitudeSquared() const
    {
        return Dot(*this);
    }

    template <std::floating_point T>
    T BVec3<T>::Magnitude() const
    {
        return std::hypot(x, y, z);
    }

    template <std::floating_point T>
    BVec3<T> BVec3<T>::Normalize() const
    {
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
        {
            throw std::domain_error("Cannot normalize non-finite components");
        }

        // Scaling avoids overflowing/underflowing the squared magnitude.
        const T scale = std::max({std::abs(x), std::abs(y), std::abs(z)});
        if (scale == T{0})
        {
            throw std::domain_error("Cannot normalize the zero vector");
        }

        const BVec3 scaled = *this / scale;
        return scaled / scaled.Magnitude();
    }

    template <std::floating_point T>
    T BVec3<T>::DistanceSquared(const BVec3& rhs) const
    {
        return (*this - rhs).MagnitudeSquared();
    }

    template <std::floating_point T>
    T BVec3<T>::Distance(const BVec3& rhs) const
    {
        return (*this - rhs).Magnitude();
    }

    template <std::floating_point T>
    T BVec3<T>::Angle(const BVec3& rhs) const
    {
        const T cosine = Normalize().Dot(rhs.Normalize());
        return std::acos(std::clamp(cosine, T{-1}, T{1}));
    }

    template <std::floating_point T>
    BVec3<T> BVec3<T>::Lerp(const BVec3& a, const BVec3& b, T t)
    {
        return a * (T{1} - t) + b * t;
    }

    template <std::floating_point T>
    BVec3<T> BVec3<T>::Min(const BVec3& a, const BVec3& b)
    {
        return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
    }

    template <std::floating_point T>
    BVec3<T> BVec3<T>::Max(const BVec3& a, const BVec3& b)
    {
        return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
    }

    template <std::floating_point T>
    const BVec3<T> BVec3<T>::Zero{0, 0, 0};

    template <std::floating_point T>
    const BVec3<T> BVec3<T>::One{1, 1, 1};

    template <std::floating_point T>
    const BVec3<T> BVec3<T>::UnitX{1, 0, 0};

    template <std::floating_point T>
    const BVec3<T> BVec3<T>::UnitY{0, 1, 0};

    template <std::floating_point T>
    const BVec3<T> BVec3<T>::UnitZ{0, 0, 1};

    template <std::floating_point T>
    BVec3<T> operator*(T scalar, const BVec3<T>& vector)
    {
        return vector * scalar;
    }

    using BVec3f = BVec3<float>;
    using BVec3d = BVec3<double>;
}
