#pragma once
#include "LogicEntity.h"
#include "TextureCutter.h"
#include <SFML/Graphics/Sprite.hpp>

namespace ij
{
    struct VisualEntity
    {
        sf::Sprite Sprite;
        sf::Vector2i SpriteSize;
        Int32 VerticalOffset;
        Int32 AnimationTime;
        TextureCutter *Cutter;
        ObjectAnimation Animation;

        VisualEntity(const sf::Sprite &sprite, const sf::Vector2i &spriteSize, Int32 verticalOffset,
                     Int32 animationTime, TextureCutter *cutter, ObjectAnimation animation);
        sf::Vector2f GetOffset() const;
    };

    void updateVisuals(const LogicEntity &logic, VisualEntity &visuals, const sf::Time &deltaTime);
} // namespace ij
