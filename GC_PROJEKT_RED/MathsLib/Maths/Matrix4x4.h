#pragma once

#include "Vec4f.h"
#include "Vec3f.h"

#include <array>
#include <limits>
#include <utility>

namespace Maths
{
    // Row-major storage, column vectors: result = matrix * vector.
    // A * B applies B first. Angles are radians, right-handed rotations.
    //template <std::floating_point T = float>
    class Matrix4x4
    {
    public:
        /// <summary>
        /// Row-major
        /// </summary>
        __m128 row[4];

        Matrix4x4(); // Identity. 
        explicit Matrix4x4(const std::array<float, 16>& elements);
        static Matrix4x4 Identity();
        static Matrix4x4 Zero();
        //We decided to remove these 2 functions, as they would decrease the number or op / s way to much
        //float& operator()(std::size_t row, std::size_t column);                
        //const float& operator()(std::size_t row, std::size_t column) const;
        Matrix4x4 operator+(const Matrix4x4& rhs) const;
        Matrix4x4 operator-(const Matrix4x4& rhs) const;
        //Matrix4x4 operator*(const Matrix4x4& rhs) const;
        //Vec4f operator*(const Vec4f& rhs) const;
        Matrix4x4 operator*(float scalar) const;
        //Matrix4x4& operator*=(const Matrix4x4& rhs);
        //bool operator==(const Matrix4x4& rhs) const;
        //bool operator!=(const Matrix4x4& rhs) const;
        //Matrix4x4 Transpose() const;
        //T Determinant() const;
        //// Gauss-Jordan with scaled partial pivoting. Rejects singular/ill-conditioned input.
        //Matrix4x4 Inverse(T relativeTolerance = T{64} * std::numeric_limits<T>::epsilon()) const;
        //static Matrix4x4 Scale(const Vec3f& scale);
        //static Matrix4x4 RotationX(T radians);
        //static Matrix4x4 RotationY(T radians);
        //static Matrix4x4 RotationZ(T radians);
        //static Matrix4x4 Translation(const Vec3f& offset);
        //Vec3f TransformPoint(const Vec3f& point) const; // Affine only, w = 1.
        //Vec3f TransformDirection(const Vec3f& direction) const; // Affine only, w = 0.
    };

    Matrix4x4::Matrix4x4() : row()
    {
        row[0] = _mm_setr_ps(1,0,0,0); // w = 1;
        row[1] = _mm_setr_ps(0,1,0,0); // z = 1;
        row[2] = _mm_setr_ps(0,0,1,0); // y = 1;
        row[3] = _mm_setr_ps(0,0,0,1); // x = 1;

    }

    Matrix4x4::Matrix4x4(const std::array<float, 16>& elements) : row()
    {
        row[0] = _mm_setr_ps(elements[0], elements[1], elements[2], elements[3]);
        row[1] = _mm_setr_ps(elements[4], elements[5], elements[6], elements[7]);
        row[2] = _mm_setr_ps(elements[8], elements[9], elements[10], elements[11]);
        row[3] = _mm_setr_ps(elements[12], elements[13], elements[14], elements[15]);
    }

    Matrix4x4 Matrix4x4::Identity()
    {
        return Matrix4x4();
    }

    Matrix4x4 Matrix4x4::Zero()
    {
        return Matrix4x4(std::array<float, 16>{});
    }

    Matrix4x4 Matrix4x4::operator+(const Matrix4x4& rhs) const
    {
        Matrix4x4 result = Zero();
        result.row[0] = _mm_add_ps(row[0], rhs.row[0]);
        result.row[1] = _mm_add_ps(row[1], rhs.row[1]);
        result.row[2] = _mm_add_ps(row[2], rhs.row[2]);
        result.row[3] = _mm_add_ps(row[3], rhs.row[3]);

        return result;
    }

    Matrix4x4 Matrix4x4::operator-(const Matrix4x4& rhs) const
    {
        Matrix4x4 result = Zero();
        result.row[0] = _mm_sub_ps(row[0], rhs.row[0]);
        result.row[1] = _mm_sub_ps(row[1], rhs.row[1]);
        result.row[2] = _mm_sub_ps(row[2], rhs.row[2]);
        result.row[3] = _mm_sub_ps(row[3], rhs.row[3]);

        return result;
    }

    //template <std::floating_point T>
    //Matrix4x4<T> Matrix4x4<T>::operator*(const Matrix4x4& rhs) const
    //{
    //    Matrix4x4 result = Zero();
    //    for (std::size_t row = 0; row < 4; ++row)
    //    {
    //        for (std::size_t column = 0; column < 4; ++column)
    //        {
    //            for (std::size_t k = 0; k < 4; ++k)
    //            {
    //                result.values[row][column] += values[row][k] * rhs.values[k][column];
    //            }
    //        }
    //    }
    //    return result;
    //}

