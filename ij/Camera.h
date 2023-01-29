#pragma once
#include "VisualEntity.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

namespace ij
{
    struct Camera final
    {
        sf::Vector2f Center;

        void draw(sf::RenderWindow &window, const sf::Sprite &sprite);
        void draw(sf::RenderWindow &window, const sf::CircleShape &shape);
        void draw(sf::RenderWindow &window, const sf::RectangleShape &shape);
        void draw(sf::RenderWindow &window, const sf::Text &text);
        [[nodiscard]] sf::Vector2f getWorldFromScreenCoordinates(const sf::RenderWindow &window,
                                                                 const sf::Vector2i &point) const;
        [[nodiscard]] bool canSee(const sf::RenderWindow &window, const VisualEntity &entity) const;
    };
} // namespace ij
