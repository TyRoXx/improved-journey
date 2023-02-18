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
#include <ij/EnemyTemplate.h>
#include <ij/FloatingText.h>
#include <ij/Input.h>
#include <ij/LogicEntity.h>
#include <ij/Map.h>
#include <ij/ObjectAnimation.h>
#include <ij/PlayerCharacter.h>
#include <ij/RandomNumberGenerator.h>
#include <ij/TextureCutter.h>
#include <ij/VisualEntity.h>
#include <ij/World.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>

namespace ij
{
    float bottomOfSprite(const sf::Sprite &sprite)
    {
        return (sprite.getPosition().y + AssertCast<float>(sprite.getTextureRect().height));
    }

    void drawHealthBar(sf::RenderWindow &window, const Object &object)
    {
        if (object.Logic.GetCurrentHealth() == object.Logic.GetMaximumHealth())
        {
            return;
        }
        constexpr Int32 width = 24;
        constexpr Int32 height = 4;
        const float x = object.Logic.Position.x - AssertCast<float>(width) / 2;
        const float y =
            object.Logic.Position.y - AssertCast<float>(object.Visuals.GetTextureRect(object.Logic.Direction).height);
        const float greenPortion = AssertCast<float>(object.Logic.GetCurrentHealth()) /
                                   AssertCast<float>(object.Logic.GetMaximumHealth()) * AssertCast<float>(width);
        {
            sf::RectangleShape green;
            green.setPosition(sf::Vector2f(x, y));
            green.setFillColor(sf::Color::Green);
            green.setSize(sf::Vector2f(greenPortion, height));
            window.draw(green);
        }
        {
            sf::RectangleShape red;
            red.setPosition(sf::Vector2f(x + greenPortion, y));
            red.setFillColor(sf::Color::Red);
            red.setSize(sf::Vector2f(width - greenPortion, height));
            window.draw(red);
        }
    }

    template <class Integer>
    Integer RoundDown(const float value)
    {
        return AssertCast<Integer>(std::floor(value));
    }

    sf::Vector2i findTileByCoordinates(const sf::Vector2f &position)
    {
        return sf::Vector2i(RoundDown<Int32>(position.x / TileSize), RoundDown<Int32>(position.y / TileSize));
    }

    struct Debugging
    {
        size_t enemiesDrawnLastFrame = 0;
        size_t tilesDrawnLastFrame = 0;
        std::array<float, 5 *FrameRate> FrameTimes = {};
        size_t NextFrameTime = 0;
        bool IsZoomedOut = false;
    };

