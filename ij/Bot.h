#pragma once
#include "AssertCast.h"
#include "LogicEntity.h"
#include "Normalize.h"
#include "RandomNumberGenerator.h"
#include "Unreachable.h"
#include "World.h"

namespace ij
{
    bool isWithinDistance(const sf::Vector2f &first, const sf::Vector2f &second, const float distance);

    struct Bot final : ObjectBehavior
    {
        virtual void update(LogicEntity &object, LogicEntity &player, World &world, const sf::Time &deltaTime,
                            RandomNumberGenerator &random) final
        {
            (void)world;
            if (isDead(object))
            {
                return;
            }
            switch (_state)
            {
            case State::MovingAround:
                if (isWithinDistance(object.Position, player.Position, 400) && !isDead(player))
                {
                    _state = State::Chasing;
                    _target = &player;
                    break;
                }
                if (object.HasBumpedIntoWall || (random.GenerateInt32(0, 1999) < 10))
                {
                    switch (object.GetActivity())
                    {
                    case ObjectActivity::Standing:
                        object.SetActivity(ObjectActivity::Walking);
                        break;
                    case ObjectActivity::Walking:
                    case ObjectActivity::Attacking:
                    case ObjectActivity::Dead:
                        object.SetActivity(ObjectActivity::Standing);
                        break;
                    }
                    object.Direction = normalize(sf::Vector2f(AssertCast<float>(random.GenerateInt32(0, 9) - 5),
                                                              AssertCast<float>(random.GenerateInt32(0, 9) - 5)));
                }
                break;

            case State::Chasing:
                assert(_target);
                if (isDead(*_target))
                {
                    object.SetActivity(ObjectActivity::Standing);
                    _state = State::MovingAround;
                    _target = nullptr;
                }
                else if (isWithinDistance(object.Position, _target->Position, 40))
                {
                    _state = State::Attacking;
                    object.SetActivity(ObjectActivity::Standing);
                }
                else if (isWithinDistance(object.Position, _target->Position, 600))
                {
                    object.SetActivity(ObjectActivity::Walking);
                    object.Direction = normalize(_target->Position - object.Position);
                }
                else
                {
                    object.SetActivity(ObjectActivity::Standing);
                    _state = State::MovingAround;
                    _target = nullptr;
                }
                break;

            case State::Attacking:
                if (!isWithinDistance(object.Position, _target->Position, 60))
                {
                    _state = State::Chasing;
                    object.SetActivity(ObjectActivity::Standing);
                    break;
                }
                _sinceLastAttack += deltaTime.asMilliseconds();
                constexpr sf::Int32 attackDelay = 1000;
                while (_sinceLastAttack >= attackDelay)
                {
                    object.SetActivity(ObjectActivity::Attacking);
                    InflictDamage(player, world, 1, random);
                    _sinceLastAttack -= attackDelay;
                }
                if (isDead(player))
                {
                    _state = State::MovingAround;
                    object.SetActivity(ObjectActivity::Standing);
                }
                break;
            }
            // acknowledged
            object.HasBumpedIntoWall = false;
        }

        enum class State
        {
            MovingAround,
            Chasing,
            Attacking
        };

        [[nodiscard]] State GetState() const
        {
            return _state;
        }

        [[nodiscard]] static const char *GetStateName(const State state)
        {
            switch (state)
            {
            case State::MovingAround:
                return "MovingAround";
            case State::Chasing:
                return "Chasing";
            case State::Attacking:
                return "Attacking";
            }
            IJ_UNREACHABLE();
        }

        [[nodiscard]] LogicEntity *GetTarget() const
        {
            return _target;
        }

    private:
        State _state = State::MovingAround;
        LogicEntity *_target = nullptr;
        sf::Int32 _sinceLastAttack = 0;
    };

} // namespace ij
