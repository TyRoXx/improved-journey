#pragma once
#include "TextureCutter.h"
#include "World.h"
#include <SFML/Graphics/Texture.hpp>
#include <filesystem>
#include <optional>

namespace ij
{
    struct EnemyTemplate final
    {
        sf::Texture Texture;
        Vector2u Size;
        int VerticalOffset;
        TextureCutter *Cutter;

        EnemyTemplate(const sf::Texture &texture, const Vector2u &size, int verticalOffset, TextureCutter *cutter);
    };

    void SpawnEnemies(World &world, size_t numberOfEnemies, const std::vector<EnemyTemplate> &enemies,
                      RandomNumberGenerator &randomNumberGenerator);
    [[nodiscard]] std::optional<std::vector<EnemyTemplate>> LoadEnemies(const std::filesystem::path &assets);
} // namespace ij
