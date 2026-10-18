#pragma once

#include "Base_Vec4.h"
#include "Base_Vec3.h"
#include <array>
#include <limits>
#include <utility>

namespace Base_Maths
{
    // Row-major storage, column vectors: result = matrix * vector.
    // A * B applies B first. Angles are radians, right-handed rotations.
    template <std::floating_point T = float>
    class BMatrix4x4
    {
    public:
        T values[4][4];

        BMatrix4x4(); // Identity.
        explicit BMatrix4x4(const std::array<T, 16>& elements);
        static BMatrix4x4 Identity();
        static BMatrix4x4 Zero();
        T& operator()(std::size_t row, std::size_t column);
        const T& operator()(std::size_t row, std::size_t column) const;
        BMatrix4x4 operator+(const BMatrix4x4& rhs) const;
        BMatrix4x4 operator-(const BMatrix4x4& rhs) const;
        BMatrix4x4 operator*(const BMatrix4x4& rhs) const;
        BVec4<T> operator*(const BVec4<T>& rhs) const;
        BMatrix4x4 operator*(T scalar) const;
        BMatrix4x4& operator*=(const BMatrix4x4& rhs);
        bool operator==(const BMatrix4x4& rhs) const;
        bool operator!=(const BMatrix4x4& rhs) const;
        BMatrix4x4 Transpose() const;
        T Determinant() const;
        // Gauss-Jordan with scaled partial pivoting. Rejects singular/ill-conditioned input.
        BMatrix4x4 Inverse(T relativeTolerance = T{64} * std::numeric_limits<T>::epsilon()) const;
        static BMatrix4x4 Scale(const BVec3<T>& scale);
        static BMatrix4x4 RotationX(T radians);
        static BMatrix4x4 RotationY(T radians);
        static BMatrix4x4 RotationZ(T radians);
        static BMatrix4x4 Translation(const BVec3<T>& offset);
        BVec3<T> TransformPoint(const BVec3<T>& point) const; // Affine only, w = 1.
        BVec3<T> TransformDirection(const BVec3<T>& direction) const; // Affine only, w = 0.
    };

    template <std::floating_point T>
    BMatrix4x4<T>::BMatrix4x4() : values{}
    {
        for (std::size_t i = 0; i < 4; ++i)
        {
            values[i][i] = T{1};
        }
    }

    template <std::floating_point T>
    BMatrix4x4<T>::BMatrix4x4(const std::array<T, 16>& elements) : values{}
    {
        for (std::size_t row = 0; row < 4; ++row)
        {
            for (std::size_t column = 0; column < 4; ++column)
            {
                values[row][column] = elements[row * 4 + column];
            }
        }
    }

    template <std::floating_point T>
    BMatrix4x4<T> BMatrix4x4<T>::Identity()
    {
        return BMatrix4x4();
    }

    template <std::floating_point T>
    BMatrix4x4<T> BMatrix4x4<T>::Zero()
    {
        return BMatrix4x4(std::array<T, 16>{});
    }

    template <std::floating_point T>
    T& BMatrix4x4<T>::operator()(std::size_t row, std::size_t column)
    {
        if (row >= 4 || column >= 4)
        {
            throw std::out_of_range("Matrix index is out of range");
        }
        return values[row][column];
    }

    template <std::floating_point T>
    const T& BMatrix4x4<T>::operator()(std::size_t row, std::size_t column) const
    {
        if (row >= 4 || column >= 4)
        {
            throw std::out_of_range("Matrix index is out of range");
        }
        return values[row][column];
    }

    template <std::floating_point T>
    BMatrix4x4<T> BMatrix4x4<T>::operator+(const BMatrix4x4& rhs) const
    {
        BMatrix4x4 result = Zero();
        for (std::size_t row = 0; row < 4; ++row)
        {
            for (std::size_t column = 0; column < 4; ++column)
            {
                result.values[row][column] = values[row][column] + rhs.values[row][column];
            }
        }
        return result;
    }

    template <std::floating_point T>
    BMatrix4x4<T> BMatrix4x4<T>::operator-(const BMatrix4x4& rhs) const
    {
        BMatrix4x4 result = Zero();
        for (std::size_t row = 0; row < 4; ++row)
        {
            for (std::size_t column = 0; column < 4; ++column)
            {
                result.values[row][column] = values[row][column] - rhs.values[row][column];
            }
        }
        return result;
    }