    //template <std::floating_point T>
    //Vec4f Matrix4x4<T>::operator*(const Vec4f& rhs) const
    //{
    //    return {
    //    };
    //}

    Matrix4x4 Matrix4x4::operator*(float scalar) const
    {
        Matrix4x4 result = Zero();

        result.row[0] = _mm_mul_ps(row[0], _mm_set1_ps(static_cast<float>(scalar)));
        result.row[1] = _mm_mul_ps(row[1], _mm_set1_ps(static_cast<float>(scalar)));
        result.row[2] = _mm_mul_ps(row[2], _mm_set1_ps(static_cast<float>(scalar)));
        result.row[3] = _mm_mul_ps(row[3], _mm_set1_ps(static_cast<float>(scalar)));

        return result;
    }

    //template <std::floating_point T>
    //Matrix4x4<T>& Matrix4x4<T>::operator*=(const Matrix4x4& rhs)
    //{
    //    *this = *this * rhs;
    //    return *this;
    //}

    //template <std::floating_point T>
    //bool Matrix4x4<T>::operator==(const Matrix4x4& rhs) const
    //{
    //    for (std::size_t row = 0; row < 4; ++row)
    //    {
    //        for (std::size_t column = 0; column < 4; ++column)
    //        {
    //            if (values[row][column] != rhs.values[row][column])
    //            {
    //                return false;
    //            }
    //        }
    //    }
    //    return true;
    //}

    //template <std::floating_point T>
    //bool Matrix4x4<T>::operator!=(const Matrix4x4& rhs) const
    //{
    //    return !(*this == rhs);
    //}

    //template <std::floating_point T>
    //Matrix4x4<T> Matrix4x4<T>::Transpose() const
    //{
    //    Matrix4x4 result = Zero();
    //    for (std::size_t row = 0; row < 4; ++row)
    //    {
    //        for (std::size_t column = 0; column < 4; ++column)
    //        {
    //            result.values[column][row] = values[row][column];
    //        }
    //    }
    //    return result;
    //}

    //template <std::floating_point T>
    //T Matrix4x4<T>::Determinant() const
    //{
    //    Matrix4x4 working = *this;
    //    T determinant = T{1};
    //    for (std::size_t column = 0; column < 4; ++column)
    //    {
    //        std::size_t pivot = column;
    //        for (std::size_t row = column + 1; row < 4; ++row)
    //        {
    //            if (std::abs(working.values[row][column]) > std::abs(working.values[pivot][column]))
    //            {
    //                pivot = row;
    //            }
    //        }
    //        if (working.values[pivot][column] == T{0})
    //        {
    //            return T{0};
    //        }
    //        if (pivot != column)
    //        {
    //            for (std::size_t j = 0; j < 4; ++j)
    //            {
    //                std::swap(working.values[pivot][j], working.values[column][j]);
    //            }
    //            determinant = -determinant;
    //        }
    //        const T diagonal = working.values[column][column];
    //        determinant *= diagonal;
    //        for (std::size_t row = column + 1; row < 4; ++row)
    //        {
    //            const T factor = working.values[row][column] / diagonal;
    //            for (std::size_t j = column + 1; j < 4; ++j)
    //            {
    //                working.values[row][j] -= factor * working.values[column][j];
    //            }
    //        }
    //    }
    //    return determinant;
    //}

