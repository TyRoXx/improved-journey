#pragma once
#include "VisualEntity.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

namespace ij
{
    struct Camera
    {
        sf::Vector2f Center;

        void draw(sf::RenderWindow &window, const sf::Sprite &sprite)
        {
            sf::Sprite moved(sprite);
            moved.move(-Center);
            moved.move(sf::Vector2f(window.getSize()) * 0.5f);
            window.draw(moved);
        }

        void draw(sf::RenderWindow &window, const sf::CircleShape &shape)
        {
            sf::CircleShape moved(shape);
            moved.move(-Center);
            moved.move(sf::Vector2f(window.getSize()) * 0.5f);
            window.draw(moved);
        }

        void draw(sf::RenderWindow &window, const sf::RectangleShape &shape)
        {
            sf::RectangleShape moved(shape);
            moved.move(-Center);
            moved.move(sf::Vector2f(window.getSize()) * 0.5f);
            window.draw(moved);
        }

        void draw(sf::RenderWindow &window, const sf::Text &text)
        {
            sf::Text moved(text);
            moved.move(-Center);
            moved.move(sf::Vector2f(window.getSize()) * 0.5f);
            window.draw(moved);
        }

        sf::Vector2f getWorldFromScreenCoordinates(const sf::RenderWindow &window, const sf::Vector2i &point) const
        {
            return (sf::Vector2f(window.getSize()) * -0.5f) + Center + sf::Vector2f(point);
        }

        bool canSee(const sf::RenderWindow &window, const VisualEntity &entity) const;
    };
} // namespace ij
