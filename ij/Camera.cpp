#include "Camera.h"

bool ij::Camera::canSee(const sf::RenderWindow &window, const VisualEntity &entity) const
{
    const sf::Rect<float> cameraArea(
        getWorldFromScreenCoordinates(window, sf::Vector2i(0, 0)), sf::Vector2f(window.getSize()));
    const sf::Rect<float> entityArea(entity.Sprite.getPosition(), sf::Vector2f(entity.SpriteSize));
    return cameraArea.intersects(entityArea);
}
