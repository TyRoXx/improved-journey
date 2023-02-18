#pragma once
#include "VisualEntity.h"
#include <SFML/Graphics/RenderWindow.hpp>

namespace ij
{
    struct Camera final
    {
        sf::Vector2f Center;

        [[nodiscard]] sf::Vector2f getWorldFromScreenCoordinates(const sf::RenderWindow &window,
                                                                 const sf::Vector2i &point) const;
        [[nodiscard]] bool canSee(const sf::RenderWindow &window, const VisualEntity &entity) const;
    };
} // namespace ij
