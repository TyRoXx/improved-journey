#include "Camera.h"

void ij::Camera::draw(sf::RenderWindow &window, const sf::Sprite &sprite) const
{
    sf::Sprite moved(sprite);
    moved.move(-Center);
    moved.move(sf::Vector2f(window.getSize()) * 0.5f);
    window.draw(moved);
}

void ij::Camera::draw(sf::RenderWindow &window, const sf::CircleShape &shape) const
{
    sf::CircleShape moved(shape);
    moved.move(-Center);
    moved.move(sf::Vector2f(window.getSize()) * 0.5f);
    window.draw(moved);
}

void ij::Camera::draw(sf::RenderWindow &window, const sf::RectangleShape &shape) const
{
    sf::RectangleShape moved(shape);
    moved.move(-Center);
    moved.move(sf::Vector2f(window.getSize()) * 0.5f);
    window.draw(moved);
}

void ij::Camera::draw(sf::RenderWindow &window, const sf::Text &text) const
{
    sf::Text moved(text);
    moved.move(-Center);
    moved.move(sf::Vector2f(window.getSize()) * 0.5f);
    window.draw(moved);
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
