#include "Camera.h"

ij::Vector2f ij::Camera::getWorldFromScreenCoordinates(const Vector2u &windowSize, const Vector2i &point) const
{
    return (AssertCastVector<float>(windowSize) * -0.5f) + Center + AssertCastVector<float>(point);
}

bool ij::Camera::canSee(const Vector2u &windowSize, const Vector2f &logicalPosition, const VisualEntity &entity) const
{
    const Rectangle<float> cameraArea(
        getWorldFromScreenCoordinates(windowSize, Vector2i(0, 0)), AssertCastVector<float>(windowSize));
    const Rectangle<float> entityArea(
        entity.GetTopLeftPosition(logicalPosition), AssertCastVector<float>(entity.SpriteSize));
    return cameraArea.Intersects(entityArea);
}
