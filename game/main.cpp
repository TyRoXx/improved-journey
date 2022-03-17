#include "imgui-SFML.h"
#include "imgui.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <array>
#include <filesystem>
#include <iostream>

#define IJ_UNREACHABLE() __assume(false)

enum class Direction
{
    Up,
    Left,
    Down,
    Right
};

sf::Vector2f DirectionToVector(Direction direction)
{
    switch (direction)
    {
    case Direction::Up:
        return sf::Vector2f(0, -1);
    case Direction::Left:
        return sf::Vector2f(-1, 0);
    case Direction::Down:
        return sf::Vector2f(0, 1);
    case Direction::Right:
        return sf::Vector2f(1, 0);
    }
    IJ_UNREACHABLE();
}

Direction DirectionFromVector(const sf::Vector2f &vector)
{
    if (vector.x >= 0)
    {
        if (vector.y >= vector.x)
        {
            return Direction::Down;
        }
        if (vector.y <= -vector.x)
        {
            return Direction::Up;
        }
        return Direction::Right;
    }
    if (vector.y >= -vector.x)
    {
        return Direction::Down;
    }
    if (vector.y <= vector.x)
    {
        return Direction::Up;
    }
    return Direction::Left;
}

enum class ObjectAnimation
{
    Standing,
    Walking
};

using TextureCutter = sf::IntRect(ObjectAnimation animation, sf::Int32 animationTime, Direction direction,
                                  const sf::Vector2i &size);

sf::IntRect cutEnemyTexture(const ObjectAnimation animation, const sf::Int32 animationTime, const Direction direction,
                            const sf::Vector2i &size)
{
    sf::Int32 speed = 150;
    switch (animation)
    {
    case ObjectAnimation::Standing:
        speed = 300;
        break;
    case ObjectAnimation::Walking:
        break;
    }
    return sf::IntRect((size.x * ((animationTime / speed) % 3)), size.y * static_cast<int>(direction), size.x, size.y);
};

sf::Vector2f normalize(const sf::Vector2f &source)
{
    float length = sqrt((source.x * source.x) + (source.y * source.y));
    if (length == 0)
    {
        return source;
    }
    return sf::Vector2f(source.x / length, source.y / length);
}

struct ObjectBehavior;

using Health = sf::Int32;

struct Object
{
    sf::Sprite Sprite;
    sf::Int32 AnimationTime = 0;
    sf::Vector2f Position;
    sf::Vector2f Direction;
    TextureCutter *Cutter = nullptr;
    sf::Vector2i SpriteSize;
    sf::Int32 VerticalOffset = 0;
    std::unique_ptr<ObjectBehavior> Behavior;
    ObjectAnimation Animation = ObjectAnimation::Standing;
    Health currentHealth = 1;
    Health maximumHealth = 1;
};

void inflictDamage(Object &defender, const Health damage)
{
    defender.currentHealth -= damage;
    if (defender.currentHealth < 0)
    {
        defender.currentHealth = 0;
    }
}

bool isDead(const Object &object)
{
    return (object.currentHealth == 0);
}

struct ObjectBehavior
{
    virtual ~ObjectBehavior()
    {
    }

    virtual void update(Object &object, Object &player, const sf::Time &deltaTime) = 0;
};

struct PlayerCharacter final : ObjectBehavior
{
    const std::array<bool, 4> &isDirectionKeyPressed;

    explicit PlayerCharacter(const std::array<bool, 4> &isDirectionKeyPressed)
        : isDirectionKeyPressed(isDirectionKeyPressed)
    {
    }

    virtual void update(Object &object, Object &player, const sf::Time &deltaTime) final
    {
        assert(&object == &player);
        (void)player;
        (void)deltaTime;

        if (isDead(object))
        {
            object.Animation = ObjectAnimation::Standing;
            return;
        }

        sf::Vector2f direction;
        for (sf::Int32 i = 0; i < 4; ++i)
        {
            if (!isDirectionKeyPressed[i])
            {
                continue;
            }
            direction += DirectionToVector(static_cast<Direction>(i));
        }
        if (direction == sf::Vector2f())
        {
            object.Animation = ObjectAnimation::Standing;
        }
        else
        {
            object.Animation = ObjectAnimation::Walking;
            object.Direction = normalize(direction);
        }
    }
};

