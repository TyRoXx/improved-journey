#pragma once
#include "TextureCutter.h"
#include "World.h"
#include <filesystem>
#include <optional>

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

    struct TextureLoader
    {
        virtual ~TextureLoader();
        [[nodiscard]] virtual std::optional<TextureId> LoadFromFile(const std::filesystem::path &textureFile) = 0;
    };

    std::optional<std::vector<EnemyTemplate>> LoadEnemies(TextureLoader &textures, const std::filesystem::path &assets);
    void SpawnEnemies(World &world, size_t numberOfEnemies, const std::vector<EnemyTemplate> &enemies,
                      RandomNumberGenerator &randomNumberGenerator);
} // namespace ij
