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
