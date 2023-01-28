#include "Map.h"

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
