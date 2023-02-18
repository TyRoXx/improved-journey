#include "Camera.h"
#include "FromSfml.h"
#include "ToSfml.h"

ij::Vector2f ij::Camera::getWorldFromScreenCoordinates(const sf::Vector2u &windowSize, const sf::Vector2i &point) const
{
    return (FromSfml(sf::Vector2f(windowSize)) * -0.5f) + Center + FromSfml(sf::Vector2f(point));
}

bool ij::Camera::canSee(const sf::Vector2u &windowSize, const Vector2f &logicalPosition,
                        const VisualEntity &entity) const
{
    const sf::Rect<float> cameraArea(
        ToSfml(getWorldFromScreenCoordinates(windowSize, sf::Vector2i(0, 0))), sf::Vector2f(windowSize));
    const sf::Rect<float> entityArea(
        ToSfml(entity.GetTopLeftPosition(logicalPosition)), sf::Vector2f(ToSfml(entity.SpriteSize)));
    return cameraArea.intersects(entityArea);
}
