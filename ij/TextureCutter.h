#pragma once
#include "AssertCast.h"
#include "Direction.h"
#include "ObjectAnimation.h"
#include <SFML/Config.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace ij
{
    using TextureCutter = sf::IntRect(ObjectAnimation animation, sf::Int32 animationTime, Direction direction,
                                      const sf::Vector2i &size);

    template <sf::Int32 WalkFrames, sf::Int32 AttackFrames>
    sf::IntRect cutEnemyTexture(const ObjectAnimation animation, const sf::Int32 animationTime,
                                const Direction direction, const sf::Vector2i &size)
    {
        switch (animation)
        {
        case ObjectAnimation::Standing:
        case ObjectAnimation::Walking:
            break;
        case ObjectAnimation::Attacking:
            return sf::IntRect(size.x * (((animationTime / 200) % AttackFrames) + WalkFrames),
                               size.y * AssertCast<int>(direction), size.x, size.y);
        case ObjectAnimation::Dead:
            return sf::IntRect(0, size.y * AssertCast<int>(direction), size.x, size.y);
        }
        return sf::IntRect(
            size.x * ((animationTime / 150) % WalkFrames), size.y * AssertCast<int>(direction), size.x, size.y);
    };

    sf::IntRect CutWolfTexture(ObjectAnimation animation, sf::Int32 animationTime, Direction direction,
                               const sf::Vector2i &size);
} // namespace ij
