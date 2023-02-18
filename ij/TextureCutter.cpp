#include "TextureCutter.h"

sf::IntRect ij::CutWolfTexture(const ObjectAnimation animation, const Int32 animationTime, const Direction direction,
                               const Vector2i &size)
{
    Int32 x = 0;
    Int32 yOffset = 8;
    switch (animation)
    {
    case ObjectAnimation::Standing:
        break;

    case ObjectAnimation::Walking:
        x = (size.x * ((animationTime / 80) % 9));
        break;

    case ObjectAnimation::Attacking:
        yOffset = 0;
        x = (size.x * ((animationTime / 80) % 7));
        break;

    case ObjectAnimation::Dead:
        return sf::IntRect(5 * size.x, 20 * size.y, size.x, size.y);
    }
    return sf::IntRect(x, size.y * (AssertCast<int>(direction) + yOffset), size.x, size.y);
}
