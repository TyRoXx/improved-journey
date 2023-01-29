#include "Map.h"

size_t ij::Map::GetHeight() const
{
    return Tiles.size() / Width;
}

int ij::Map::GetTileAt(const size_t x, const size_t y) const
{
    return Tiles[(y * Width) + x];
}

[[nodiscard]] ij::Map ij::GenerateRandomMap(RandomNumberGenerator &random)
{
    Map result;
    result.Width = 500;
    for (size_t i = 0; i < (500 * result.Width); ++i)
    {
        result.Tiles.push_back(random.GenerateInt32(0, 3));
    }
    return result;
}
