#pragma once
#include "Color.h"
#include "Int.h"
#include "Vector2.h"

namespace ij
{
    struct TextureId final
    {
        UInt32 Value;

        explicit TextureId(UInt32 value);
    };

    struct Sprite final
    {
        TextureId Texture;
        Vector2i Position;
        Color ColorMultiplier;
        Vector2u TextureTopLeft;
        Vector2u TextureSize;

        Sprite(TextureId texture, const Vector2i &position, Color colorMultiplier, const Vector2u &textureTopLeft,
               const Vector2u &textureSize);
    };

} // namespace ij
