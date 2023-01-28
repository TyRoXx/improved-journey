#pragma once
#include "Direction.h"
#include "LogicEntity.h"
#include "World.h"
#include <array>

namespace ij
{
    struct PlayerCharacter final : ObjectBehavior
    {
        const std::array<bool, 4> &isDirectionKeyPressed;
        bool &isAttackPressed;

        explicit PlayerCharacter(const std::array<bool, 4> &isDirectionKeyPressed, bool &isAttackPressed);
        virtual void update(LogicEntity &object, LogicEntity &player, World &world, const sf::Time &deltaTime,
                            RandomNumberGenerator &random) final;
    };

} // namespace ij