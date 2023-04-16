#pragma once

namespace ij
{
    struct LogicEntity;
    struct World;
    struct Input;
    struct Debugging;

    void UpdateUserInterface(LogicEntity &player, const World &world, const Input &input, Debugging &debugging);
} // namespace ij