    template <std::floating_point T>
    BMatrix4x4<T> BMatrix4x4<T>::operator*(const BMatrix4x4& rhs) const
    {
        BMatrix4x4 result = Zero();
        for (std::size_t row = 0; row < 4; ++row)
        {
            for (std::size_t column = 0; column < 4; ++column)
            {
                for (std::size_t k = 0; k < 4; ++k)
                {
                    result.values[row][column] += values[row][k] * rhs.values[k][column];
                }
            }
        }
        return result;
    }

    template <std::floating_point T>
    BVec4<T> BMatrix4x4<T>::operator*(const BVec4<T>& rhs) const
    {
        return {
            values[0][0] * rhs.x + values[0][1] * rhs.y + values[0][2] * rhs.z + values[0][3] * rhs.w,
            values[1][0] * rhs.x + values[1][1] * rhs.y + values[1][2] * rhs.z + values[1][3] * rhs.w,
            values[2][0] * rhs.x + values[2][1] * rhs.y + values[2][2] * rhs.z + values[2][3] * rhs.w,
            values[3][0] * rhs.x + values[3][1] * rhs.y + values[3][2] * rhs.z + values[3][3] * rhs.w
        };
    }

    template <std::floating_point T>
    BMatrix4x4<T> BMatrix4x4<T>::operator*(T scalar) const
    {
        BMatrix4x4 result = Zero();
        for (std::size_t row = 0; row < 4; ++row)
        {
            for (std::size_t column = 0; column < 4; ++column)
            {
                result.values[row][column] = values[row][column] * scalar;
            }
        }
        return result;
    }

