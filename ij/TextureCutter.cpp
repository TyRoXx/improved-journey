#include "TextureCutter.h"

ij::TextureRectangle ij::CutWolfTexture(const ObjectAnimation animation, const TimeSpan animationTime,
                                        const Direction direction, const Vector2u &size)
{
    UInt32 x = 0;
    UInt32 yOffset = 8;
    switch (animation)
    {
    case ObjectAnimation::Standing:
        break;

    case ObjectAnimation::Walking:
        x = (size.x * AssertCast<UInt32>((animationTime.Milliseconds / 80) % 9));
        break;

    case ObjectAnimation::Attacking:
        yOffset = 0;
        x = (size.x * AssertCast<UInt32>((animationTime.Milliseconds / 80) % 7));
        break;

    case ObjectAnimation::Dead:
        return TextureRectangle(Vector2u((5 * size.x), (20 * size.y)), size);
    }
    return TextureRectangle(Vector2u(x, (size.y * (AssertCast<Int32>(direction) + yOffset))), size);
}
