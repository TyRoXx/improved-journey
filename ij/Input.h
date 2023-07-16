#pragma once
#include "World.h"
#include <array>

namespace ij
{
    struct Input final
    {
        std::array<bool, 4> isDirectionKeyPressed = {};
        bool isAttackPressed = false;
        Object *selectedEnemy = nullptr;
    };
} // namespace ij
