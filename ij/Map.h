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

        size_t GetHeight() const
        {
            return Tiles.size() / Width;
        }

        int GetTileAt(const size_t x, const size_t y) const
        {
            return Tiles[(y * Width) + x];
        }
    };

    [[nodiscard]] Map GenerateRandomMap(RandomNumberGenerator &random);
} // namespace ij
