#include "imgui-SFML.h"
#include "imgui.h"
#include <SFML/Graphics/CircleShape.hpp>
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

using TextureCutter = sf::IntRect(const bool isMoving, const sf::Int32 animationTime, const Direction direction,
                                  const sf::Vector2i &size);

sf::IntRect cutEnemyTexture(const bool isMoving, const sf::Int32 animationTime, const Direction direction,
                            const sf::Vector2i &size)
{
    return sf::IntRect((size.x * ((animationTime / (isMoving ? 150 : 300)) % 3)), size.y * static_cast<int>(direction),
                       size.x, size.y);
};

struct ObjectBehavior;

struct Object
{
    sf::Sprite Sprite;
    sf::Int32 AnimationTime = 0;
    sf::Vector2f Position;
    Direction Dir = Direction::Down;
    TextureCutter *Cutter = nullptr;
    sf::Vector2i SpriteSize;
    sf::Int32 VerticalOffset = 0;
    std::unique_ptr<ObjectBehavior> Behavior;
    bool isMoving = false;
};

struct ObjectBehavior
{
    virtual ~ObjectBehavior()
    {
    }

    virtual void update(Object &object) = 0;
};

struct PlayerCharacter final : ObjectBehavior
{
    const std::array<bool, 4> &isDirectionKeyPressed;

    explicit PlayerCharacter(const std::array<bool, 4> &isDirectionKeyPressed)
        : isDirectionKeyPressed(isDirectionKeyPressed)
    {
    }

    virtual void update(Object &object) final
    {
        object.isMoving = std::ranges::any_of(isDirectionKeyPressed, std::identity());
        if (object.isMoving && !isDirectionKeyPressed[static_cast<size_t>(object.Dir)])
        {
            object.Dir =
                static_cast<Direction>(std::ranges::find(isDirectionKeyPressed, true) - isDirectionKeyPressed.begin());
        }
    }
};

struct Bot final : ObjectBehavior
{
    virtual void update(Object &object) final
    {
        if (std::rand() % 2000 < 10)
        {
            object.isMoving = !object.isMoving;
            object.Dir = static_cast<Direction>(std::rand() % 4);
        }
    }
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
};

void updateObject(Object &object, const sf::Time &deltaTime)
{
    object.Behavior->update(object);

    if (object.isMoving)
    {
        const float velocity = 120;
        const auto change = DirectionToVector(object.Dir) * deltaTime.asSeconds() * velocity;
        object.Position.x += change.x;
        object.Position.y += change.y;
    }

    object.AnimationTime += deltaTime.asMilliseconds();
    object.Sprite.setTextureRect(object.Cutter(object.isMoving, object.AnimationTime, object.Dir, object.SpriteSize));
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

    TextureCutter *const wolfCutter = [](const bool isMoving, const sf::Int32 animationTime, const Direction direction,
                                         const sf::Vector2i &size) {
        return sf::IntRect(isMoving ? (size.x * ((animationTime / 80) % 9)) : 0,
                           size.y * (static_cast<int>(direction) + 8), size.x, size.y);
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

    Direction lastDirectionPressedDown = Direction::Down;
    std::array<bool, 4> isDirectionKeyPressed = {};

    Object player;
    player.Sprite.setTexture(wolfsheet1Texture);
    player.Cutter = wolfCutter;
    player.SpriteSize = sf::Vector2i(64, 64);
    player.Position = sf::Vector2f(400, 400);
    player.Behavior = std::make_unique<PlayerCharacter>(isDirectionKeyPressed);

    std::vector<Object> enemies;
    for (size_t i = 0; i < enemyFileNames.size(); ++i)
    {
        Object &enemy = enemies.emplace_back();
        enemy.Sprite.setTexture(enemyTextures[i]);
        enemy.Cutter = cutEnemyTexture;
        enemy.SpriteSize = enemySizes[i];
        enemy.Position.x = static_cast<float>(std::rand() % 1200);
        enemy.Position.y = static_cast<float>(std::rand() % 800);
        enemy.Dir = static_cast<Direction>(std::rand() % 4);
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
                    lastDirectionPressedDown = Direction::Up;
                    isDirectionKeyPressed[static_cast<size_t>(lastDirectionPressedDown)] = true;
                    break;
                case sf::Keyboard::A:
                    lastDirectionPressedDown = Direction::Left;
                    isDirectionKeyPressed[static_cast<size_t>(lastDirectionPressedDown)] = true;
                    break;
                case sf::Keyboard::S:
                    lastDirectionPressedDown = Direction::Down;
                    isDirectionKeyPressed[static_cast<size_t>(lastDirectionPressedDown)] = true;
                    break;
                case sf::Keyboard::D:
                    lastDirectionPressedDown = Direction::Right;
                    isDirectionKeyPressed[static_cast<size_t>(lastDirectionPressedDown)] = true;
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

        updateObject(player, deltaTime);
        camera.Center = player.Position;
        spritesToDrawInZOrder.emplace_back(&player.Sprite);

        for (Object &enemy : enemies)
        {
            updateObject(enemy, deltaTime);
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
