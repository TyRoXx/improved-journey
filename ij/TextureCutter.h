#pragma once
#include "AssertCast.h"
#include "Direction.h"
#include "Int.h"
#include "ObjectAnimation.h"
#include "TimeSpan.h"

namespace ij
{
    template <class T>
    [[nodiscard]] bool IsValueInRange(T value, T min, T max) noexcept
    {
        return (value >= min) && (value <= max);
    }

    template <class T>
    struct Rectangle final
    {
        Vector2<T> Position;
        Vector2<T> Size;

        Rectangle(const Vector2<T> &position, const Vector2<T> &size)
            : Position(position)
            , Size(size)
        {
        }

        [[nodiscard]] bool Intersects(const Rectangle &other) const noexcept
        {
            const bool xOverlap = IsValueInRange(Position.x, other.Position.x, other.Position.x + other.Size.x) ||
                                  IsValueInRange(other.Position.x, Position.x, Position.x + Size.x);

            const bool yOverlap = IsValueInRange(Position.y, other.Position.y, other.Position.y + other.Size.y) ||
                                  IsValueInRange(other.Position.y, Position.y, Position.y + Size.y);

            return xOverlap && yOverlap;
        }
    };

    using TextureRectangle = Rectangle<UInt32>;
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
