#include "TextureCutter.h"

sf::IntRect ij::CutWolfTexture(const ObjectAnimation animation, const TimeSpan animationTime, const Direction direction,
                               const Vector2u &size)
{
    Int32 x = 0;
    Int32 yOffset = 8;
    switch (animation)
    {
    case ObjectAnimation::Standing:
        break;

    case ObjectAnimation::Walking:
        x = AssertCast<Int32>(size.x * ((animationTime.Milliseconds / 80) % 9));
        break;

    case ObjectAnimation::Attacking:
        yOffset = 0;
        x = AssertCast<Int32>(size.x * ((animationTime.Milliseconds / 80) % 7));
        break;

    case ObjectAnimation::Dead:
        return sf::IntRect(AssertCast<Int32>(5 * size.x), AssertCast<Int32>(20 * size.y), AssertCast<Int32>(size.x),
                           AssertCast<Int32>(size.y));
    }
    return sf::IntRect(x, AssertCast<Int32>(size.y * (AssertCast<Int32>(direction) + yOffset)),
                       AssertCast<Int32>(size.x), AssertCast<Int32>(size.y));
}
