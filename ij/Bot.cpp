#include "Bot.h"

bool ij::isWithinDistance(const sf::Vector2f &first, const sf::Vector2f &second, const float distance)
{
    const float xDiff = (first.x - second.x);
    const float yDiff = (first.y - second.y);
    return (distance * distance) >= ((xDiff * xDiff) + (yDiff * yDiff));
}
