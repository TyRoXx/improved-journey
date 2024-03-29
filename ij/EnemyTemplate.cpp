#include "EnemyTemplate.h"
#include "Bot.h"
#include <array>

ij::EnemyTemplate::EnemyTemplate(TextureId texture, const Vector2u &size, const int verticalOffset,
                                 TextureCutter *const cutter)
    : Texture(texture)
    , Size(size)
    , VerticalOffset(verticalOffset)
    , Cutter(cutter)
{
}

ij::TextureLoader::~TextureLoader()
{
}

std::optional<std::vector<ij::EnemyTemplate>> ij::LoadEnemies(TextureLoader &textures,
                                                              const std::filesystem::path &assets)
{
    const std::array<const char *, 10> enemyFileNames = {
        "bat", "bee", "big_worm", "eyeball", "ghost", "man_eater_flower", "pumpking", "slime", "small_worm", "snake"};
    std::array<UInt32, 10> enemyTextures = {};
    for (size_t i = 0; i < enemyFileNames.size(); ++i)
    {
        const std::filesystem::path enemyFile = (assets / "lpc-monsters" / (std::string(enemyFileNames[i]) + ".png"));
        const std::optional<TextureId> loaded = textures.LoadFromFile(enemyFile);
        if (!loaded)
        {
            return std::nullopt;
        }
        enemyTextures[i] = loaded->Value;
    }

    std::vector<EnemyTemplate> enemies;
    enemies.emplace_back(TextureId(enemyTextures[0]), Vector2u(64, 64), 4, &cutEnemyTexture<4, 3>);
    enemies.emplace_back(TextureId(enemyTextures[1]), Vector2u(32, 32), 2, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(TextureId(enemyTextures[2]), Vector2u(64, 64), 18, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(TextureId(enemyTextures[3]), Vector2u(64, 64), 17, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(TextureId(enemyTextures[4]), Vector2u(64, 64), 13, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(TextureId(enemyTextures[5]), Vector2u(128, 128), 28, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(TextureId(enemyTextures[6]), Vector2u(64, 64), 10, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(TextureId(enemyTextures[7]), Vector2u(64, 64), 20, &cutEnemyTexture<3, 3>);
    enemies.emplace_back(TextureId(enemyTextures[8]), Vector2u(64, 64), 19, &cutEnemyTexture<3, 7>);
    enemies.emplace_back(TextureId(enemyTextures[9]), Vector2u(64, 64), 18, &cutEnemyTexture<4, 3>);
    return enemies;
}

void ij::SpawnEnemies(World &world, const size_t numberOfEnemies, const std::vector<EnemyTemplate> &enemies,
                      RandomNumberGenerator &randomNumberGenerator)
{
    for (const EnemyTemplate &enemyTemplate : enemies)
    {
        for (size_t k = 0; k < (numberOfEnemies / enemies.size()); ++k)
        {
            const Vector2f position = GenerateRandomPointForSpawning(world, randomNumberGenerator);
            const Vector2f direction =
                DirectionToVector(AssertCast<Direction>(randomNumberGenerator.GenerateInt32(0, 3)));
            world.enemies.emplace_back(
                VisualEntity(enemyTemplate.Texture, enemyTemplate.Size, enemyTemplate.VerticalOffset,
                             TimeSpan::FromMilliseconds(0), enemyTemplate.Cutter, ObjectAnimation::Standing),
                LogicEntity(
                    std::make_unique<Bot>(), position, direction, true, false, 100, 100, ObjectActivity::Standing));
        }
    }
}
