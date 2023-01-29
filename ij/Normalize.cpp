#include "Normalize.h"
#include <cmath>

sf::Vector2f ij::normalize(const sf::Vector2f &source)
{
    const float length = std::sqrt((source.x * source.x) + (source.y * source.y));
    if (length == 0)
    {
        return source;
    }
    return sf::Vector2f(source.x / length, source.y / length);
}
