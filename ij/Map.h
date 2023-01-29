#pragma once
#include "RandomNumberGenerator.h"
#include <vector>

namespace ij
{
    constexpr int NoTile = 3;

    struct Map final
    {
        std::vector<int> Tiles;
        size_t Width;

        [[nodiscard]] size_t GetHeight() const;
        [[nodiscard]] int GetTileAt(size_t x, size_t y) const;
    };

    [[nodiscard]] Map GenerateRandomMap(RandomNumberGenerator &random);
} // namespace ij