    template <std::floating_point T>
    BMatrix4x4<T>& BMatrix4x4<T>::operator*=(const BMatrix4x4& rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    template <std::floating_point T>
    bool BMatrix4x4<T>::operator==(const BMatrix4x4& rhs) const
    {
        for (std::size_t row = 0; row < 4; ++row)
        {
            for (std::size_t column = 0; column < 4; ++column)
            {
                if (values[row][column] != rhs.values[row][column])
                {
                    return false;
                }
            }
        }
        return true;
    }

    template <std::floating_point T>
    bool BMatrix4x4<T>::operator!=(const BMatrix4x4& rhs) const
    {
        return !(*this == rhs);
    }

    template <std::floating_point T>
    BMatrix4x4<T> BMatrix4x4<T>::Transpose() const
    {
        BMatrix4x4 result = Zero();
        for (std::size_t row = 0; row < 4; ++row)
        {
            for (std::size_t column = 0; column < 4; ++column)
            {
                result.values[column][row] = values[row][column];
            }
        }
        return result;
    }

    template <std::floating_point T>
    T BMatrix4x4<T>::Determinant() const
    {
        BMatrix4x4 working = *this;
        T determinant = T{1};
        for (std::size_t column = 0; column < 4; ++column)
        {
            std::size_t pivot = column;
            for (std::size_t row = column + 1; row < 4; ++row)
            {
                if (std::abs(working.values[row][column]) > std::abs(working.values[pivot][column]))
                {
                    pivot = row;
                }
            }
            if (working.values[pivot][column] == T{0})
            {
                return T{0};
            }
            if (pivot != column)
            {
                for (std::size_t j = 0; j < 4; ++j)
                {
                    std::swap(working.values[pivot][j], working.values[column][j]);
                }
                determinant = -determinant;
            }
            const T diagonal = working.values[column][column];
            determinant *= diagonal;
            for (std::size_t row = column + 1; row < 4; ++row)
            {
                const T factor = working.values[row][column] / diagonal;
                for (std::size_t j = column + 1; j < 4; ++j)
                {
                    working.values[row][j] -= factor * working.values[column][j];
                }
            }
        }
        return determinant;
    }

    template <std::floating_point T>
    BMatrix4x4<T> BMatrix4x4<T>::Inverse(T relativeTolerance) const
    {
        if (!std::isfinite(relativeTolerance) || relativeTolerance < T{0} || relativeTolerance >= T{1})
        {
            throw std::invalid_argument("Inverse tolerance must be finite and in [0, 1)");
        }
        BMatrix4x4 left = *this;
        BMatrix4x4 right;
        T scales[4]{};
        for (std::size_t row = 0; row < 4; ++row)
        {
            for (std::size_t column = 0; column < 4; ++column)
            {
                if (!std::isfinite(left.values[row][column]))
                {
                    throw std::domain_error("Cannot invert a non-finite matrix");
                }
                scales[row] = std::max(scales[row], std::abs(left.values[row][column]));
            }
            if (scales[row] == T{0})
            {
                throw std::domain_error("Cannot invert a singular matrix");
            }
        }
        for (std::size_t column = 0; column < 4; ++column)
        {
            std::size_t pivot = column;
            for (std::size_t row = column + 1; row < 4; ++row)
            {
                if (std::abs(left.values[row][column]) / scales[row] >
                    std::abs(left.values[pivot][column]) / scales[pivot])
                {
                    pivot = row;
                }
            }
            if (std::abs(left.values[pivot][column]) / scales[pivot] <= relativeTolerance)
            {
                throw std::domain_error("Matrix is singular or too ill-conditioned");
            }
            for (std::size_t j = 0; j < 4; ++j)
            {
                std::swap(left.values[column][j], left.values[pivot][j]);
                std::swap(right.values[column][j], right.values[pivot][j]);
            }
            std::swap(scales[column], scales[pivot]);
            const T diagonal = left.values[column][column];
            for (std::size_t j = 0; j < 4; ++j)
            {
                left.values[column][j] /= diagonal;
                right.values[column][j] /= diagonal;
            }
            for (std::size_t row = 0; row < 4; ++row)
            {
                if (row == column)
                {
                    continue;
                }
                const T factor = left.values[row][column];
                for (std::size_t j = 0; j < 4; ++j)
                {
                    left.values[row][j] -= factor * left.values[column][j];
                    right.values[row][j] -= factor * right.values[column][j];
                }
            }
        }
        for (std::size_t row = 0; row < 4; ++row)
        {
            for (std::size_t column = 0; column < 4; ++column)
            {
                if (!std::isfinite(right.values[row][column]))
                {
                    throw std::domain_error("Inverse is not representable");
                }
            }
        }
        return right;
    }

    template <std::floating_point T>
    BMatrix4x4<T> BMatrix4x4<T>::Scale(const BVec3<T>& scale)
    {
        BMatrix4x4 result;
        result.values[0][0] = scale.x;
        result.values[1][1] = scale.y;
        result.values[2][2] = scale.z;
        return result;
    }

    template <std::floating_point T>
    BMatrix4x4<T> BMatrix4x4<T>::RotationX(T radians)
    {
        BMatrix4x4 result;
        const T cosine = std::cos(radians);
        const T sine = std::sin(radians);
        result.values[1][1] = cosine;
        result.values[2][2] = cosine;
        result.values[1][2] = -sine;
        result.values[2][1] = sine;
        return result;
    }

    template <std::floating_point T>
    BMatrix4x4<T> BMatrix4x4<T>::RotationY(T radians)
    {
        BMatrix4x4 result;
        const T cosine = std::cos(radians);
        const T sine = std::sin(radians);
        result.values[2][2] = cosine;
        result.values[0][0] = cosine;
        result.values[2][0] = -sine;
        result.values[0][2] = sine;
        return result;
    }

    template <std::floating_point T>
    BMatrix4x4<T> BMatrix4x4<T>::RotationZ(T radians)
    {
        BMatrix4x4 result;
        const T cosine = std::cos(radians);
        const T sine = std::sin(radians);
        result.values[0][0] = cosine;
        result.values[1][1] = cosine;
        result.values[0][1] = -sine;
        result.values[1][0] = sine;
        return result;
    }

    template <std::floating_point T>
    BMatrix4x4<T> BMatrix4x4<T>::Translation(const BVec3<T>& offset)
    {
        BMatrix4x4 result;
        result.values[0][3] = offset.x;
        result.values[1][3] = offset.y;
        result.values[2][3] = offset.z;
        return result;
    }

    template <std::floating_point T>
    BVec3<T> BMatrix4x4<T>::TransformPoint(const BVec3<T>& point) const
    {
        if (values[3][0] != T{0} || values[3][1] != T{0} ||
            values[3][2] != T{0} || values[3][3] != T{1})
        {
            throw std::domain_error("Transform requires an affine matrix");
        }
        const BVec4<T> result = *this * BVec4<T>(point.x, point.y, point.z, T{1});
        return {result.x, result.y, result.z};
    }

    template <std::floating_point T>
    BVec3<T> BMatrix4x4<T>::TransformDirection(const BVec3<T>& direction) const
    {
        if (values[3][0] != T{0} || values[3][1] != T{0} ||
            values[3][2] != T{0} || values[3][3] != T{1})
        {
            throw std::domain_error("Transform requires an affine matrix");
        }
        const BVec4<T> result = *this * BVec4<T>(direction.x, direction.y, direction.z, T{0});
        return {result.x, result.y, result.z};
    }

}
