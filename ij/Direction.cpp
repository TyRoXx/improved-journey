#include "Direction.h"
#include "Unreachable.h"

ij::Vector2f ij::DirectionToVector(Direction direction)
{
    switch (direction)
    {
    case Direction::Up:
        return Vector2f(0, -1);
    case Direction::Left:
        return Vector2f(-1, 0);
    case Direction::Down:
        return Vector2f(0, 1);
    case Direction::Right:
        return Vector2f(1, 0);
    }
    IJ_UNREACHABLE();
}

ij::Direction ij::DirectionFromVector(const Vector2f &vector)
{
    if (vector.x >= 0)
    {
        if (vector.y >= vector.x)
        {
            return Direction::Down;
        }
        if (vector.y <= -vector.x)
        {
            return Direction::Up;
        }
        return Direction::Right;
    }
    if (vector.y >= -vector.x)
    {
        return Direction::Down;
    }
    if (vector.y <= vector.x)
    {
        return Direction::Up;
    }
    return Direction::Left;
}
