#include "imgui-SFML.h"
#include "imgui.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <array>
#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <random>

template <class To, class From>
To AssertCast(const From &from)
{
    assert(from == static_cast<From>(static_cast<To>(from)));
    return static_cast<To>(from);
}

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
    Walking,
    Attacking,
    Dead
};

[[nodiscard]] const char *GetObjectAnimationName(const ObjectAnimation animation)
{
    switch (animation)
    {
    case ObjectAnimation::Standing:
        return "Standing";
    case ObjectAnimation::Walking:
        return "Walking";
    case ObjectAnimation::Attacking:
        return "Attacking";
    case ObjectAnimation::Dead:
        return "Dead";
    }
    IJ_UNREACHABLE();
}

using TextureCutter = sf::IntRect(ObjectAnimation animation, sf::Int32 animationTime, Direction direction,
                                  const sf::Vector2i &size);

template <sf::Int32 WalkFrames, sf::Int32 AttackFrames>
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
    case ObjectAnimation::Attacking:
        return sf::IntRect(size.x * (((animationTime / 200) % AttackFrames) + WalkFrames),
                           size.y * AssertCast<int>(direction), size.x, size.y);
    case ObjectAnimation::Dead:
        return sf::IntRect(0, size.y * AssertCast<int>(direction), size.x, size.y);
    }
    return sf::IntRect(
        size.x * ((animationTime / 150) % WalkFrames), size.y * AssertCast<int>(direction), size.x, size.y);
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

struct VisualEntity
{
    sf::Sprite Sprite;
    sf::Vector2i SpriteSize;
    sf::Int32 VerticalOffset = 0;
    sf::Int32 AnimationTime = 0;
    TextureCutter *Cutter = nullptr;
    ObjectAnimation Animation = ObjectAnimation::Standing;

    sf::Vector2f GetOffset() const
    {
        return sf::Vector2f(AssertCast<float>(SpriteSize.x / 2), AssertCast<float>(SpriteSize.y)) -
               sf::Vector2f(0, AssertCast<float>(VerticalOffset));
    }
};

enum class ObjectActivity
{
    Standing,
    Walking,
    Attacking,
    Dead
};

struct LogicEntity;

bool isDead(const LogicEntity &entity);

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
    bool HasBumpedIntoWall = false;

private:
    Health currentHealth = 100;
    Health maximumHealth = 100;
    ObjectActivity Activity = ObjectActivity::Standing;
};

bool isDead(const LogicEntity &entity)
{
    return (entity.GetCurrentHealth() == 0);
}

struct Object final
{
    VisualEntity Visuals;
    LogicEntity Logic;
};

struct RandomNumberGenerator
{
    virtual ~RandomNumberGenerator()
    {
    }

    virtual sf::Int32 GenerateInt32(sf::Int32 minimum, sf::Int32 maximum) = 0;
};

struct StandardRandomNumberGenerator final : RandomNumberGenerator
{
    std::random_device seedGenerator;
    std::default_random_engine engine;

    StandardRandomNumberGenerator()
        : engine(seedGenerator())
    {
    }

    sf::Int32 GenerateInt32(sf::Int32 minimum, sf::Int32 maximum) override
    {
        std::uniform_int_distribution<sf::Int32> distribution(minimum, maximum);
        return distribution(engine);
    }
};

struct FloatingText final
{
    std::unique_ptr<sf::Text> Text;
    sf::Time Age;
    sf::Time MaxAge;

