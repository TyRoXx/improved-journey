#pragma once
#include "AssertCast.h"
#include "Direction.h"
#include "Int.h"
#include "ObjectAnimation.h"
#include "TimeSpan.h"
#include <SFML/Graphics/Rect.hpp>

namespace ij
{
    template <class P, class S>
    struct Rectangle final
    {
        Vector2<P> Position;
        Vector2<S> Size;
    };

    using TextureRectangle = Rectangle<UInt32, UInt32>;
    using TextureCutter = TextureRectangle(ObjectAnimation animation, TimeSpan animationTime, Direction direction,
                                           const Vector2u &size);

    template <Int32 WalkFrames, Int32 AttackFrames>
    TextureRectangle cutEnemyTexture(const ObjectAnimation animation, const TimeSpan animationTime,
                                     const Direction direction, const Vector2u &size)
    {
        switch (animation)
        {
        case ObjectAnimation::Standing:
        case ObjectAnimation::Walking:
            break;
        case ObjectAnimation::Attacking:
            return TextureRectangle(
                Vector2u(
                    (size.x * (AssertCast<UInt32>((animationTime.Milliseconds / 200) % AttackFrames) + WalkFrames)),
                    (size.y * AssertCast<UInt32>(direction))),
                size);
        case ObjectAnimation::Dead:
            return TextureRectangle(Vector2u(0, (size.y * AssertCast<UInt32>(direction))), size);
        }
        return TextureRectangle(Vector2u((size.x * (AssertCast<UInt32>(animationTime.Milliseconds / 150) % WalkFrames)),
                                         (size.y * AssertCast<UInt32>(direction))),
                                size);
    }

    TextureRectangle CutWolfTexture(ObjectAnimation animation, TimeSpan animationTime, Direction direction,
                                    const Vector2u &size);
} // namespace ij
