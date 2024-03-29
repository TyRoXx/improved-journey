#include "World.h"
#include <fmt/format.h>

ij::Object::Object(VisualEntity visuals, LogicEntity logic)
    : Visuals(std::move(visuals))
    , Logic(std::move(logic))
{
}

ij::World::World(FontId font, const Map &map, Canvas &visualCanvas)
    : Font(font)
    , map(map)
    , VisualCanvas(visualCanvas)
{
}

[[nodiscard]] ij::Object *ij::FindEnemyByPosition(World &world, const Vector2f &position)
{
    for (Object &enemy : world.enemies)
    {
        const Vector2f topLeft = enemy.Visuals.GetTopLeftPosition(enemy.Logic.Position);
        const Vector2f bottomRight = topLeft + AssertCastVector<float>(enemy.Visuals.SpriteSize);
        if ((position.x >= topLeft.x) && (position.x <= bottomRight.x) && (position.y >= topLeft.y) &&
            (position.y <= bottomRight.y))
        {
            return &enemy;
        }
    }
    return nullptr;
}

std::vector<ij::Object *> ij::FindEnemiesInCircle(World &world, const Vector2f &center, float radius)
{
    std::vector<Object *> results;
    for (Object &enemy : world.enemies)
    {
        if (isWithinDistance(center, enemy.Logic.Position, radius))
        {
            results.emplace_back(&enemy);
        }
    }
    return results;
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
    world.FloatingTexts.emplace_back(
        world.VisualCanvas, fmt::format("{}", damage), damaged.Position, world.Font, random);
}

bool ij::isWithinDistance(const Vector2f &first, const Vector2f &second, const float distance)
{
    const float xDiff = (first.x - second.x);
    const float yDiff = (first.y - second.y);
    return (distance * distance) >= ((xDiff * xDiff) + (yDiff * yDiff));
}

ij::Vector2f ij::GenerateRandomPointForSpawning(const World &world, RandomNumberGenerator &randomNumberGenerator)
{
    Vector2f position(0, 0);
    size_t attempt = 0;
    do
    {
        ++attempt;
        assert(attempt < 100);
        position.x = AssertCast<float>(
            TileSize * randomNumberGenerator.GenerateInt32(0, AssertCast<Int32>(world.map.Width - 1)) + (TileSize / 2));
        position.y = AssertCast<float>(
            TileSize * randomNumberGenerator.GenerateInt32(0, AssertCast<Int32>(world.map.GetHeight() - 1)) +
            (TileSize / 2));
    } while (!IsWalkable(position, DefaultEntityDimensions, world));
    return position;
}

void ij::UpdateWorld(TimeSpan &remainingSimulationTime, LogicEntity &player, World &world,
                     RandomNumberGenerator &randomNumberGenerator)
{
    const TimeSpan simulationTimeStep = TimeSpan::FromMilliseconds(AssertCast<Int64>(1000 / FrameRate));
    while (remainingSimulationTime >= simulationTimeStep)
    {
        remainingSimulationTime -= simulationTimeStep;
        updateLogic(player, player, world, simulationTimeStep, randomNumberGenerator);
        for (Object &enemy : world.enemies)
        {
            updateLogic(enemy.Logic, player, world, simulationTimeStep, randomNumberGenerator);
        }
    }
}
