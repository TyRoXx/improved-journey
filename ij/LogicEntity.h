#pragma once
#include "Int.h"
#include "Vector2.h"
#include <SFML/System/Time.hpp>
#include <memory>

namespace ij
{
    enum class ObjectActivity
    {
        Standing,
        Walking,
        Attacking,
        Dead
    };

    struct LogicEntity;

    bool isDead(const LogicEntity &entity);

    struct World;
    struct RandomNumberGenerator;

    struct ObjectBehavior
    {
        virtual ~ObjectBehavior();
        virtual void update(LogicEntity &object, LogicEntity &player, World &world, const sf::Time &deltaTime,
                            RandomNumberGenerator &random) = 0;
    };

    using Health = Int32;

    struct LogicEntity final
    {
        LogicEntity(std::unique_ptr<ObjectBehavior> behavior, const Vector2f &position, const Vector2f &direction,
                    bool hasCollisionWithWalls, bool hasBumpedIntoWall, Health currentHealth, Health maximumHealth,
                    ObjectActivity activity);
        [[nodiscard]] ObjectActivity GetActivity() const;
        void SetActivity(ObjectActivity activity);
        [[nodiscard]] bool inflictDamage(Health damage);
        [[nodiscard]] Health GetCurrentHealth() const;
        [[nodiscard]] Health GetMaximumHealth() const;

        std::unique_ptr<ObjectBehavior> Behavior;
        Vector2f Position;
        Vector2f Direction;
        bool HasCollisionWithWalls = true;
        bool HasBumpedIntoWall = false;

    private:
        Health currentHealth = 100;
        Health maximumHealth = 100;
        ObjectActivity Activity = ObjectActivity::Standing;
    };

    constexpr int TileSize = 32;
    const Vector2f DefaultEntityDimensions(8, 8);

    [[nodiscard]] bool IsWalkablePoint(const Vector2f &point, const World &world);
    [[nodiscard]] bool IsWalkable(const Vector2f &point, const Vector2f &entityDimensions, const World &world);
    void MoveWithCollisionDetection(LogicEntity &entity, const Vector2f &desiredChange, const World &world);
    void updateLogic(LogicEntity &entity, LogicEntity &player, World &world, const sf::Time &deltaTime,
                     RandomNumberGenerator &random);
} // namespace ij
