#pragma once
#include <SFML/System/Vector2.hpp>
#include <ij/Vector2.h>

namespace ij
{
    template <class T>
    Vector2<T> FromSfml(const sf::Vector2<T> &value) noexcept
    {
        return Vector2<T>(value.x, value.y);
    }
} // namespace ij
