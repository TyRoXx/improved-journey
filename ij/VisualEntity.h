#pragma once
#include "LogicEntity.h"
#include "TextureCutter.h"
#include <SFML/Graphics/Sprite.hpp>

namespace ij
{
    struct VisualEntity
    {
        const sf::Texture *Texture;
        sf::Vector2i SpriteSize;
        Int32 VerticalOffset;
        Int32 AnimationTime;
        TextureCutter *Cutter;
        ObjectAnimation Animation;

        VisualEntity(const sf::Texture *texture, const sf::Vector2i &spriteSize, Int32 verticalOffset,
                     Int32 animationTime, TextureCutter *cutter, ObjectAnimation animation);
        [[nodiscard]] sf::Vector2f GetOffset() const;
        [[nodiscard]] sf::IntRect GetTextureRect(const sf::Vector2f &direction) const;
        [[nodiscard]] sf::Vector2f GetTopLeftPosition(const sf::Vector2f &bottomLeftPosition) const;
    };

    void updateVisuals(const LogicEntity &logic, VisualEntity &visuals, const sf::Time &deltaTime);
    [[nodiscard]] sf::Sprite CreateSpriteForVisualEntity(const LogicEntity &logic, const VisualEntity &visuals);
} // namespace ij
