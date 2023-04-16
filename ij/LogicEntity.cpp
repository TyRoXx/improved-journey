#include "LogicEntity.h"
#include "World.h"
#include <cmath>

bool ij::isDead(const LogicEntity &entity)
{
    return (entity.GetCurrentHealth() == 0);
}

ij::ObjectBehavior::~ObjectBehavior() = default;

ij::LogicEntity::LogicEntity(std::unique_ptr<ObjectBehavior> behavior, const Vector2f &position,
                             const Vector2f &direction, bool hasCollisionWithWalls, bool hasBumpedIntoWall,
                             Health currentHealth, Health maximumHealth, ObjectActivity activity)
    : Behavior(std::move(behavior))
    , Position(position)
    , Direction(direction)
    , HasCollisionWithWalls(hasCollisionWithWalls)
    , HasBumpedIntoWall(hasBumpedIntoWall)
    , currentHealth(currentHealth)
    , maximumHealth(maximumHealth)
    , Activity(activity)
{
}

ij::ObjectActivity ij::LogicEntity::GetActivity() const
{
    return Activity;
}

void ij::LogicEntity::SetActivity(const ObjectActivity activity)
{
    if (isDead(*this))
    {
        Activity = ObjectActivity::Dead;
        return;
    }
    Activity = activity;
}

bool ij::LogicEntity::inflictDamage(const Health damage)
{
    if (currentHealth == 0)
    {
        return false;
    }
    currentHealth -= damage;
    if (currentHealth < 0)
    {
        currentHealth = 0;
    }
    if (isDead(*this))
    {
        SetActivity(ObjectActivity::Dead);
    }
    return true;
}

ij::Health ij::LogicEntity::GetCurrentHealth() const
{
    return currentHealth;
}

ij::Health ij::LogicEntity::GetMaximumHealth() const
{
    return maximumHealth;
}

[[nodiscard]] bool ij::IsWalkablePoint(const Vector2f &point, const World &world)
{
    const Vector2<ptrdiff_t> tileIndex(
        AssertCast<ptrdiff_t>(std::floor(point.x / TileSize)), AssertCast<ptrdiff_t>(std::floor(point.y / TileSize)));
    if ((tileIndex.x < 0) || (tileIndex.y < 0))
    {
        return false;
    }
    if ((tileIndex.x >= AssertCast<ptrdiff_t>(world.map.Width)) ||
        (tileIndex.y >= AssertCast<ptrdiff_t>(world.map.GetHeight())))
    {
        return false;
    }
    const int tile = world.map.GetTileAt(AssertCast<size_t>(tileIndex.x), AssertCast<size_t>(tileIndex.y));
    return (tile != NoTile);
}

[[nodiscard]] bool ij::IsWalkable(const Vector2f &point, const Vector2f &entityDimensions, const World &world)
{
    const Vector2f halfDimensions = entityDimensions / 2.0f;
    // current heuristic: check the four corners of the bounding box around the entity in addition to the center
    return
        // top left
        IsWalkablePoint(point - halfDimensions, world) &&
        // bottom right
        IsWalkablePoint(point + halfDimensions, world) &&
        // bottom left
        IsWalkablePoint(point + Vector2f(-halfDimensions.x, halfDimensions.y), world) &&
        // top right
        IsWalkablePoint(point + Vector2f(halfDimensions.x, -halfDimensions.y), world) &&
        // center
        IsWalkablePoint(point, world);
}

void ij::MoveWithCollisionDetection(LogicEntity &entity, const Vector2f &desiredChange, const World &world)
{
    const Vector2f desiredDestination = (entity.Position + desiredChange);
    if (!entity.HasCollisionWithWalls || IsWalkable(desiredDestination, DefaultEntityDimensions, world))
    {
        entity.Position = desiredDestination;
        entity.HasBumpedIntoWall = false;
    }
    else if (const Vector2f horizontalAlternative = (entity.Position + Vector2f(desiredChange.x, 0));
             IsWalkable(horizontalAlternative, DefaultEntityDimensions, world))
    {
        entity.Position = horizontalAlternative;
        entity.HasBumpedIntoWall = true;
    }
    else if (const Vector2f verticalAlternative = (entity.Position + Vector2f(0, desiredChange.y));
             IsWalkable(verticalAlternative, DefaultEntityDimensions, world))
    {
        entity.Position = verticalAlternative;
        entity.HasBumpedIntoWall = true;
    }
    else
    {
        entity.HasBumpedIntoWall = true;
    }
}

void ij::updateLogic(LogicEntity &entity, LogicEntity &player, World &world, const TimeSpan deltaTime,
                     RandomNumberGenerator &random)
{
    entity.Behavior->update(entity, player, world, deltaTime, random);

    switch (entity.GetActivity())
    {
    case ObjectActivity::Standing:
    case ObjectActivity::Attacking:
    case ObjectActivity::Dead:
        break;

    case ObjectActivity::Walking: {
        constexpr float velocity = 0.08f;
        const Vector2f change = entity.Direction * (AssertCast<float>(deltaTime.Milliseconds) * velocity);
        MoveWithCollisionDetection(entity, change, world);
        break;
    }
    }
}
