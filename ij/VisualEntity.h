#pragma once
#include "LogicEntity.h"
#include "TextureCutter.h"

namespace ij
{
    struct TextureId final
    {
        UInt32 Value;

        explicit TextureId(UInt32 value)
            : Value(value)
        {
        }
    };

    struct VisualEntity final
    {
        TextureId Texture;
        Vector2u SpriteSize;
        Int32 VerticalOffset;
        TimeSpan AnimationTime;
        TextureCutter *Cutter;
        ObjectAnimation Animation;

        VisualEntity(TextureId texture, const Vector2u &spriteSize, Int32 verticalOffset, TimeSpan animationTime,
                     TextureCutter *cutter, ObjectAnimation animation);
        [[nodiscard]] Vector2f GetOffset() const;
        [[nodiscard]] TextureRectangle GetTextureRect(const Vector2f &direction) const;
        [[nodiscard]] Vector2f GetTopLeftPosition(const Vector2f &bottomLeftPosition) const;
    };

    void updateVisuals(const LogicEntity &logic, VisualEntity &visuals, TimeSpan deltaTime);

    struct Color final
    {
        std::uint8_t Red, Green, Blue, Alpha;

        Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) noexcept
            : Red(red)
            , Green(green)
            , Blue(blue)
            , Alpha(alpha)
        {
        }
    };

    struct Sprite final
    {
        TextureId Texture;
        Vector2i Position;
        Color ColorMultiplier;
        Vector2u TextureTopLeft;
        Vector2u TextureSize;

        Sprite(TextureId texture, const Vector2i &position, Color colorMultiplier, const Vector2u &textureTopLeft,
               const Vector2u &textureSize)
            : Texture(texture)
            , Position(position)
            , ColorMultiplier(colorMultiplier)
            , TextureTopLeft(textureTopLeft)
            , TextureSize(textureSize)
        {
        }
    };

    [[nodiscard]] Sprite CreateSpriteForVisualEntity(const LogicEntity &logic, const VisualEntity &visuals);
} // namespace ij
