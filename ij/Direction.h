#pragma once
#include <SFML/System/Vector2.hpp>

namespace ij
{
    enum class Direction
    {
        Up,
        Left,
        Down,
        Right
    };

    sf::Vector2f DirectionToVector(Direction direction);
    Direction DirectionFromVector(const sf::Vector2f &vector);
} // namespace ij
