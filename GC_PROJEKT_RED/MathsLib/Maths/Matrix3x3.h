#pragma once
#include <xmmintrin.h>
#include "Vec3f.h"
#include <array>
#include <limits>
#include <utility>

namespace Maths
{
    // Row-major storage, column vectors: result = matrix * vector.
    // A * B applies B first. Angles are radians, right-handed rotations.
    class Matrix3x3
    {

    public:
        __m128 row[3];

        Matrix3x3(); // Identity.
        explicit Matrix3x3(const std::array<float, 9>& elements);
        Matrix3x3(__m128 row0, __m128 row1, __m128 row2);


        static Matrix3x3 Identity();
        static Matrix3x3 Zero();
        Matrix3x3 operator+(const Matrix3x3& rhs) const;
        Matrix3x3 operator-(const Matrix3x3& rhs) const;
        Matrix3x3 operator*(const Matrix3x3& rhs) const;
        Maths::Vec3f operator*(const Maths::Vec3f& rhs) const;
        Matrix3x3 operator*(float scalar) const;
        Matrix3x3& operator*=(const Matrix3x3& rhs);
        bool operator==(const Matrix3x3& rhs) const;
        bool operator!=(const Matrix3x3& rhs) const;
        Matrix3x3 Transpose() const;
        float Determinant() const;
        //Gauss-Jordan with scaled partial pivoting. Rejects singular/ill-conditioned input.
        Matrix3x3 Inverse(float relativeTolerance = 64.0f * std::numeric_limits<float>::epsilon()) const;
        static Matrix3x3 Scale(const Maths::Vec3f& scale);
        static Matrix3x3 RotationX(float radians);
        static Matrix3x3 RotationY(float radians);
        static Matrix3x3 RotationZ(float radians);
    };

    inline Matrix3x3::Matrix3x3()
    {
        row[0] = _mm_setr_ps(1, 0, 0, 0); // x = 1;
        row[1] = _mm_setr_ps(0, 1, 0, 0); // y = 1;
        row[2] = _mm_setr_ps(0, 0, 1, 0); // z = 1;
    }

    inline Matrix3x3::Matrix3x3(const std::array<float, 9>& elements)
    {
        row[0] = _mm_setr_ps(elements[0], elements[1], elements[2], 0.0f);
        row[1] = _mm_setr_ps(elements[3], elements[4], elements[5], 0.0f);
        row[2] = _mm_setr_ps(elements[6], elements[7], elements[8], 0.0f);
    }

    inline Matrix3x3::Matrix3x3(__m128 row0, __m128 row1, __m128 row2) {
        row[0] = row0;
        row[1] = row1;
        row[2] = row2;
    }

    inline Matrix3x3 Matrix3x3::operator+(const Matrix3x3& rhs) const
    {
        Matrix3x3 result = Zero();

        result.row[0] = _mm_add_ps(row[0], rhs.row[0]);
        result.row[1] = _mm_add_ps(row[1], rhs.row[1]);
        result.row[2] = _mm_add_ps(row[2], rhs.row[2]);

        return result;
    }
    inline Matrix3x3 Matrix3x3::operator-(const Matrix3x3& rhs) const
    {
        Maths::Matrix3x3 result = Zero();

        result.row[0] = _mm_sub_ps(row[0], rhs.row[0]);
        result.row[1] = _mm_sub_ps(row[1], rhs.row[1]);
        result.row[2] = _mm_sub_ps(row[2], rhs.row[2]);

        return result;
    }

}