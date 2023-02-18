#pragma once
#include "LogicEntity.h"
#include "TextureCutter.h"
#include <SFML/Graphics/Sprite.hpp>

namespace ij
{
    struct VisualEntity
    {
        const sf::Texture *Texture;
        Vector2u SpriteSize;
        Int32 VerticalOffset;
        TimeSpan AnimationTime;
        TextureCutter *Cutter;
        ObjectAnimation Animation;

        VisualEntity(const sf::Texture *texture, const Vector2u &spriteSize, Int32 verticalOffset,
                     TimeSpan animationTime, TextureCutter *cutter, ObjectAnimation animation);
        [[nodiscard]] Vector2f GetOffset() const;
        [[nodiscard]] sf::IntRect GetTextureRect(const Vector2f &direction) const;
        [[nodiscard]] Vector2f GetTopLeftPosition(const Vector2f &bottomLeftPosition) const;
    };

    void updateVisuals(const LogicEntity &logic, VisualEntity &visuals, TimeSpan deltaTime);
    [[nodiscard]] sf::Sprite CreateSpriteForVisualEntity(const LogicEntity &logic, const VisualEntity &visuals);
} // namespace ij
