#pragma once
#include "VisualEntity.h"

namespace ij
{
    struct Camera final
    {
        sf::Vector2f Center;

        [[nodiscard]] sf::Vector2f getWorldFromScreenCoordinates(const sf::Vector2u &windowSize,
                                                                 const sf::Vector2i &point) const;
        [[nodiscard]] bool canSee(const sf::Vector2u &windowSize, const sf::Vector2f &logicalPosition,
                                  const VisualEntity &entity) const;
    };
} // namespace ij
