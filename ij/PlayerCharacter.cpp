#include "PlayerCharacter.h"
#include "Normalize.h"
#include <cassert>

ij::PlayerCharacter::PlayerCharacter(const std::array<bool, 4> &isDirectionKeyPressed, bool &isAttackPressed)
    : isDirectionKeyPressed(isDirectionKeyPressed)
    , isAttackPressed(isAttackPressed)
{
}

void ij::PlayerCharacter::update(LogicEntity &object, LogicEntity &player, World &world, const TimeSpan deltaTime,
                                 RandomNumberGenerator &random)
{
    assert(&object == &player);
    (void)player;
    (void)deltaTime;

    Vector2f direction(0, 0);
    for (size_t i = 0; i < 4; ++i)
    {
        if (!isDirectionKeyPressed[i])
        {
            continue;
        }
        direction += DirectionToVector(AssertCast<Direction>(i));
    }
    if ((direction.x == 0) && (direction.y == 0))
    {
        if (isAttackPressed && !isDead(object))
        {
            object.SetActivity(ObjectActivity::Attacking);
            for (Object *const enemy : FindEnemiesInCircle(world, object.Position, 100.0f))
            {
                InflictDamage(enemy->Logic, world, 2, random);
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
