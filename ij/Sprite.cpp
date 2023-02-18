#include "Sprite.h"

ij::TextureId::TextureId(UInt32 value)
    : Value(value)
{
}

ij::Sprite::Sprite(TextureId texture, const Vector2i &position, Color colorMultiplier, const Vector2u &textureTopLeft,
                   const Vector2u &textureSize)
    : Texture(texture)
    , Position(position)
    , ColorMultiplier(colorMultiplier)
    , TextureTopLeft(textureTopLeft)
    , TextureSize(textureSize)
{
}
