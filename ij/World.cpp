#include "World.h"

[[nodiscard]] ij::Object *ij::FindEnemyByPosition(World &world, const sf::Vector2f &position)
{
    for (Object &enemy : world.enemies)
    {
        const sf::Vector2f topLeft = enemy.Logic.Position - enemy.Visuals.GetOffset();
        const sf::Vector2f bottomRight = topLeft + sf::Vector2f(enemy.Visuals.SpriteSize);
        if ((position.x >= topLeft.x) && (position.x <= bottomRight.x) && (position.y >= topLeft.y) &&
            (position.y <= bottomRight.y))
        {
            return &enemy;
        }
    }
    return nullptr;
}

namespace ij
{
    template <class T>
    void EraseRandomElementUnstable(std::vector<T> &container, RandomNumberGenerator &random)
    {
        if (container.empty())
        {
            return;
        }
        const size_t erased = random.GenerateSize(0, container.size() - 1);
        container[erased] = std::move(container.back());
        container.pop_back();
    }
} // namespace ij

void ij::InflictDamage(LogicEntity &damaged, World &world, const Health damage, RandomNumberGenerator &random)
{
    if (!damaged.inflictDamage(damage))
    {
        return;
    }
    constexpr size_t floatingTextLimit = 1000;
    while (world.FloatingTexts.size() >= floatingTextLimit)
    {
        EraseRandomElementUnstable(world.FloatingTexts, random);
    }
    world.FloatingTexts.emplace_back(fmt::format("{}", damage), damaged.Position, world.Font, random);
}