    //template <std::floating_point T>
    //Matrix4x4<T> Matrix4x4<T>::Inverse(T relativeTolerance) const
    //{
    //    if (!std::isfinite(relativeTolerance) || relativeTolerance < T{0} || relativeTolerance >= T{1})
    //    {
    //        throw std::invalid_argument("Inverse tolerance must be finite and in [0, 1)");
    //    }
    //    Matrix4x4 left = *this;
    //    Matrix4x4 right;
    //    T scales[4]{};
    //    for (std::size_t row = 0; row < 4; ++row)
    //    {
    //        for (std::size_t column = 0; column < 4; ++column)
    //        {
    //            if (!std::isfinite(left.values[row][column]))
    //            {
    //                throw std::domain_error("Cannot invert a non-finite matrix");
    //            }
    //            scales[row] = std::max(scales[row], std::abs(left.values[row][column]));
    //        }
    //        if (scales[row] == T{0})
    //        {
    //            throw std::domain_error("Cannot invert a singular matrix");
    //        }
    //    }
    //    for (std::size_t column = 0; column < 4; ++column)
    //    {
    //        std::size_t pivot = column;
    //        for (std::size_t row = column + 1; row < 4; ++row)
    //        {
    //            if (std::abs(left.values[row][column]) / scales[row] >
    //                std::abs(left.values[pivot][column]) / scales[pivot])
    //            {
    //                pivot = row;
    //            }
    //        }
    //        if (std::abs(left.values[pivot][column]) / scales[pivot] <= relativeTolerance)
    //        {
    //            throw std::domain_error("Matrix is singular or too ill-conditioned");
    //        }
    //        for (std::size_t j = 0; j < 4; ++j)
    //        {
    //            std::swap(left.values[column][j], left.values[pivot][j]);
    //            std::swap(right.values[column][j], right.values[pivot][j]);
    //        }
    //        std::swap(scales[column], scales[pivot]);
    //        const T diagonal = left.values[column][column];
    //        for (std::size_t j = 0; j < 4; ++j)
    //        {
    //            left.values[column][j] /= diagonal;
    //            right.values[column][j] /= diagonal;
    //        }
    //        for (std::size_t row = 0; row < 4; ++row)
    //        {
    //            if (row == column)
    //            {
    //                continue;
    //            }
    //            const T factor = left.values[row][column];
    //            for (std::size_t j = 0; j < 4; ++j)
    //            {
    //                left.values[row][j] -= factor * left.values[column][j];
    //                right.values[row][j] -= factor * right.values[column][j];
    //            }
    //        }
    //    }
    //    for (std::size_t row = 0; row < 4; ++row)
    //    {
    //        for (std::size_t column = 0; column < 4; ++column)
    //        {
    //            if (!std::isfinite(right.values[row][column]))
    //            {
    //                throw std::domain_error("Inverse is not representable");
    //            }
    //        }
    //    }
    //    return right;
    //}

    //template <std::floating_point T>
    //Matrix4x4<T> Matrix4x4<T>::Scale(const Vec3f& scale)
    //{
    //    Matrix4x4 result;
    //    result.values[0][0] = scale.x;
    //    result.values[1][1] = scale.y;
    //    result.values[2][2] = scale.z;
    //    return result;
    //}

    //template <std::floating_point T>
    //Matrix4x4<T> Matrix4x4<T>::RotationX(T radians)
    //{
    //    Matrix4x4 result;
    //    const T cosine = std::cos(radians);
    //    const T sine = std::sin(radians);
    //    result.values[1][1] = cosine;
    //    result.values[2][2] = cosine;
    //    result.values[1][2] = -sine;
    //    result.values[2][1] = sine;
    //    return result;
    //}

    //template <std::floating_point T>
    //Matrix4x4<T> Matrix4x4<T>::RotationY(T radians)
    //{
    //    Matrix4x4 result;
    //    const T cosine = std::cos(radians);
    //    const T sine = std::sin(radians);
    //    result.values[2][2] = cosine;
    //    result.values[0][0] = cosine;
    //    result.values[2][0] = -sine;
    //    result.values[0][2] = sine;
    //    return result;
    //}

    //template <std::floating_point T>
    //Matrix4x4<T> Matrix4x4<T>::RotationZ(T radians)
    //{
    //    Matrix4x4 result;
    //    const T cosine = std::cos(radians);
    //    const T sine = std::sin(radians);
    //    result.values[0][0] = cosine;
    //    result.values[1][1] = cosine;
    //    result.values[0][1] = -sine;
    //    result.values[1][0] = sine;
    //    return result;
    //}

    //template <std::floating_point T>
    //Matrix4x4<T> Matrix4x4<T>::Translation(const Vec3f& offset)
    //{
    //    Matrix4x4 result;
    //    result.values[0][3] = offset.x;
    //    result.values[1][3] = offset.y;
    //    result.values[2][3] = offset.z;
    //    return result;
    //}

    //template <std::floating_point T>
    //Vec3f Matrix4x4<T>::TransformPoint(const Vec3f& point) const
    //{
    //    if (values[3][0] != T{0} || values[3][1] != T{0} ||
    //        values[3][2] != T{0} || values[3][3] != T{1})
    //    {
    //        throw std::domain_error("Transform requires an affine matrix");
    //    }
    //    const Vec4f result = *this * Vec4f(point.x, point.y, point.z, T{1});
    //    return {result.x, result.y, result.z};
    //}

    //template <std::floating_point T>
    //Vec3f Matrix4x4<T>::TransformDirection(const Vec3f& direction) const
    //{
    //    if (values[3][0] != T{0} || values[3][1] != T{0} ||
    //        values[3][2] != T{0} || values[3][3] != T{1})
    //    {
    //        throw std::domain_error("Transform requires an affine matrix");
    //    }
    //    const Vec4f result = *this * Vec4f(direction.x, direction.y, direction.z, T{0});
    //    return {result.x, result.y, result.z};
    //}

}
