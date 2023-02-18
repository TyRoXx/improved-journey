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
        TextureId Texture;
        Vector2u Size;
        int VerticalOffset;
        TextureCutter *Cutter;

        EnemyTemplate(TextureId texture, const Vector2u &size, int verticalOffset, TextureCutter *cutter);
    };

    void SpawnEnemies(World &world, size_t numberOfEnemies, const std::vector<EnemyTemplate> &enemies,
                      RandomNumberGenerator &randomNumberGenerator);

    struct TextureManager final
    {
        [[nodiscard]] std::optional<TextureId> LoadFromFile(const std::filesystem::path &textureFile);
        const sf::Texture &GetTexture(const TextureId &id) const;

    private:
        std::vector<sf::Texture> _textures;
    };

    [[nodiscard]] std::optional<std::vector<EnemyTemplate>> LoadEnemies(TextureManager &textures,
                                                                        const std::filesystem::path &assets);
} // namespace ij