bool isWithinDistance(const sf::Vector2f &first, const sf::Vector2f &second, const float distance)
{
    const float xDiff = (first.x - second.x);
    const float yDiff = (first.y - second.y);
    return (distance * distance) >= ((xDiff * xDiff) + (yDiff * yDiff));
}

struct Bot final : ObjectBehavior
{
    virtual void update(Object &object, Object &player, const sf::Time &deltaTime) final
    {
        switch (_state)
        {
        case State::MovingAround:
            if (isWithinDistance(object.Position, player.Position, 400) && !isDead(player))
            {
                _state = State::Chasing;
                _target = &player;
                break;
            }
            if (std::rand() % 2000 < 10)
            {
                switch (object.Animation)
                {
                case ObjectAnimation::Standing:
                    object.Animation = ObjectAnimation::Walking;
                    break;
                case ObjectAnimation::Walking:
                    object.Animation = ObjectAnimation::Standing;
                    break;
                }
                object.Direction = normalize(
                    sf::Vector2f(static_cast<float>(std::rand() % 10 - 5), static_cast<float>(std::rand() % 10 - 5)));
            }
            break;

        case State::Chasing:
            if (isWithinDistance(object.Position, _target->Position, 80))
            {
                _state = State::Attacking;
                break;
            }
            if (isWithinDistance(object.Position, _target->Position, 600))
            {
                object.Animation = ObjectAnimation::Walking;
                object.Direction = normalize(_target->Position - object.Position);
            }
            else
            {
                object.Animation = ObjectAnimation::Standing;
                _state = State::MovingAround;
                _target = nullptr;
            }
            break;

        case State::Attacking:
            object.Animation = ObjectAnimation::Standing;
            if (!isWithinDistance(object.Position, _target->Position, 100))
            {
                _state = State::Chasing;
                break;
            }
            _sinceLastAttack += deltaTime.asMilliseconds();
            constexpr sf::Int32 attackDelay = 4000;
            while (_sinceLastAttack >= attackDelay)
            {
                inflictDamage(player, 5);
                _sinceLastAttack -= attackDelay;
            }
            if (isDead(player))
            {
                _state = State::MovingAround;
            }
            break;
        }
    }

private:
    enum class State
    {
        MovingAround,
        Chasing,
        Attacking
    };

    State _state = State::MovingAround;
    Object *_target = nullptr;
    sf::Int32 _sinceLastAttack = 0;
};

struct Camera
{
    sf::Vector2f Center;

    void draw(sf::RenderWindow &window, const sf::Sprite &sprite)
    {
        sf::Sprite moved(sprite);
        moved.move(-Center);
        moved.move(sf::Vector2f(window.getSize()) * 0.5f);
        window.draw(moved);
    }

    void draw(sf::RenderWindow &window, const sf::CircleShape &shape)
    {
        sf::CircleShape moved(shape);
        moved.move(-Center);
        moved.move(sf::Vector2f(window.getSize()) * 0.5f);
        window.draw(moved);
    }

    void draw(sf::RenderWindow &window, const sf::RectangleShape &shape)
    {
        sf::RectangleShape moved(shape);
        moved.move(-Center);
        moved.move(sf::Vector2f(window.getSize()) * 0.5f);
        window.draw(moved);
    }
};

void updateObject(Object &object, Object &player, const sf::Time &deltaTime)
{
    object.Behavior->update(object, player, deltaTime);

    switch (object.Animation)
    {
    case ObjectAnimation::Standing:
        break;

    case ObjectAnimation::Walking: {
        const float velocity = 120;
        const auto change = object.Direction * deltaTime.asSeconds() * velocity;
        object.Position.x += change.x;
        object.Position.y += change.y;
        break;
    }
    }

    object.AnimationTime += deltaTime.asMilliseconds();
    object.Sprite.setTextureRect(object.Cutter(
        object.Animation, object.AnimationTime, DirectionFromVector(object.Direction), object.SpriteSize));
    // the position of an object is at the bottom center of the sprite (on the ground)
    object.Sprite.setPosition(
        object.Position -
        sf::Vector2f(static_cast<float>(object.SpriteSize.x / 2), static_cast<float>(object.SpriteSize.y)) +
        sf::Vector2f(0, static_cast<float>(object.VerticalOffset)));
}

