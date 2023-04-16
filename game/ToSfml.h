#pragma once
#include <SFML/System/Vector2.hpp>
#include <ij/Vector2.h>

namespace ij
{
    template <class T>
    sf::Vector2<T> ToSfml(const Vector2<T> &value) noexcept
    {
        return sf::Vector2<T>(value.x, value.y);
    }
} // namespace ij
