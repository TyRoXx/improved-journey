#pragma once
#include "AssertCast.h"
#include "Int.h"

namespace ij
{
    template <class T>
    struct Vector2 final
    {
        T x, y;

        Vector2(T x, T y) noexcept
            : x(x)
            , y(y)
        {
        }
    };

    template <class T>
    Vector2<T> &operator+=(Vector2<T> &left, const Vector2<T> &right) noexcept
    {
        left.x += right.x;
        left.y += right.y;
        return left;
    }

    template <class T>
    Vector2<T> operator+(const Vector2<T> &left, const Vector2<T> &right) noexcept
    {
        return {left.x + right.x, left.y + right.y};
    }

    template <class T>
    Vector2<T> operator-(const Vector2<T> &left, const Vector2<T> &right) noexcept
    {
        return {left.x - right.x, left.y - right.y};
    }

    template <class T>
    Vector2<T> operator/(const Vector2<T> &left, const T right) noexcept
    {
        return {left.x / right, left.y / right};
    }

    template <class T>
    Vector2<T> operator*(const Vector2<T> &left, const T right) noexcept
    {
        return {left.x * right, left.y * right};
    }

    template <class To, class From>
    Vector2<To> AssertCastVector(const Vector2<From> &from)
    {
        return {AssertCast<To>(from.x), AssertCast<To>(from.y)};
    }

    using Vector2f = Vector2<float>;
    using Vector2i = Vector2<Int32>;
} // namespace ij
