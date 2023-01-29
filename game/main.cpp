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
#include <ij/Bot.h>
#include <ij/Camera.h>
#include <ij/Direction.h>
#include <ij/FloatingText.h>
#include <ij/LogicEntity.h>
#include <ij/Map.h>
#include <ij/ObjectAnimation.h>
#include <ij/PlayerCharacter.h>
#include <ij/RandomNumberGenerator.h>
#include <ij/TextureCutter.h>
#include <ij/VisualEntity.h>
#include <ij/World.h>
#include <iostream>

namespace ij
{
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
        const float x = object.Logic.Position.x - AssertCast<float>(width) / 2;
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

    struct Input final
    {
        std::array<bool, 4> isDirectionKeyPressed = {};
        bool isAttackPressed = false;
        Object *selectedEnemy = nullptr;

        void ProcessEvents(sf::RenderWindow &window, const Camera &camera, World &world)
        {
            sf::Event event = {};
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
        }
    };

    constexpr unsigned frameRate = 60;

    struct Debugging
    {
        size_t enemiesDrawnLastFrame = 0;
        size_t tilesDrawnLastFrame = 0;
        std::array<float, 5 *frameRate> FrameTimes = {};
        size_t NextFrameTime = 0;
    };

    void UpdateUserInterface(sf::RenderWindow &window, const sf::Time deltaTime, LogicEntity &player,
                             const World &world, const Input &input, const Debugging &debugging)
    {
        ImGui::SFML::Update(window, deltaTime);
        ImGui::Begin("Character");
        {
            ImGui::Text("Health");
            ImGui::SameLine();
            ImGui::ProgressBar(AssertCast<float>(player.GetCurrentHealth()) /
                               AssertCast<float>(player.GetMaximumHealth()));
        }
        ImGui::End();

        if (Object *const selectedEnemy = input.selectedEnemy)
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
        ImGui::LabelText("Enemies drawn", "%zu", debugging.enemiesDrawnLastFrame);
        ImGui::LabelText("Tiles in the world", "%zu", world.map.Tiles.size());
        ImGui::LabelText("Tiles drawn", "%zu", debugging.tilesDrawnLastFrame);
        ImGui::LabelText("Floating texts in the world", "%zu", world.FloatingTexts.size());
        ImGui::Checkbox("Player/wall collision", &player.HasCollisionWithWalls);
        ImGui::PlotHistogram("Frame times (ms)", debugging.FrameTimes.data(),
                             AssertCast<int>(debugging.FrameTimes.size()), AssertCast<int>(debugging.NextFrameTime),
                             nullptr, 0.0f, 100.0f, ImVec2(300, 100));
        ImGui::End();
    }

