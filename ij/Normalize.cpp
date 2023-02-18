#include "Normalize.h"
#include <cmath>

ij::Vector2f ij::normalize(const Vector2f &source)
{
    const float length = std::sqrt((source.x * source.x) + (source.y * source.y));
    if (length == 0)
    {
        return source;
    }
    return Vector2f(source.x / length, source.y / length);
}
