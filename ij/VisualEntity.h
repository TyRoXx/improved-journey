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
        sf::Int32 VerticalOffset = 0;
        sf::Int32 AnimationTime = 0;
        TextureCutter *Cutter = nullptr;
        ObjectAnimation Animation = ObjectAnimation::Standing;

        sf::Vector2f GetOffset() const;
    };

    void updateVisuals(const LogicEntity &logic, VisualEntity &visuals, const sf::Time &deltaTime);
} // namespace ij