    void DrawWorld(sf::RenderWindow &window, Camera &camera, Input &input, Debugging &debugging, World &world,
                   Object &player, const sf::Texture &grassTexture, const sf::Time timeSinceLastDraw)
    {
        const sf::Vector2i topLeft =
            findTileByCoordinates(camera.getWorldFromScreenCoordinates(window, sf::Vector2i(0, 0)));
        const sf::Vector2i bottomRight =
            findTileByCoordinates(camera.getWorldFromScreenCoordinates(window, sf::Vector2i(window.getSize())));

        debugging.tilesDrawnLastFrame = 0;
        for (size_t y = AssertCast<size_t>((std::max)(0, topLeft.y)),
                    yStop = AssertCast<size_t>(
                        (std::min<ptrdiff_t>)(bottomRight.y, AssertCast<ptrdiff_t>(world.map.GetHeight() - 1)));
             y <= yStop; ++y)
        {
            for (size_t x = AssertCast<size_t>((std::max)(0, topLeft.x)),
                        xStop = AssertCast<size_t>(
                            (std::min<ptrdiff_t>)(bottomRight.x, AssertCast<ptrdiff_t>(world.map.Width - 1)));
                 x <= xStop; ++x)
            {
                const int tile = world.map.GetTileAt(x, y);
                if (tile == NoTile)
                {
                    continue;
                }
                sf::Sprite grass(grassTexture);
                grass.setTextureRect(sf::IntRect(tile * TileSize, 160, TileSize, TileSize));
                grass.setPosition(sf::Vector2f(AssertCast<float>(x) * TileSize, AssertCast<float>(y) * TileSize));
                camera.draw(window, grass);
                ++debugging.tilesDrawnLastFrame;
            }
        }

        std::vector<const Object *> visibleEnemies;
        std::vector<const sf::Sprite *> spritesToDrawInZOrder;

        updateVisuals(player.Logic, player.Visuals, timeSinceLastDraw);
        spritesToDrawInZOrder.emplace_back(&player.Visuals.Sprite);

        debugging.enemiesDrawnLastFrame = 0;
        for (Object &enemy : world.enemies)
        {
            updateVisuals(enemy.Logic, enemy.Visuals, timeSinceLastDraw);
            if (camera.canSee(window, enemy.Visuals))
            {
                visibleEnemies.push_back(&enemy);
                spritesToDrawInZOrder.emplace_back(&enemy.Visuals.Sprite);
                ++debugging.enemiesDrawnLastFrame;
            }
        }

        for (size_t i = 0; i < world.FloatingTexts.size();)
        {
            world.FloatingTexts[i].Update(timeSinceLastDraw);
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

            if (enemy == input.selectedEnemy)
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
    }

    void UpdateWorld(sf::Time &remainingSimulationTime, LogicEntity &player, World &world,
                     RandomNumberGenerator &randomNumberGenerator)
    {
        const sf::Time simulationTimeStep = sf::milliseconds(AssertCast<sf::Int32>(1000 / frameRate));
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

    struct EnemyTemplate
    {
        sf::Texture Texture;
        sf::Vector2i Size;
        int VerticalOffset;
        TextureCutter *Cutter;

        EnemyTemplate(const sf::Texture &texture, const sf::Vector2i &size, const int verticalOffset,
                      TextureCutter *const cutter)
            : Texture(texture)
            , Size(size)
            , VerticalOffset(verticalOffset)
            , Cutter(cutter)
        {
        }
    };

    sf::Vector2f GenerateRandomPointForSpawning(const World &world, RandomNumberGenerator &randomNumberGenerator)
    {
        sf::Vector2f position;
        size_t attempt = 0;
        do
        {
            ++attempt;
            assert(attempt < 100);
            position.x = AssertCast<float>(
                TileSize * randomNumberGenerator.GenerateInt32(0, AssertCast<sf::Int32>(world.map.Width - 1)) +
                (TileSize / 2));
            position.y = AssertCast<float>(
                TileSize * randomNumberGenerator.GenerateInt32(0, AssertCast<sf::Int32>(world.map.GetHeight() - 1)) +
                (TileSize / 2));
        } while (!IsWalkable(position, DefaultEntityDimensions, world));
        return position;
    }

    void SpawnEnemies(World &world, const size_t numberOfEnemies, const std::vector<EnemyTemplate> &enemies,
                      RandomNumberGenerator &randomNumberGenerator)
    {
        for (const EnemyTemplate &enemyTemplate : enemies)
        {
            for (size_t k = 0; k < (numberOfEnemies / enemies.size()); ++k)
            {
                const sf::Vector2f position = GenerateRandomPointForSpawning(world, randomNumberGenerator);
                const sf::Vector2f direction =
                    DirectionToVector(AssertCast<Direction>(randomNumberGenerator.GenerateInt32(0, 3)));
                world.enemies.emplace_back(
                    VisualEntity(sf::Sprite(enemyTemplate.Texture), enemyTemplate.Size, enemyTemplate.VerticalOffset, 0,
                                 enemyTemplate.Cutter, ObjectAnimation::Standing),
                    LogicEntity(
                        std::make_unique<Bot>(), position, direction, true, false, 100, 100, ObjectActivity::Standing));
            }
        }
    }

    std::optional<std::vector<EnemyTemplate>> LoadEnemies(const std::filesystem::path &assets)
    {
        std::array<sf::Texture, 10> enemyTextures = {};
        const std::array<const char *, 10> enemyFileNames = {
            "bat",      "bee",   "big_worm",   "eyeball", "ghost", "man_eater_flower",
            "pumpking", "slime", "small_worm", "snake"};
        for (size_t i = 0; i < enemyFileNames.size(); ++i)
        {
            const std::filesystem::path enemyFile =
                (assets / "lpc-monsters" / (std::string(enemyFileNames[i]) + ".png"));
            if (!enemyTextures[i].loadFromFile(enemyFile.string()))
            {
                return std::nullopt;
            }
        }

        std::vector<EnemyTemplate> enemies;
        enemies.emplace_back(enemyTextures[0], sf::Vector2i(64, 64), 4, &cutEnemyTexture<4, 3>);
        enemies.emplace_back(enemyTextures[1], sf::Vector2i(32, 32), 2, &cutEnemyTexture<3, 3>);
        enemies.emplace_back(enemyTextures[2], sf::Vector2i(64, 64), 18, &cutEnemyTexture<3, 3>);
        enemies.emplace_back(enemyTextures[3], sf::Vector2i(64, 64), 17, &cutEnemyTexture<3, 3>);
        enemies.emplace_back(enemyTextures[4], sf::Vector2i(64, 64), 13, &cutEnemyTexture<3, 3>);
        enemies.emplace_back(enemyTextures[5], sf::Vector2i(128, 128), 28, &cutEnemyTexture<3, 3>);
        enemies.emplace_back(enemyTextures[6], sf::Vector2i(64, 64), 10, &cutEnemyTexture<3, 3>);
        enemies.emplace_back(enemyTextures[7], sf::Vector2i(64, 64), 20, &cutEnemyTexture<3, 3>);
        enemies.emplace_back(enemyTextures[8], sf::Vector2i(64, 64), 19, &cutEnemyTexture<3, 7>);
        enemies.emplace_back(enemyTextures[9], sf::Vector2i(64, 64), 18, &cutEnemyTexture<4, 3>);
        return enemies;
    }
} // namespace ij

int main()
{
    using namespace ij;
    sf::RenderWindow window(sf::VideoMode(1200, 800), "Improved Journey");
    window.setFramerateLimit(frameRate);
    if (!ImGui::SFML::Init(window))
    {
        std::cerr << "Could not initialize ImGui::SFML\n";
        return 1;
    }

    const std::filesystem::path assets =
        std::filesystem::current_path().parent_path().parent_path() / "improved-journey" / "assets";
    loadAllSprites(assets);

    sf::Font font;
    if (!font.loadFromFile((assets / "Roboto-Font" / "Roboto-Light.ttf").string()))
    {
        std::cerr << "Could not load font\n";
        return 1;
    }

    const std::filesystem::path wolfsheet1File = (assets / "LPC Wolfman" / "Male" / "Gray" / "Universal.png");
    assert(std::filesystem::exists(wolfsheet1File));

    sf::Texture wolfsheet1Texture;
    if (!wolfsheet1Texture.loadFromFile(wolfsheet1File.string()))
    {
        std::cerr << "Could not load player texture\n";
        return 1;
    }

    const std::filesystem::path grassFile = (assets / "LPC Base Assets" / "tiles" / "grass.png");
    sf::Texture grassTexture;
    if (!grassTexture.loadFromFile(grassFile.string()))
    {
        std::cerr << "Could not load grass texture\n";
        return 1;
    }

    const std::optional<std::vector<EnemyTemplate>> maybeEnemies = LoadEnemies(assets);
    if (!maybeEnemies)
    {
        std::cerr << "Could not load enemies\n";
        return 1;
    }

    Input input;
    StandardRandomNumberGenerator randomNumberGenerator;
    const Map map = GenerateRandomMap(randomNumberGenerator);

    constexpr float enemiesPerTile = 0.02f;
    const size_t numberOfEnemies = static_cast<size_t>(AssertCast<float>(map.Tiles.size()) * enemiesPerTile);
    World world(font, map);
    SpawnEnemies(world, numberOfEnemies, *maybeEnemies, randomNumberGenerator);

    Object player(VisualEntity(sf::Sprite(wolfsheet1Texture), sf::Vector2i(64, 64), 0, 0, CutWolfTexture,
                               ObjectAnimation::Standing),
                  LogicEntity(std::make_unique<PlayerCharacter>(input.isDirectionKeyPressed, input.isAttackPressed),
                              GenerateRandomPointForSpawning(world, randomNumberGenerator), sf::Vector2f(), true, false,
                              100, 100, ObjectActivity::Standing));

    Camera camera{player.Logic.Position};
    Debugging debugging;
    sf::Clock deltaClock;
    sf::Time remainingSimulationTime;
    while (window.isOpen())
    {
        input.ProcessEvents(window, camera, world);

        const sf::Time deltaTime = deltaClock.restart();
        debugging.FrameTimes[debugging.NextFrameTime] = AssertCast<float>(deltaTime.asMilliseconds());
        debugging.NextFrameTime = (debugging.NextFrameTime + 1) % debugging.FrameTimes.size();

        // fix the time step to make physics and NPC behaviour independent from the frame rate
        remainingSimulationTime += deltaTime;
        UpdateWorld(remainingSimulationTime, player.Logic, world, randomNumberGenerator);

        UpdateUserInterface(window, deltaTime, player.Logic, world, input, debugging);

        window.clear();

        camera.Center = player.Logic.Position;

        DrawWorld(window, camera, input, debugging, world, player, grassTexture, deltaTime);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}
