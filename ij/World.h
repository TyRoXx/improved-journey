#pragma once
#include "FloatingText.h"
#include "LogicEntity.h"
#include "Map.h"
#include "VisualEntity.h"
#include <fmt/format.h>
#include <vector>

namespace ij
{
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

    [[nodiscard]] Object *FindEnemyByPosition(World &world, const sf::Vector2f &position);
    void InflictDamage(LogicEntity &damaged, World &world, const Health damage, RandomNumberGenerator &random);
} // namespace ij
