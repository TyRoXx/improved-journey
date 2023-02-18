#pragma once
#include "Vector2.h"
#include <SFML/System/Vector2.hpp>

namespace ij
{
    template <class T>
    Vector2<T> FromSfml(const sf::Vector2<T> &value) noexcept
    {
        return Vector2<T>(value.x, value.y);
    }
} // namespace ij
