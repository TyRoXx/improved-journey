#pragma once
#include "LogicEntity.h"
#include "RandomNumberGenerator.h"
#include "World.h"

namespace ij
{
    struct Bot final : ObjectBehavior
    {
        void update(LogicEntity &object, LogicEntity &player, World &world, const sf::Time &deltaTime,
                    RandomNumberGenerator &random) override;

        enum class State
        {
            MovingAround,
            Chasing,
            Attacking
        };

        [[nodiscard]] State GetState() const;
        [[nodiscard]] static const char *GetStateName(State state);
        [[nodiscard]] LogicEntity *GetTarget() const;

    private:
        State _state = State::MovingAround;
        LogicEntity *_target = nullptr;
        Int32 _sinceLastAttack = 0;
    };
} // namespace ij
