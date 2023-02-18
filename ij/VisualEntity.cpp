#include "VisualEntity.h"

ij::VisualEntity::VisualEntity(const sf::Texture *texture, const sf::Vector2i &spriteSize, Int32 verticalOffset,
                               Int32 animationTime, TextureCutter *cutter, ObjectAnimation animation)
    : Texture(texture)
    , SpriteSize(spriteSize)
    , VerticalOffset(verticalOffset)
    , AnimationTime(animationTime)
    , Cutter(cutter)
    , Animation(animation)
{
    assert(Texture);
}

sf::Vector2f ij::VisualEntity::GetOffset() const
{
    return sf::Vector2f(AssertCast<float>(SpriteSize.x / 2), AssertCast<float>(SpriteSize.y)) -
           sf::Vector2f(0, AssertCast<float>(VerticalOffset));
}

sf::IntRect ij::VisualEntity::GetTextureRect(const sf::Vector2f &direction) const
{
    return Cutter(Animation, AnimationTime, DirectionFromVector(direction), SpriteSize);
}

sf::Vector2f ij::VisualEntity::GetTopLeftPosition(const sf::Vector2f &bottomLeftPosition) const
{
    return bottomLeftPosition - GetOffset();
}

void ij::updateVisuals(const LogicEntity &logic, VisualEntity &visuals, const sf::Time &deltaTime)
{
    switch (logic.GetActivity())
    {
    case ObjectActivity::Standing:
        visuals.Animation = ObjectAnimation::Standing;
        break;

    case ObjectActivity::Attacking:
        visuals.Animation = ObjectAnimation::Attacking;
        break;

    case ObjectActivity::Dead:
        visuals.Animation = ObjectAnimation::Dead;
        break;

    case ObjectActivity::Walking: {
        visuals.Animation = ObjectAnimation::Walking;
        break;
    }
    }
    visuals.AnimationTime += deltaTime.asMilliseconds();
}

sf::Sprite ij::CreateSpriteForVisualEntity(const LogicEntity &logic, const VisualEntity &visuals)
{
    bool isColoredDead = false;
    switch (visuals.Animation)
    {
    case ObjectAnimation::Standing:
    case ObjectAnimation::Walking:
    case ObjectAnimation::Attacking:
        break;
    case ObjectAnimation::Dead:
        isColoredDead = true;
        break;
    }
    assert(visuals.Texture);
    sf::Sprite result(*visuals.Texture);
    result.setTextureRect(visuals.GetTextureRect(logic.Direction));
    // the position of an object is at the bottom center of the sprite (on the ground)
    result.setPosition(visuals.GetTopLeftPosition(logic.Position));
    result.setColor(isColoredDead ? sf::Color(128, 128, 128, 255) : sf::Color::White);
    return result;
}
