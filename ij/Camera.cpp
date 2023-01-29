#include "Camera.h"

void ij::Camera::draw(sf::RenderWindow &window, const sf::Sprite &sprite) const
{
    window.draw(sprite);
}

void ij::Camera::draw(sf::RenderWindow &window, const sf::CircleShape &shape) const
{
    window.draw(shape);
}

void ij::Camera::draw(sf::RenderWindow &window, const sf::RectangleShape &shape) const
{
    window.draw(shape);
}

void ij::Camera::draw(sf::RenderWindow &window, const sf::Text &text) const
{
    window.draw(text);
}

sf::Vector2f ij::Camera::getWorldFromScreenCoordinates(const sf::RenderWindow &window, const sf::Vector2i &point) const
{
    return (sf::Vector2f(window.getSize()) * -0.5f) + Center + sf::Vector2f(point);
}

bool ij::Camera::canSee(const sf::RenderWindow &window, const VisualEntity &entity) const
{
    const sf::Rect<float> cameraArea(
        getWorldFromScreenCoordinates(window, sf::Vector2i(0, 0)), sf::Vector2f(window.getSize()));
    const sf::Rect<float> entityArea(entity.Sprite.getPosition(), sf::Vector2f(entity.SpriteSize));
    return cameraArea.intersects(entityArea);
}
