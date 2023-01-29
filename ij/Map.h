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

        size_t GetHeight() const;
        int GetTileAt(const size_t x, const size_t y) const;
    };

    [[nodiscard]] Map GenerateRandomMap(RandomNumberGenerator &random);
} // namespace ij
