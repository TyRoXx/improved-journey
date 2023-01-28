#pragma once
#include <SFML/Config.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
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
        virtual ~ObjectBehavior()
        {
        }

        virtual void update(LogicEntity &object, LogicEntity &player, World &world, const sf::Time &deltaTime,
                            RandomNumberGenerator &random) = 0;
    };

    using Health = sf::Int32;

    struct LogicEntity
    {
        ObjectActivity GetActivity() const
        {
            return Activity;
        }

        void SetActivity(const ObjectActivity activity)
        {
            if (isDead(*this))
            {
                Activity = ObjectActivity::Dead;
                return;
            }
            Activity = activity;
        }

        [[nodiscard]] bool inflictDamage(const Health damage)
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

        Health GetCurrentHealth() const
        {
            return currentHealth;
        }

        Health GetMaximumHealth() const
        {
            return maximumHealth;
        }

        std::unique_ptr<ObjectBehavior> Behavior;
        sf::Vector2f Position;
        sf::Vector2f Direction;
        bool HasCollisionWithWalls = true;
        bool HasBumpedIntoWall = false;

    private:
        Health currentHealth = 100;
        Health maximumHealth = 100;
        ObjectActivity Activity = ObjectActivity::Standing;
    };
} // namespace ij
