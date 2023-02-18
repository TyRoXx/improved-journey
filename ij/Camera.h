#pragma once
#include "VisualEntity.h"

namespace ij
{
    struct Camera final
    {
        Vector2f Center;

        [[nodiscard]] Vector2f getWorldFromScreenCoordinates(const sf::Vector2u &windowSize,
                                                             const sf::Vector2i &point) const;
        [[nodiscard]] bool canSee(const sf::Vector2u &windowSize, const Vector2f &logicalPosition,
                                  const VisualEntity &entity) const;
    };
} // namespace ij
