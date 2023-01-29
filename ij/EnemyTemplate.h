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
        sf::Vector2i Size;
        int VerticalOffset;
        TextureCutter *Cutter;

        EnemyTemplate(const sf::Texture &texture, const sf::Vector2i &size, const int verticalOffset,
                      TextureCutter *const cutter);
    };

    void SpawnEnemies(World &world, const size_t numberOfEnemies, const std::vector<EnemyTemplate> &enemies,
                      RandomNumberGenerator &randomNumberGenerator);
    [[nodiscard]] std::optional<std::vector<EnemyTemplate>> LoadEnemies(const std::filesystem::path &assets);
} // namespace ij
