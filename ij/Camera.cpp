#include "Camera.h"

sf::Vector2f ij::Camera::getWorldFromScreenCoordinates(const sf::Vector2u &windowSize, const sf::Vector2i &point) const
{
    return (sf::Vector2f(windowSize) * -0.5f) + Center + sf::Vector2f(point);
}

bool ij::Camera::canSee(const sf::Vector2u &windowSize, const sf::Vector2f &logicalPosition,
                        const VisualEntity &entity) const
{
    const sf::Rect<float> cameraArea(
        getWorldFromScreenCoordinates(windowSize, sf::Vector2i(0, 0)), sf::Vector2f(windowSize));
    const sf::Rect<float> entityArea(entity.GetTopLeftPosition(logicalPosition), sf::Vector2f(entity.SpriteSize));
    return cameraArea.intersects(entityArea);
}
