#pragma once
#include "LogicEntity.h"
#include "Sprite.h"
#include "TextureCutter.h"

namespace ij
{
    struct VisualEntity final
    {
        TextureId Texture;
        Vector2u SpriteSize;
        Int32 VerticalOffset;
        TimeSpan AnimationTime;
        TextureCutter *Cutter;
        ObjectAnimation Animation;

        VisualEntity(TextureId texture, const Vector2u &spriteSize, Int32 verticalOffset, TimeSpan animationTime,
                     TextureCutter *cutter, ObjectAnimation animation);
        [[nodiscard]] Vector2f GetOffset() const;
        [[nodiscard]] TextureRectangle GetTextureRect(const Vector2f &direction) const;
        [[nodiscard]] Vector2f GetTopLeftPosition(const Vector2f &bottomLeftPosition) const;
    };

    void updateVisuals(const LogicEntity &logic, VisualEntity &visuals, TimeSpan deltaTime);
    [[nodiscard]] Sprite CreateSpriteForVisualEntity(const LogicEntity &logic, const VisualEntity &visuals);
} // namespace ij
