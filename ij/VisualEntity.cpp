#include "VisualEntity.h"

sf::Vector2f ij::VisualEntity::GetOffset() const
{
    return sf::Vector2f(AssertCast<float>(SpriteSize.x / 2), AssertCast<float>(SpriteSize.y)) -
           sf::Vector2f(0, AssertCast<float>(VerticalOffset));
}

void ij::updateVisuals(const LogicEntity &logic, VisualEntity &visuals, const sf::Time &deltaTime)
{
    bool isColoredDead = false;
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
        isColoredDead = true;
        break;

    case ObjectActivity::Walking: {
        visuals.Animation = ObjectAnimation::Walking;
        break;
    }
    }

    visuals.AnimationTime += deltaTime.asMilliseconds();
    visuals.Sprite.setTextureRect(visuals.Cutter(
        visuals.Animation, visuals.AnimationTime, DirectionFromVector(logic.Direction), visuals.SpriteSize));
    // the position of an object is at the bottom center of the sprite (on the ground)
    visuals.Sprite.setPosition(logic.Position - visuals.GetOffset());
    visuals.Sprite.setColor(isColoredDead ? sf::Color(128, 128, 128, 255) : sf::Color::White);
}
