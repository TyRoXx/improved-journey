#pragma once
#include "AssertCast.h"
#include "Direction.h"
#include "Int.h"
#include "ObjectAnimation.h"
#include "TimeSpan.h"
#include <SFML/Graphics/Rect.hpp>

namespace ij
{
    using TextureCutter = sf::IntRect(ObjectAnimation animation, TimeSpan animationTime, Direction direction,
                                      const Vector2u &size);

    template <Int32 WalkFrames, Int32 AttackFrames>
    sf::IntRect cutEnemyTexture(const ObjectAnimation animation, const TimeSpan animationTime,
                                const Direction direction, const Vector2u &size)
    {
        switch (animation)
        {
        case ObjectAnimation::Standing:
        case ObjectAnimation::Walking:
            break;
        case ObjectAnimation::Attacking:
            return sf::IntRect(
                AssertCast<Int32>(size.x * (((animationTime.Milliseconds / 200) % AttackFrames) + WalkFrames)),
                AssertCast<Int32>(size.y * AssertCast<Int32>(direction)), AssertCast<Int32>(size.x),
                AssertCast<Int32>(size.y));
        case ObjectAnimation::Dead:
            return sf::IntRect(0, AssertCast<Int32>(size.y * AssertCast<Int32>(direction)), AssertCast<Int32>(size.x),
                               AssertCast<Int32>(size.y));
        }
        return sf::IntRect(AssertCast<Int32>(size.x * ((animationTime.Milliseconds / 150) % WalkFrames)),
                           AssertCast<Int32>(size.y * AssertCast<Int32>(direction)), AssertCast<Int32>(size.x),
                           AssertCast<Int32>(size.y));
    }

    sf::IntRect CutWolfTexture(ObjectAnimation animation, TimeSpan animationTime, Direction direction,
                               const Vector2u &size);
} // namespace ij
