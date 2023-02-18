#include "Camera.h"
#include "FromSfml.h"
#include "ToSfml.h"

ij::Vector2f ij::Camera::getWorldFromScreenCoordinates(const Vector2u &windowSize, const Vector2i &point) const
{
    return (AssertCastVector<float>(windowSize) * -0.5f) + Center + AssertCastVector<float>(point);
}

bool ij::Camera::canSee(const Vector2u &windowSize, const Vector2f &logicalPosition, const VisualEntity &entity) const
{
    const sf::Rect<float> cameraArea(
        ToSfml(getWorldFromScreenCoordinates(windowSize, Vector2i(0, 0))), sf::Vector2f(ToSfml(windowSize)));
    const sf::Rect<float> entityArea(
        ToSfml(entity.GetTopLeftPosition(logicalPosition)), sf::Vector2f(ToSfml(entity.SpriteSize)));
    return cameraArea.intersects(entityArea);
}
