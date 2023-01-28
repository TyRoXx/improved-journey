#include "PlayerCharacter.h"
#include "Normalize.h"
#include <cassert>

ij::PlayerCharacter::PlayerCharacter(const std::array<bool, 4> &isDirectionKeyPressed, bool &isAttackPressed)
    : isDirectionKeyPressed(isDirectionKeyPressed)
    , isAttackPressed(isAttackPressed)
{
}

void ij::PlayerCharacter::update(LogicEntity &object, LogicEntity &player, World &world, const sf::Time &deltaTime,
                                 RandomNumberGenerator &random)
{
    assert(&object == &player);
    (void)player;
    (void)deltaTime;

    sf::Vector2f direction;
    for (size_t i = 0; i < 4; ++i)
    {
        if (!isDirectionKeyPressed[i])
        {
            continue;
        }
        direction += DirectionToVector(AssertCast<Direction>(i));
    }
    if (direction == sf::Vector2f())
    {
        if (isAttackPressed && !isDead(object))
        {
            object.SetActivity(ObjectActivity::Attacking);
            for (Object &enemy : world.enemies)
            {
                InflictDamage(enemy.Logic, world, 2, random);
            }
        }
        else
        {
            object.SetActivity(ObjectActivity::Standing);
        }
    }
    else
    {
        object.SetActivity(ObjectActivity::Walking);
        object.Direction = normalize(direction);
    }
}
