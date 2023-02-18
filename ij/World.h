#pragma once
#include "FloatingText.h"
#include "LogicEntity.h"
#include "Map.h"
#include "VisualEntity.h"
#include <vector>

namespace ij
{
    constexpr unsigned FrameRate = 60;

    struct Object final
    {
        VisualEntity Visuals;
        LogicEntity Logic;

        Object(VisualEntity visuals, LogicEntity logic);
    };

    struct World final
    {
        std::vector<Object> enemies;
        std::vector<FloatingText> FloatingTexts;
        const sf::Font &Font;
        const Map &map;

        explicit World(const sf::Font &font, const Map &map);
    };

    [[nodiscard]] Object *FindEnemyByPosition(World &world, const Vector2f &position);
    [[nodiscard]] std::vector<Object *> FindEnemiesInCircle(World &world, const Vector2f &center, float radius);
    void InflictDamage(LogicEntity &damaged, World &world, Health damage, RandomNumberGenerator &random);
    [[nodiscard]] bool isWithinDistance(const Vector2f &first, const Vector2f &second, float distance);
    [[nodiscard]] Vector2f GenerateRandomPointForSpawning(const World &world,
                                                          RandomNumberGenerator &randomNumberGenerator);
    void UpdateWorld(sf::Time &remainingSimulationTime, LogicEntity &player, World &world,
                     RandomNumberGenerator &randomNumberGenerator);
} // namespace ij
