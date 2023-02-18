#pragma once
#include "Vector2.h"
#include <SFML/System/Vector2.hpp>

namespace ij
{
    template <class T>
    sf::Vector2<T> ToSfml(const Vector2<T> &value) noexcept
    {
        return sf::Vector2<T>(value.x, value.y);
    }
} // namespace ij