    void UpdateUserInterface(sf::RenderWindow &window, const sf::Time deltaTime, LogicEntity &player,
                             const World &world, const Input &input, Debugging &debugging)
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
                ImGui::LabelText("Direction", "%f %f", AssertCast<double>(selectedEnemy->Logic.Direction.x),
                                 AssertCast<double>(selectedEnemy->Logic.Direction.y));
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
        ImGui::Checkbox("Zoom out", &debugging.IsZoomedOut);
        ImGui::End();
    }

    void DrawWorld(sf::RenderWindow &window, Camera &camera, Input &input, Debugging &debugging, World &world,
                   Object &player, const sf::Texture &grassTexture, const sf::Time timeSinceLastDraw)
    {
        const sf::Vector2u windowSize = window.getSize();
        const sf::Vector2i topLeft =
            findTileByCoordinates(camera.getWorldFromScreenCoordinates(windowSize, sf::Vector2i(0, 0)));
        const sf::Vector2i bottomRight =
            findTileByCoordinates(camera.getWorldFromScreenCoordinates(windowSize, sf::Vector2i(window.getSize())));

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
                window.draw(grass);
                ++debugging.tilesDrawnLastFrame;
            }
        }

        std::vector<const Object *> visibleEnemies;
        std::vector<sf::Sprite> spritesToDrawInZOrder;
        updateVisuals(player.Logic, player.Visuals, timeSinceLastDraw);
        spritesToDrawInZOrder.emplace_back(CreateSpriteForVisualEntity(player.Logic, player.Visuals));

        debugging.enemiesDrawnLastFrame = 0;
        for (Object &enemy : world.enemies)
        {
            updateVisuals(enemy.Logic, enemy.Visuals, timeSinceLastDraw);
            sf::Sprite sprite = CreateSpriteForVisualEntity(enemy.Logic, enemy.Visuals);
            if (camera.canSee(windowSize, enemy.Logic.Position, enemy.Visuals))
            {
                visibleEnemies.push_back(&enemy);
                spritesToDrawInZOrder.emplace_back(sprite);
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

        std::ranges::sort(spritesToDrawInZOrder, [](const sf::Sprite &left, const sf::Sprite &right) -> bool {
            return (bottomOfSprite(left) < bottomOfSprite(right));
        });
        for (const sf::Sprite &sprite : spritesToDrawInZOrder)
        {
            window.draw(sprite);
        }

        for (FloatingText &floatingText : world.FloatingTexts)
        {
            window.draw(*floatingText.Text);
        }

        drawHealthBar(window, player);
        for (const Object *const enemy : visibleEnemies)
        {
            drawHealthBar(window, *enemy);
        }

        {
            sf::CircleShape circle(1);
            circle.setOutlineColor(sf::Color(0, 255, 0));
            circle.setFillColor(sf::Color(0, 255, 0));
            circle.setPosition(player.Logic.Position);
            window.draw(circle);
        }
        for (const Object *const enemy : visibleEnemies)
        {
            {
                sf::CircleShape circle(1);
                circle.setOutlineColor(sf::Color(255, 0, 0));
                circle.setFillColor(sf::Color(255, 0, 0));
                circle.setPosition(enemy->Logic.Position);
                window.draw(circle);
            }

            if (enemy == input.selectedEnemy)
            {
                sf::RectangleShape rect;
                rect.setPosition(enemy->Visuals.GetTopLeftPosition(enemy->Logic.Position));
                rect.setSize(sf::Vector2f(enemy->Visuals.SpriteSize));
                rect.setFillColor(sf::Color::Transparent);
                rect.setOutlineColor(sf::Color::White);
                rect.setOutlineThickness(1);
                window.draw(rect);
            }
        }
    }
} // namespace ij

int main()
{
    using namespace ij;
    sf::RenderWindow window(sf::VideoMode(1200, 800), "Improved Journey");
    window.setFramerateLimit(FrameRate);
    if (!ImGui::SFML::Init(window))
    {
        std::cerr << "Could not initialize ImGui::SFML\n";
        return 1;
    }

    const std::filesystem::path assets =
        std::filesystem::current_path().parent_path().parent_path() / "improved-journey" / "assets";

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

    Object player(
        VisualEntity(&wolfsheet1Texture, sf::Vector2i(64, 64), 0, 0, CutWolfTexture, ObjectAnimation::Standing),
        LogicEntity(std::make_unique<PlayerCharacter>(input.isDirectionKeyPressed, input.isAttackPressed),
                    GenerateRandomPointForSpawning(world, randomNumberGenerator), sf::Vector2f(), true, false, 100, 100,
                    ObjectActivity::Standing));

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
        sf::Vector2f viewSize = sf::Vector2f(window.getSize());
        if (debugging.IsZoomedOut)
        {
            viewSize *= 2.0f;
        }
        window.setView(sf::View(camera.Center, viewSize));

        DrawWorld(window, camera, input, debugging, world, player, grassTexture, deltaTime);

        if (debugging.IsZoomedOut)
        {
            sf::RectangleShape cameraBorder;
            cameraBorder.setPosition(camera.Center - (sf::Vector2f(window.getSize()) * 0.5f));
            cameraBorder.setSize(sf::Vector2f(window.getSize()));
            cameraBorder.setOutlineColor(sf::Color::Red);
            cameraBorder.setOutlineThickness(2);
            cameraBorder.setFillColor(sf::Color::Transparent);
            window.draw(cameraBorder);
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}
