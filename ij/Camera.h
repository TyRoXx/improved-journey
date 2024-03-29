#pragma once
#include "VisualEntity.h"

namespace ij
{
    struct Camera final
    {
        Vector2f Center;

        [[nodiscard]] Vector2f getWorldFromScreenCoordinates(const Vector2u &windowSize, const Vector2i &point) const;
        [[nodiscard]] bool canSee(const Vector2u &windowSize, const Vector2f &logicalPosition,
                                  const VisualEntity &entity) const;
    };
} // namespace ij
