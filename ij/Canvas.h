#pragma once
#include "Sprite.h"
#include <string>

namespace ij
{
    using TextId = size_t;
    using FontId = size_t;

    struct Canvas;

    struct Text
    {
        Text();
        Text(Text &&other) noexcept;
        Text(Canvas &canvas, TextId id);
        ~Text();
        Text &operator=(Text &&other) noexcept;
        void SetPosition(const Vector2f &position);
        Vector2f GetPosition();
        void Draw();

    private:
        Canvas *_canvas = nullptr;
        TextId _id = 0;
    };

    struct Canvas
    {
        virtual ~Canvas();
        [[nodiscard]] virtual Vector2u GetSize() = 0;
        virtual void DrawDot(const Vector2i &position, Color color) = 0;
        virtual void DrawRectangle(const Vector2i &topLeft, const Vector2u &size, Color outline, Color fill,
                                   float outlineThickness) = 0;
        virtual void DrawSprite(const Sprite &sprite) = 0;
        [[nodiscard]] virtual Text CreateText(const std::string &content, FontId font, UInt32 size,
                                              const Vector2f &position, Color fillColor, Color outlineColor,
                                              float outlineThickness) = 0;
        virtual void SetTextPosition(TextId id, const Vector2f &position) = 0;
        [[nodiscard]] virtual Vector2f GetTextPosition(TextId id) = 0;
        virtual void DeleteText(TextId id) = 0;
        virtual void DrawText(TextId id) = 0;
    };

} // namespace ij
