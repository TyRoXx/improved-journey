#include "imgui-SFML.h"
#include "imgui.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <array>
#include <filesystem>
#include <fmt/format.h>
#include <ij/AssertCast.h>
#include <ij/Direction.h>
#include <ij/FloatingText.h>
#include <ij/LogicEntity.h>
#include <ij/Map.h>
#include <ij/Normalize.h>
#include <ij/ObjectAnimation.h>
#include <ij/RandomNumberGenerator.h>
#include <ij/TextureCutter.h>
#include <ij/Unreachable.h>
#include <ij/VisualEntity.h>
#include <iostream>
#include <random>

namespace ij
{
    struct Object final
    {
        VisualEntity Visuals;
        LogicEntity Logic;
    };

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

    void InflictDamage(LogicEntity &damaged, World &world, const Health damage, RandomNumberGenerator &random)
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

        sf::Vector2f getWorldFromScreenCoordinates(const sf::RenderWindow &window, const sf::Vector2i &point) const
        {
            return (sf::Vector2f(window.getSize()) * -0.5f) + Center + sf::Vector2f(point);
        }

        bool canSee(const sf::RenderWindow &window, const VisualEntity &entity) const
        {
            const sf::Rect<float> cameraArea(
                getWorldFromScreenCoordinates(window, sf::Vector2i(0, 0)), sf::Vector2f(window.getSize()));
            const sf::Rect<float> entityArea(entity.Sprite.getPosition(), sf::Vector2f(entity.SpriteSize));
            return cameraArea.intersects(entityArea);
        }
    };

    constexpr int TileSize = 32;
    const sf::Vector2f DefaultEntityDimensions(8, 8);

    [[nodiscard]] bool IsWalkablePoint(const sf::Vector2f &point, const World &world)
    {
        const sf::Vector2<ptrdiff_t> tileIndex(AssertCast<ptrdiff_t>(std::floor(point.x / TileSize)),
                                               AssertCast<ptrdiff_t>(std::floor(point.y / TileSize)));
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
        if (!entity.HasCollisionWithWalls || IsWalkable(desiredDestination, DefaultEntityDimensions, world))
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

    template <class Integer>
    Integer RoundDown(const float value)
    {
        return AssertCast<Integer>(std::floor(value));
    }

    sf::Vector2i findTileByCoordinates(const sf::Vector2f &position)
    {
        return sf::Vector2i(RoundDown<sf::Int32>(position.x / TileSize), RoundDown<sf::Int32>(position.y / TileSize));
    }
} // namespace ij

int main()
{
    using namespace ij;
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
    player.Visuals.Cutter = CutWolfTexture;
    player.Visuals.SpriteSize = sf::Vector2i(64, 64);
    player.Logic.Position = sf::Vector2f(400, 400);
    player.Logic.Behavior = std::make_unique<PlayerCharacter>(isDirectionKeyPressed, isAttackPressed);

    StandardRandomNumberGenerator randomNumberGenerator;
    const Map map = GenerateRandomMap(randomNumberGenerator);

    const size_t numberOfTiles = map.Tiles.size();
    const float enemiesPerTile = 0.02f;
    const size_t numberOfEnemies = static_cast<size_t>(AssertCast<float>(numberOfTiles) * enemiesPerTile);
    World world(font, map);
    for (size_t i = 0; i < enemyFileNames.size(); ++i)
    {
        for (size_t k = 0; k < (numberOfEnemies / enemyFileNames.size()); ++k)
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
    size_t tilesDrawnLastFrame = 0;
    size_t enemiesDrawnLastFrame = 0;

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

        ImGui::Begin("Debug");
        ImGui::LabelText("Enemies in the world", "%zu", world.enemies.size());
        ImGui::LabelText("Enemies drawn", "%zu", enemiesDrawnLastFrame);
        ImGui::LabelText("Tiles in the world", "%zu", map.Tiles.size());
        ImGui::LabelText("Tiles drawn", "%zu", tilesDrawnLastFrame);
        ImGui::LabelText("Floating texts in the world", "%zu", world.FloatingTexts.size());
        ImGui::Checkbox("Player/wall collision", &player.Logic.HasCollisionWithWalls);
        ImGui::End();

        window.clear();

        camera.Center = player.Logic.Position;

        const sf::Vector2i topLeft =
            findTileByCoordinates(camera.getWorldFromScreenCoordinates(window, sf::Vector2i(0, 0)));
        const sf::Vector2i bottomRight =
            findTileByCoordinates(camera.getWorldFromScreenCoordinates(window, sf::Vector2i(window.getSize())));

        tilesDrawnLastFrame = 0;
        for (size_t y = AssertCast<size_t>((std::max)(0, topLeft.y)),
                    yStop = AssertCast<size_t>(
                        (std::min<ptrdiff_t>)(bottomRight.y, AssertCast<ptrdiff_t>(map.GetHeight() - 1)));
             y <= yStop; ++y)
        {
            for (size_t x = AssertCast<size_t>((std::max)(0, topLeft.x)),
                        xStop = AssertCast<size_t>(
                            (std::min<ptrdiff_t>)(bottomRight.x, AssertCast<ptrdiff_t>(map.Width - 1)));
                 x <= xStop; ++x)
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
                ++tilesDrawnLastFrame;
            }
        }

        std::vector<const Object *> visibleEnemies;
        std::vector<const sf::Sprite *> spritesToDrawInZOrder;

        updateVisuals(player.Logic, player.Visuals, simulationTimeStep);
        spritesToDrawInZOrder.emplace_back(&player.Visuals.Sprite);

        enemiesDrawnLastFrame = 0;
        for (Object &enemy : world.enemies)
        {
            updateVisuals(enemy.Logic, enemy.Visuals, simulationTimeStep);
            if (camera.canSee(window, enemy.Visuals))
            {
                visibleEnemies.push_back(&enemy);
                spritesToDrawInZOrder.emplace_back(&enemy.Visuals.Sprite);
                ++enemiesDrawnLastFrame;
            }
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
        for (const Object *const enemy : visibleEnemies)
        {
            drawHealthBar(window, camera, *enemy);
        }

        {
            sf::CircleShape circle(1);
            circle.setOutlineColor(sf::Color(0, 255, 0));
            circle.setFillColor(sf::Color(0, 255, 0));
            circle.setPosition(player.Logic.Position);
            camera.draw(window, circle);
        }
        for (const Object *const enemy : visibleEnemies)
        {
            {
                sf::CircleShape circle(1);
                circle.setOutlineColor(sf::Color(255, 0, 0));
                circle.setFillColor(sf::Color(255, 0, 0));
                circle.setPosition(enemy->Logic.Position);
                camera.draw(window, circle);
            }

            if (enemy == selectedEnemy)
            {
                sf::RectangleShape rect;
                rect.setPosition(enemy->Logic.Position - enemy->Visuals.GetOffset());
                rect.setSize(sf::Vector2f(enemy->Visuals.SpriteSize));
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
