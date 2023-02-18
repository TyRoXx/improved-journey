#include "EnemyTemplate.h"
#include "Bot.h"
#include <array>

ij::EnemyTemplate::EnemyTemplate(const sf::Texture &texture, const sf::Vector2i &size, const int verticalOffset,
                                 TextureCutter *const cutter)
    : Texture(texture)
    , Size(size)
    , VerticalOffset(verticalOffset)
    , Cutter(cutter)
{
}

void ij::SpawnEnemies(World &world, const size_t numberOfEnemies, const std::vector<EnemyTemplate> &enemies,
                      RandomNumberGenerator &randomNumberGenerator)
{
    for (const EnemyTemplate &enemyTemplate : enemies)
    {
        for (size_t k = 0; k < (numberOfEnemies / enemies.size()); ++k)
        {
            const sf::Vector2f position = GenerateRandomPointForSpawning(world, randomNumberGenerator);
            const sf::Vector2f direction =
                DirectionToVector(AssertCast<Direction>(randomNumberGenerator.GenerateInt32(0, 3)));
            world.enemies.emplace_back(
                VisualEntity(&enemyTemplate.Texture, enemyTemplate.Size, enemyTemplate.VerticalOffset, 0,
                             enemyTemplate.Cutter, ObjectAnimation::Standing),
                LogicEntity(
                    std::make_unique<Bot>(), position, direction, true, false, 100, 100, ObjectActivity::Standing));
        }
    }
}

std::optional<std::vector<ij::EnemyTemplate>> ij::LoadEnemies(const std::filesystem::path &assets)
{
    std::array<sf::Texture, 10> enemyTextures = {};
    const std::array<const char *, 10> enemyFileNames = {
        "bat", "bee", "big_worm", "eyeball", "ghost", "man_eater_flower", "pumpking", "slime", "small_worm", "snake"};
    for (size_t i = 0; i < enemyFileNames.size(); ++i)
    {
        const std::filesystem::path enemyFile = (assets / "lpc-monsters" / (std::string(enemyFileNames[i]) + ".png"));
        if (!enemyTextures[i].loadFromFile(enemyFile.string()))
        {
            return std::nullopt;
        }
    }

    std::vector<EnemyTemplate> enemies;
    enemies.emplace_back(enemyTextures[0], sf::Vector2i(64, 64), 4, &cutEnemyTexture<4, 3>);
    enemies.emplace_back(enemyTextures[1], sf::Vector2i(32, 32), 2, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(enemyTextures[2], sf::Vector2i(64, 64), 18, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(enemyTextures[3], sf::Vector2i(64, 64), 17, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(enemyTextures[4], sf::Vector2i(64, 64), 13, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(enemyTextures[5], sf::Vector2i(128, 128), 28, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(enemyTextures[6], sf::Vector2i(64, 64), 10, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(enemyTextures[7], sf::Vector2i(64, 64), 20, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(enemyTextures[8], sf::Vector2i(64, 64), 19, &cutEnemyTexture<3, 7>);
    enemies.emplace_back(enemyTextures[9], sf::Vector2i(64, 64), 18, &cutEnemyTexture<4, 3>);
    return enemies;
}
