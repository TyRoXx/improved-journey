#include "VisualEntity.h"

sf::Vector2f ij::VisualEntity::GetOffset() const
{
    return sf::Vector2f(AssertCast<float>(SpriteSize.x / 2), AssertCast<float>(SpriteSize.y)) -
           sf::Vector2f(0, AssertCast<float>(VerticalOffset));
}
