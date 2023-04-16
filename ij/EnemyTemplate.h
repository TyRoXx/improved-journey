#pragma once
#include "TextureCutter.h"
#include "World.h"

namespace ij
{
    struct EnemyTemplate final
    {
        TextureId Texture;
        Vector2u Size;
        int VerticalOffset;
        TextureCutter *Cutter;

        EnemyTemplate(TextureId texture, const Vector2u &size, int verticalOffset, TextureCutter *cutter);
    };

    void SpawnEnemies(World &world, size_t numberOfEnemies, const std::vector<EnemyTemplate> &enemies,
                      RandomNumberGenerator &randomNumberGenerator);
} // namespace ij
