#pragma once
#include "Canvas.h"
#include "RandomNumberGenerator.h"
#include "TimeSpan.h"

namespace ij
{
    struct FloatingText final
    {
        Text VisualItem;
        TimeSpan Age;
        TimeSpan MaxAge;

        explicit FloatingText(Canvas &canvas, const std::string &text, const Vector2f &position, FontId font,
                              RandomNumberGenerator &random);
        void Update(TimeSpan deltaTime);
        [[nodiscard]] bool HasExpired() const;
    };
} // namespace ij