float bottomOfSprite(const sf::Sprite &sprite)
{
    return (sprite.getPosition().y + static_cast<float>(sprite.getTextureRect().height));
}

void drawHealthBar(sf::RenderWindow &window, Camera &camera, const Object &object)
{
    if (object.currentHealth == object.maximumHealth)
    {
        return;
    }
    constexpr sf::Int32 width = 24;
    constexpr sf::Int32 height = 4;
    const float x = object.Position.x - width / 2;
    const float y = object.Position.y - static_cast<float>(object.Sprite.getTextureRect().height);
    const float greenPortion =
        static_cast<float>(object.currentHealth) / static_cast<float>(object.maximumHealth) * static_cast<float>(width);
    {
        sf::RectangleShape green;
        green.setPosition(sf::Vector2f(x, y));
        green.setFillColor(sf::Color::Green);
        green.setSize(sf::Vector2f(greenPortion, height));
        camera.draw(window, green);
    }
    {
        sf::RectangleShape red;
        red.setPosition(sf::Vector2f(x + greenPortion, y));
        red.setFillColor(sf::Color::Red);
        red.setSize(sf::Vector2f(width - greenPortion, height));
        camera.draw(window, red);
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(1200, 800), "Improved Journey");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    const auto assets = std::filesystem::current_path().parent_path().parent_path() / "improved-journey" / "assets";
    const auto wolfsheet1File = (assets / "LPC Wolfman" / "Male" / "Gray" / "Universal.png");
    assert(std::filesystem::exists(wolfsheet1File));

    sf::Texture wolfsheet1Texture;
    if (!wolfsheet1Texture.loadFromFile(wolfsheet1File.string()))
    {
        return 1;
    }

    TextureCutter *const wolfCutter = [](const ObjectAnimation animation, const sf::Int32 animationTime,
                                         const Direction direction, const sf::Vector2i &size) {
        sf::Int32 x = 0;
        switch (animation)
        {
        case ObjectAnimation::Standing:
            break;

        case ObjectAnimation::Walking:
            x = (size.x * ((animationTime / 80) % 9));
            break;
        }
        return sf::IntRect(x, size.y * (static_cast<int>(direction) + 8), size.x, size.y);
    };

    const auto grassFile = (assets / "LPC Base Assets" / "tiles" / "grass.png");
    sf::Texture grassTexture;
    if (!grassTexture.loadFromFile(grassFile.string()))
    {
        return 1;
    }

    std::array<sf::Texture, 10> enemyTextures = {};
    const std::array<const char *, 10> enemyFileNames = {
        "bat", "bee", "big_worm", "eyeball", "ghost", "man_eater_flower", "pumpking", "slime", "small_worm", "snake"};
    const std::array<sf::Vector2i, 10> enemySizes = {
        sf::Vector2i(64, 64),   sf::Vector2i(32, 32), sf::Vector2i(64, 64), sf::Vector2i(64, 64), sf::Vector2i(64, 64),
        sf::Vector2i(128, 128), sf::Vector2i(64, 64), sf::Vector2i(64, 64), sf::Vector2i(64, 64), sf::Vector2i(64, 64)};
    const std::array<int, 10> enemyVerticalOffset = {4, 2, 18, 17, 13, 28, 10, 20, 19, 18};
    for (size_t i = 0; i < enemyFileNames.size(); ++i)
    {
        const auto enemyFile = (assets / "lpc-monsters" / (std::string(enemyFileNames[i]) + ".png"));
        if (!enemyTextures[i].loadFromFile(enemyFile.string()))
        {
            return 1;
        }
    }

    std::array<bool, 4> isDirectionKeyPressed = {};

    Object player;
    player.Sprite.setTexture(wolfsheet1Texture);
    player.Cutter = wolfCutter;
    player.SpriteSize = sf::Vector2i(64, 64);
    player.Position = sf::Vector2f(400, 400);
    player.Behavior = std::make_unique<PlayerCharacter>(isDirectionKeyPressed);
    player.currentHealth = player.maximumHealth = 100;

    std::vector<Object> enemies;
    for (size_t i = 0; i < enemyFileNames.size(); ++i)
    {
        Object &enemy = enemies.emplace_back();
        enemy.Sprite.setTexture(enemyTextures[i]);
        enemy.Cutter = cutEnemyTexture;
        enemy.SpriteSize = enemySizes[i];
        enemy.Position.x = static_cast<float>(std::rand() % 1200);
        enemy.Position.y = static_cast<float>(std::rand() % 800);
        enemy.Direction = DirectionToVector(static_cast<Direction>(std::rand() % 4));
        enemy.VerticalOffset = enemyVerticalOffset[i];
        enemy.Behavior = std::make_unique<Bot>();
    }

    Camera camera{player.Position};

    sf::Clock deltaClock;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::W:
                    isDirectionKeyPressed[static_cast<size_t>(Direction::Up)] = true;
                    break;
                case sf::Keyboard::A:
                    isDirectionKeyPressed[static_cast<size_t>(Direction::Left)] = true;
                    break;
                case sf::Keyboard::S:
                    isDirectionKeyPressed[static_cast<size_t>(Direction::Down)] = true;
                    break;
                case sf::Keyboard::D:
                    isDirectionKeyPressed[static_cast<size_t>(Direction::Right)] = true;
                    break;
                default:
                    break;
                }
            }
            else if (event.type == sf::Event::KeyReleased)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::W:
                    isDirectionKeyPressed[static_cast<size_t>(Direction::Up)] = false;
                    break;
                case sf::Keyboard::A:
                    isDirectionKeyPressed[static_cast<size_t>(Direction::Left)] = false;
                    break;
                case sf::Keyboard::S:
                    isDirectionKeyPressed[static_cast<size_t>(Direction::Down)] = false;
                    break;
                case sf::Keyboard::D:
                    isDirectionKeyPressed[static_cast<size_t>(Direction::Right)] = false;
                    break;
                default:
                    break;
                }
            }
        }

        const sf::Time deltaTime = deltaClock.restart();
        ImGui::SFML::Update(window, deltaTime);
        window.clear();

        for (sf::Int32 y = 0; y < 30; ++y)
        {
            for (sf::Int32 x = 0; x < 30; ++x)
            {
                sf::Sprite grass(grassTexture);
                grass.setTextureRect(sf::IntRect(((x + y) % 3) * 32, 160, 32, 32));
                grass.setPosition(sf::Vector2f(static_cast<float>(x) * 32.0f, static_cast<float>(y) * 32.0f));
                camera.draw(window, grass);
            }
        }

        std::vector<const sf::Sprite *> spritesToDrawInZOrder;

        updateObject(player, player, deltaTime);
        camera.Center = player.Position;
        spritesToDrawInZOrder.emplace_back(&player.Sprite);

        for (Object &enemy : enemies)
        {
            updateObject(enemy, player, deltaTime);
            spritesToDrawInZOrder.emplace_back(&enemy.Sprite);
        }

        std::ranges::sort(
            spritesToDrawInZOrder, [](const sf::Sprite *const left, const sf::Sprite *const right) -> bool {
                return (bottomOfSprite(*left) < bottomOfSprite(*right));
            });
        for (const sf::Sprite *const sprite : spritesToDrawInZOrder)
        {
            camera.draw(window, *sprite);
        }

        drawHealthBar(window, camera, player);
        for (const Object &enemy : enemies)
        {
            drawHealthBar(window, camera, enemy);
        }

        {
            sf::CircleShape circle(1);
            circle.setOutlineColor(sf::Color(0, 255, 0));
            circle.setFillColor(sf::Color(0, 255, 0));
            circle.setPosition(player.Position);
            camera.draw(window, circle);
        }
        for (const Object &enemy : enemies)
        {
            sf::CircleShape circle(1);
            circle.setOutlineColor(sf::Color(255, 0, 0));
            circle.setFillColor(sf::Color(255, 0, 0));
            circle.setPosition(enemy.Position);
            camera.draw(window, circle);
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}
