#pragma once
#include "Vector2.h"

namespace ij
{
    enum class Direction
    {
        Up,
        Left,
        Down,
        Right
    };

    Vector2f DirectionToVector(Direction direction);
    Direction DirectionFromVector(const Vector2f &vector);
} // namespace ij