    explicit FloatingText(const sf::String &text, const sf::Vector2f &position, const sf::Font &font,
                          RandomNumberGenerator &random)
        : Text(std::make_unique<sf::Text>(text, font, 14u))
        , Age()
        , MaxAge(sf::milliseconds(random.GenerateInt32(5000, 10'000)))
    {
        Text->setPosition(position + sf::Vector2f(AssertCast<float>(20 - random.GenerateInt32(0, 39)),
                                                  AssertCast<float>(-100 + random.GenerateInt32(0, 39))));
        Text->setFillColor(sf::Color::Red);
        Text->setOutlineColor(sf::Color::Black);
        Text->setOutlineThickness(1);
    }

    void Update(const sf::Time &deltaTime)
    {
        Age += deltaTime;
        Text->setPosition(Text->getPosition() + sf::Vector2f(0, deltaTime.asSeconds() * -10));
    }

    bool HasExpired() const
    {
        return (Age >= MaxAge);
    }
};

constexpr int NoTile = 3;

struct Map final
{
    std::vector<int> Tiles;
    size_t Width;

    size_t GetHeight() const
    {
        return Tiles.size() / Width;
    }

    int GetTileAt(const size_t x, const size_t y) const
    {
        return Tiles[(y * Width) + x];
    }
};

[[nodiscard]] Map GenerateRandomMap(RandomNumberGenerator &random)
{
    Map result;
    result.Width = 50;
    for (size_t i = 0; i < (50 * result.Width); ++i)
    {
        result.Tiles.push_back(random.GenerateInt32(0, 3));
    }
    return result;
}

struct World final
{
    std::vector<Object> enemies;
    std::vector<FloatingText> FloatingTexts;
    const sf::Font &Font;
    const Map &map;

    explicit World(const sf::Font &font, const Map &map)
        : Font(font)
        , map(map)
    {
    }
};

[[nodiscard]] Object *FindEnemyByPosition(World &world, const sf::Vector2f &position)
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

void InflictDamage(LogicEntity &damaged, World &world, const Health damage, RandomNumberGenerator &random)
{
    if (!damaged.inflictDamage(damage))
    {
        return;
    }
    world.FloatingTexts.emplace_back(fmt::format("{}", damage), damaged.Position, world.Font, random);
}

struct ObjectBehavior
{
    virtual ~ObjectBehavior()
    {
    }

    virtual void update(LogicEntity &object, LogicEntity &player, World &world, const sf::Time &deltaTime,
                        RandomNumberGenerator &random) = 0;
};

struct PlayerCharacter final : ObjectBehavior
{
    const std::array<bool, 4> &isDirectionKeyPressed;
    bool &isAttackPressed;

    explicit PlayerCharacter(const std::array<bool, 4> &isDirectionKeyPressed, bool &isAttackPressed)
        : isDirectionKeyPressed(isDirectionKeyPressed)
        , isAttackPressed(isAttackPressed)
    {
    }

    virtual void update(LogicEntity &object, LogicEntity &player, World &world, const sf::Time &deltaTime,
                        RandomNumberGenerator &random) final
    {
        assert(&object == &player);
        (void)player;
        (void)deltaTime;

        sf::Vector2f direction;
        for (size_t i = 0; i < 4; ++i)
        {
            if (!isDirectionKeyPressed[i])
            {
                continue;
            }
            direction += DirectionToVector(AssertCast<Direction>(i));
        }
        if (direction == sf::Vector2f())
        {
            if (isAttackPressed && !isDead(object))
            {
                object.SetActivity(ObjectActivity::Attacking);
                for (Object &enemy : world.enemies)
                {
                    InflictDamage(enemy.Logic, world, 2, random);
                }
            }
            else
            {
                object.SetActivity(ObjectActivity::Standing);
            }
        }
        else
        {
            object.SetActivity(ObjectActivity::Walking);
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
    virtual void update(LogicEntity &object, LogicEntity &player, World &world, const sf::Time &deltaTime,
                        RandomNumberGenerator &random) final
    {
        (void)world;
        if (isDead(object))
        {
            return;
        }
        switch (_state)
        {
        case State::MovingAround:
            if (isWithinDistance(object.Position, player.Position, 400) && !isDead(player))
            {
                _state = State::Chasing;
                _target = &player;
                break;
            }
            if (object.HasBumpedIntoWall || (random.GenerateInt32(0, 1999) < 10))
            {
                switch (object.GetActivity())
                {
                case ObjectActivity::Standing:
                    object.SetActivity(ObjectActivity::Walking);
                    break;
                case ObjectActivity::Walking:
                case ObjectActivity::Attacking:
                case ObjectActivity::Dead:
                    object.SetActivity(ObjectActivity::Standing);
                    break;
                }
                object.Direction = normalize(sf::Vector2f(AssertCast<float>(random.GenerateInt32(0, 9) - 5),
                                                          AssertCast<float>(random.GenerateInt32(0, 9) - 5)));
            }
            break;

        case State::Chasing:
            assert(_target);
            if (isDead(*_target))
            {
                object.SetActivity(ObjectActivity::Standing);
                _state = State::MovingAround;
                _target = nullptr;
            }
            else if (isWithinDistance(object.Position, _target->Position, 40))
            {
                _state = State::Attacking;
                object.SetActivity(ObjectActivity::Standing);
            }
            else if (isWithinDistance(object.Position, _target->Position, 600))
            {
                object.SetActivity(ObjectActivity::Walking);
                object.Direction = normalize(_target->Position - object.Position);
            }
            else
            {
                object.SetActivity(ObjectActivity::Standing);
                _state = State::MovingAround;
                _target = nullptr;
            }
            break;

        case State::Attacking:
            if (!isWithinDistance(object.Position, _target->Position, 60))
            {
                _state = State::Chasing;
                object.SetActivity(ObjectActivity::Standing);
                break;
            }
            _sinceLastAttack += deltaTime.asMilliseconds();
            constexpr sf::Int32 attackDelay = 1000;
            while (_sinceLastAttack >= attackDelay)
            {
                object.SetActivity(ObjectActivity::Attacking);
                InflictDamage(player, world, 1, random);
                _sinceLastAttack -= attackDelay;
            }
            if (isDead(player))
            {
                _state = State::MovingAround;
                object.SetActivity(ObjectActivity::Standing);
            }
            break;
        }
        // acknowledged
        object.HasBumpedIntoWall = false;
    }

    enum class State
    {
        MovingAround,
        Chasing,
        Attacking
    };

    [[nodiscard]] State GetState() const
    {
        return _state;
    }

    [[nodiscard]] static const char *GetStateName(const State state)
    {
        switch (state)
        {
        case State::MovingAround:
            return "MovingAround";
        case State::Chasing:
            return "Chasing";
        case State::Attacking:
            return "Attacking";
        }
        IJ_UNREACHABLE();
    }

    [[nodiscard]] LogicEntity *GetTarget() const
    {
        return _target;
    }

private:
    State _state = State::MovingAround;
    LogicEntity *_target = nullptr;
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

    void draw(sf::RenderWindow &window, const sf::Text &text)
    {
        sf::Text moved(text);
        moved.move(-Center);
        moved.move(sf::Vector2f(window.getSize()) * 0.5f);
        window.draw(moved);
    }

    sf::Vector2f getWorldFromScreenCoordinates(const sf::RenderWindow &window, const sf::Vector2i &point)
    {
        return (sf::Vector2f(window.getSize()) * -0.5f) + Center + sf::Vector2f(point);
    }
};

constexpr int TileSize = 32;
const sf::Vector2f DefaultEntityDimensions(8, 8);

[[nodiscard]] bool IsWalkablePoint(const sf::Vector2f &point, const World &world)
{
    const sf::Vector2<ptrdiff_t> tileIndex(
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

[[nodiscard]] bool IsWalkable(const sf::Vector2f &point, const sf::Vector2f &entityDimensions, const World &world)
{
    const sf::Vector2f halfDimensions = entityDimensions / 2.0f;
    // current heuristic: check the four corners of the bounding box around the entity in addition to the center
    return
        // top left
        IsWalkablePoint(point - halfDimensions, world) &&
        // bottom right
        IsWalkablePoint(point + halfDimensions, world) &&
        // bottom left
        IsWalkablePoint(point + sf::Vector2f(-halfDimensions.x, halfDimensions.y), world) &&
        // top right
        IsWalkablePoint(point + sf::Vector2f(halfDimensions.x, -halfDimensions.y), world) &&
        // center
        IsWalkablePoint(point, world);
}

void MoveWithCollisionDetection(LogicEntity &entity, const sf::Vector2f &desiredChange, const World &world)
{
    const sf::Vector2f desiredDestination = (entity.Position + desiredChange);
    if (IsWalkable(desiredDestination, DefaultEntityDimensions, world))
    {
        entity.Position = desiredDestination;
        entity.HasBumpedIntoWall = false;
    }
    else if (const sf::Vector2f horizontalAlternative = (entity.Position + sf::Vector2f(desiredChange.x, 0));
             IsWalkable(horizontalAlternative, DefaultEntityDimensions, world))
    {
        entity.Position = horizontalAlternative;
        entity.HasBumpedIntoWall = true;
    }
    else if (const sf::Vector2f verticalAlternative = (entity.Position + sf::Vector2f(0, desiredChange.y));
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

void updateLogic(Object &object, LogicEntity &player, World &world, const sf::Time &deltaTime,
                 RandomNumberGenerator &random)
{
    object.Logic.Behavior->update(object.Logic, player, world, deltaTime, random);

    switch (object.Logic.GetActivity())
    {
    case ObjectActivity::Standing:
        break;

    case ObjectActivity::Attacking:
        break;

    case ObjectActivity::Dead:
        break;

    case ObjectActivity::Walking: {
        const float velocity = 80;
        const auto change = object.Logic.Direction * deltaTime.asSeconds() * velocity;
        MoveWithCollisionDetection(object.Logic, change, world);
        break;
    }
    }
}

void updateVisuals(const LogicEntity &logic, VisualEntity &visuals, const sf::Time &deltaTime)
{
    bool isColoredDead = false;
    switch (logic.GetActivity())
    {
    case ObjectActivity::Standing:
        visuals.Animation = ObjectAnimation::Standing;
        break;

    case ObjectActivity::Attacking:
        visuals.Animation = ObjectAnimation::Attacking;
        break;

    case ObjectActivity::Dead:
        visuals.Animation = ObjectAnimation::Dead;
        isColoredDead = true;
        break;

    case ObjectActivity::Walking: {
        visuals.Animation = ObjectAnimation::Walking;
        break;
    }
    }

    visuals.AnimationTime += deltaTime.asMilliseconds();
    visuals.Sprite.setTextureRect(visuals.Cutter(
        visuals.Animation, visuals.AnimationTime, DirectionFromVector(logic.Direction), visuals.SpriteSize));
    // the position of an object is at the bottom center of the sprite (on the ground)
    visuals.Sprite.setPosition(logic.Position - visuals.GetOffset());
    visuals.Sprite.setColor(isColoredDead ? sf::Color(128, 128, 128, 255) : sf::Color::White);
}

float bottomOfSprite(const sf::Sprite &sprite)
{
    return (sprite.getPosition().y + AssertCast<float>(sprite.getTextureRect().height));
}

void drawHealthBar(sf::RenderWindow &window, Camera &camera, const Object &object)
{
    if (object.Logic.GetCurrentHealth() == object.Logic.GetMaximumHealth())
    {
        return;
    }
    constexpr sf::Int32 width = 24;
    constexpr sf::Int32 height = 4;
    const float x = object.Logic.Position.x - width / 2;
    const float y = object.Logic.Position.y - AssertCast<float>(object.Visuals.Sprite.getTextureRect().height);
    const float greenPortion = AssertCast<float>(object.Logic.GetCurrentHealth()) /
                               AssertCast<float>(object.Logic.GetMaximumHealth()) * AssertCast<float>(width);
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

void loadAllSprites(const std::filesystem::path &assets)
{
    std::filesystem::directory_iterator i(assets);
    for (; i != std::filesystem::directory_iterator(); ++i)
    {
        const auto &entry = *i;
        if (entry.is_directory())
        {
            loadAllSprites(entry.path());
        }
        else if (entry.is_regular_file() && (entry.path().extension() == ".png"))
        {
            sf::Texture texture;
            if (!texture.loadFromFile(entry.path().string()))
            {
                std::cerr << "Could not load " << entry.path() << '\n';
            }
        }
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(1200, 800), "Improved Journey");
    const unsigned frameRate = 60;
    window.setFramerateLimit(frameRate);
    ImGui::SFML::Init(window);

    const auto assets = std::filesystem::current_path().parent_path().parent_path() / "improved-journey" / "assets";
    loadAllSprites(assets);

    sf::Font font;
    if (!font.loadFromFile((assets / "Roboto-Font" / "Roboto-Light.ttf").string()))
    {
        return 1;
    }

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
        sf::Int32 yOffset = 8;
        switch (animation)
        {
        case ObjectAnimation::Standing:
            break;

        case ObjectAnimation::Walking:
            x = (size.x * ((animationTime / 80) % 9));
            break;

        case ObjectAnimation::Attacking:
            yOffset = 0;
            x = (size.x * ((animationTime / 80) % 7));
            break;

        case ObjectAnimation::Dead:
            return sf::IntRect(5 * size.x, 20 * size.y, size.x, size.y);
        }
        return sf::IntRect(x, size.y * (AssertCast<int>(direction) + yOffset), size.x, size.y);
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
    const std::array<TextureCutter *, 10> enemyTextureCutters = {
        &cutEnemyTexture<4, 3>, &cutEnemyTexture<3, 3>, &cutEnemyTexture<3, 3>, &cutEnemyTexture<3, 3>,
        &cutEnemyTexture<3, 3>, &cutEnemyTexture<3, 3>, &cutEnemyTexture<3, 3>, &cutEnemyTexture<3, 3>,
        &cutEnemyTexture<3, 7>, &cutEnemyTexture<4, 3>};
    for (size_t i = 0; i < enemyFileNames.size(); ++i)
    {
        const auto enemyFile = (assets / "lpc-monsters" / (std::string(enemyFileNames[i]) + ".png"));
        if (!enemyTextures[i].loadFromFile(enemyFile.string()))
        {
            return 1;
        }
    }

    std::array<bool, 4> isDirectionKeyPressed = {};
    bool isAttackPressed = false;

    Object player;
    player.Visuals.Sprite.setTexture(wolfsheet1Texture);
    player.Visuals.Cutter = wolfCutter;
    player.Visuals.SpriteSize = sf::Vector2i(64, 64);
    player.Logic.Position = sf::Vector2f(400, 400);
    player.Logic.Behavior = std::make_unique<PlayerCharacter>(isDirectionKeyPressed, isAttackPressed);

    StandardRandomNumberGenerator randomNumberGenerator;
    const Map map = GenerateRandomMap(randomNumberGenerator);

    World world(font, map);
    for (size_t i = 0; i < enemyFileNames.size(); ++i)
    {
        for (size_t k = 0; k < 10; ++k)
        {
            Object &enemy = world.enemies.emplace_back();
            enemy.Visuals.Sprite.setTexture(enemyTextures[i]);
            enemy.Visuals.Cutter = enemyTextureCutters[i];
            enemy.Visuals.SpriteSize = enemySizes[i];
            enemy.Visuals.VerticalOffset = enemyVerticalOffset[i];
            do
            {
                enemy.Logic.Position.x = AssertCast<float>(
                    TileSize * randomNumberGenerator.GenerateInt32(0, AssertCast<sf::Int32>(map.Width - 1)) +
                    (TileSize / 2));
                enemy.Logic.Position.y = AssertCast<float>(
                    TileSize * randomNumberGenerator.GenerateInt32(0, AssertCast<sf::Int32>(map.GetHeight() - 1)) +
                    (TileSize / 2));
            } while (!IsWalkable(enemy.Logic.Position, DefaultEntityDimensions, world));
            enemy.Logic.Direction = DirectionToVector(AssertCast<Direction>(randomNumberGenerator.GenerateInt32(0, 3)));
            enemy.Logic.Behavior = std::make_unique<Bot>();
        }
    }

    Camera camera{player.Logic.Position};
    Object *selectedEnemy = nullptr;

    sf::Clock deltaClock;
    sf::Time remainingSimulationTime;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::W:
                    isDirectionKeyPressed[AssertCast<size_t>(Direction::Up)] = true;
                    break;
                case sf::Keyboard::A:
                    isDirectionKeyPressed[AssertCast<size_t>(Direction::Left)] = true;
                    break;
                case sf::Keyboard::S:
                    isDirectionKeyPressed[AssertCast<size_t>(Direction::Down)] = true;
                    break;
                case sf::Keyboard::D:
                    isDirectionKeyPressed[AssertCast<size_t>(Direction::Right)] = true;
                    break;
                case sf::Keyboard::Space:
                    isAttackPressed = true;
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
                    isDirectionKeyPressed[AssertCast<size_t>(Direction::Up)] = false;
                    break;
                case sf::Keyboard::A:
                    isDirectionKeyPressed[AssertCast<size_t>(Direction::Left)] = false;
                    break;
                case sf::Keyboard::S:
                    isDirectionKeyPressed[AssertCast<size_t>(Direction::Down)] = false;
                    break;
                case sf::Keyboard::D:
                    isDirectionKeyPressed[AssertCast<size_t>(Direction::Right)] = false;
                    break;
                case sf::Keyboard::Space:
                    isAttackPressed = false;
                    break;
                default:
                    break;
                }
            }
            else if (!ImGui::GetIO().WantCaptureMouse && (event.type == sf::Event::MouseButtonPressed) &&
                     (event.mouseButton.button == sf::Mouse::Button::Left))
            {
                const sf::Vector2f pointInWorld = camera.getWorldFromScreenCoordinates(
                    window, sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                selectedEnemy = FindEnemyByPosition(world, pointInWorld);
            }
        }

        const sf::Time deltaTime = deltaClock.restart();
        ImGui::SFML::Update(window, deltaTime);
        ImGui::Begin("Character");
        {
            ImGui::Text("Health");
            ImGui::SameLine();
            ImGui::ProgressBar(AssertCast<float>(player.Logic.GetCurrentHealth()) /
                               AssertCast<float>(player.Logic.GetMaximumHealth()));
        }
        ImGui::End();

        if (selectedEnemy)
        {
            ImGui::Begin("Enemy");
            {
                ImGui::Text("Health");
                ImGui::SameLine();
                ImGui::ProgressBar(AssertCast<float>(selectedEnemy->Logic.GetCurrentHealth()) /
                                   AssertCast<float>(selectedEnemy->Logic.GetMaximumHealth()));
                ImGui::BeginDisabled();
                ImGui::Checkbox("Bumped", &selectedEnemy->Logic.HasBumpedIntoWall);
                ImGui::EndDisabled();
                ImGui::LabelText("Animation", "%s", GetObjectAnimationName(selectedEnemy->Visuals.Animation));
                ImGui::LabelText(
                    "Direction", "%f %f", selectedEnemy->Logic.Direction.x, selectedEnemy->Logic.Direction.y);
                if (const Bot *const bot = dynamic_cast<const Bot *>(selectedEnemy->Logic.Behavior.get()))
                {
                    ImGui::LabelText("State", "%s", Bot::GetStateName(bot->GetState()));
                    ImGui::BeginDisabled();
                    bool hasTarget = (bot->GetTarget() != nullptr);
                    ImGui::Checkbox("Has target", &hasTarget);
                    ImGui::EndDisabled();
                }
            }
            ImGui::End();
        }

        window.clear();

        for (size_t y = 0, height = (map.Tiles.size() / map.Width); y < height; ++y)
        {
            for (size_t x = 0; x < map.Width; ++x)
            {
                const int tile = map.GetTileAt(x, y);
                if (tile == NoTile)
                {
                    continue;
                }
                sf::Sprite grass(grassTexture);
                grass.setTextureRect(sf::IntRect(tile * TileSize, 160, TileSize, TileSize));
                grass.setPosition(sf::Vector2f(AssertCast<float>(x) * TileSize, AssertCast<float>(y) * TileSize));
                camera.draw(window, grass);
            }
        }

        // fix the time step to make physics and NPC behaviour independent from the frame rate
        remainingSimulationTime += deltaTime;
        const sf::Time simulationTimeStep = sf::milliseconds(1000 / frameRate);
        while (remainingSimulationTime >= simulationTimeStep)
        {
            remainingSimulationTime -= simulationTimeStep;
            updateLogic(player, player.Logic, world, simulationTimeStep, randomNumberGenerator);
            for (Object &enemy : world.enemies)
            {
                updateLogic(enemy, player.Logic, world, simulationTimeStep, randomNumberGenerator);
            }
        }

        std::vector<const sf::Sprite *> spritesToDrawInZOrder;

        updateVisuals(player.Logic, player.Visuals, simulationTimeStep);
        camera.Center = player.Logic.Position;
        spritesToDrawInZOrder.emplace_back(&player.Visuals.Sprite);

        for (Object &enemy : world.enemies)
        {
            updateVisuals(enemy.Logic, enemy.Visuals, simulationTimeStep);
            spritesToDrawInZOrder.emplace_back(&enemy.Visuals.Sprite);
        }

        for (size_t i = 0; i < world.FloatingTexts.size();)
        {
            world.FloatingTexts[i].Update(deltaTime);
            if (world.FloatingTexts[i].HasExpired())
            {
                if ((i + 1) < world.FloatingTexts.size())
                {
                    world.FloatingTexts[i] = std::move(world.FloatingTexts.back());
                }
                world.FloatingTexts.pop_back();
            }
            else
            {
                ++i;
            }
        }

        std::ranges::sort(
            spritesToDrawInZOrder, [](const sf::Sprite *const left, const sf::Sprite *const right) -> bool {
                return (bottomOfSprite(*left) < bottomOfSprite(*right));
            });
        for (const sf::Sprite *const sprite : spritesToDrawInZOrder)
        {
            camera.draw(window, *sprite);
        }

        for (FloatingText &floatingText : world.FloatingTexts)
        {
            camera.draw(window, *floatingText.Text);
        }

        drawHealthBar(window, camera, player);
        for (const Object &enemy : world.enemies)
        {
            drawHealthBar(window, camera, enemy);
        }

        {
            sf::CircleShape circle(1);
            circle.setOutlineColor(sf::Color(0, 255, 0));
            circle.setFillColor(sf::Color(0, 255, 0));
            circle.setPosition(player.Logic.Position);
            camera.draw(window, circle);
        }
        for (const Object &enemy : world.enemies)
        {
            {
                sf::CircleShape circle(1);
                circle.setOutlineColor(sf::Color(255, 0, 0));
                circle.setFillColor(sf::Color(255, 0, 0));
                circle.setPosition(enemy.Logic.Position);
                camera.draw(window, circle);
            }

            if (&enemy == selectedEnemy)
            {
                sf::RectangleShape rect;
                rect.setPosition(enemy.Logic.Position - enemy.Visuals.GetOffset());
                rect.setSize(sf::Vector2f(enemy.Visuals.SpriteSize));
                rect.setFillColor(sf::Color::Transparent);
                rect.setOutlineColor(sf::Color::White);
                rect.setOutlineThickness(1);
                camera.draw(window, rect);
            }
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}
