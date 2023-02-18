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
#include <ij/FromSfml.h>
#include <ij/Input.h>
#include <ij/LogicEntity.h>
#include <ij/Map.h>
#include <ij/ObjectAnimation.h>
#include <ij/PlayerCharacter.h>
#include <ij/RandomNumberGenerator.h>
#include <ij/TextureCutter.h>
#include <ij/ToSfml.h>
#include <ij/VisualEntity.h>
#include <ij/World.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>

namespace ij
{
    struct Color final
    {
        std::uint8_t Red, Green, Blue, Alpha;

        Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) noexcept
            : Red(red)
            , Green(green)
            , Blue(blue)
            , Alpha(alpha)
        {
        }
    };

    sf::Color ToSfml(const Color &value) noexcept
    {
        return sf::Color(value.Red, value.Green, value.Blue, value.Alpha);
    }

    struct Canvas
    {
        virtual ~Canvas()
        {
        }
        [[nodiscard]] virtual Vector2u GetSize() = 0;
        virtual void DrawDot(const Vector2i &position, Color color) = 0;
        virtual void DrawRectangle(const Vector2i &topLeft, const Vector2u &size, Color outline, Color fill) = 0;
    };

    struct SfmlCanvas final : Canvas
    {
        sf::RenderWindow &Window;

        explicit SfmlCanvas(sf::RenderWindow &window)
            : Window(window)
        {
        }

        [[nodiscard]] Vector2u GetSize() override
        {
            return FromSfml(Window.getSize());
        }

        void DrawDot(const Vector2i &position, const Color color) override
        {
            sf::CircleShape circle(1);
            circle.setOutlineColor(ToSfml(color));
            circle.setFillColor(ToSfml(color));
            circle.setPosition(sf::Vector2f(ToSfml(position)));
            Window.draw(circle);
        }

        void DrawRectangle(const Vector2i &topLeft, const Vector2u &size, const Color outline,
                           const Color fill) override
        {
            sf::RectangleShape rect;
            rect.setPosition(sf::Vector2f(ToSfml(topLeft)));
            rect.setSize(sf::Vector2f(ToSfml(size)));
            rect.setFillColor(ToSfml(fill));
            rect.setOutlineColor(ToSfml(outline));
            rect.setOutlineThickness(1);
            Window.draw(rect);
        }
    };

    float bottomOfSprite(const sf::Sprite &sprite)
    {
        return (sprite.getPosition().y + AssertCast<float>(sprite.getTextureRect().height));
    }

    void drawHealthBar(Canvas &canvas, const Object &object)
    {
        if (object.Logic.GetCurrentHealth() == object.Logic.GetMaximumHealth())
        {
            return;
        }
        constexpr UInt32 width = 24;
        constexpr UInt32 height = 4;
        const Int32 x = RoundDown<Int32>(object.Logic.Position.x) - AssertCast<Int32>(width / 2);
        const Int32 y =
            RoundDown<Int32>(object.Logic.Position.y) - object.Visuals.GetTextureRect(object.Logic.Direction).height;
        const UInt32 greenPortion =
            RoundDown<UInt32>(AssertCast<float>(object.Logic.GetCurrentHealth()) /
                              AssertCast<float>(object.Logic.GetMaximumHealth()) * AssertCast<float>(width));
        const Color green(0, 255, 0, 255);
        canvas.DrawRectangle(Vector2i(x, y), Vector2u(greenPortion, height), green, green);
        const Color red(255, 0, 0, 255);
        canvas.DrawRectangle(
            Vector2i(x + AssertCast<Int32>(greenPortion), y), Vector2u((width - greenPortion), height), red, red);
    }

    Vector2i findTileByCoordinates(const Vector2f &position)
    {
        return Vector2i(RoundDown<Int32>(position.x / TileSize), RoundDown<Int32>(position.y / TileSize));
    }

    struct Debugging
    {
        size_t enemiesDrawnLastFrame = 0;
        size_t tilesDrawnLastFrame = 0;
        std::array<float, 5 *FrameRate> FrameTimes = {};
        size_t NextFrameTime = 0;
        bool IsZoomedOut = false;
    };

    void UpdateUserInterface(LogicEntity &player, const World &world, const Input &input, Debugging &debugging)
    {
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

    void DrawWorld(sf::RenderWindow &window, const Camera &camera, const Input &input, Debugging &debugging,
                   World &world, Object &player, const sf::Texture &grassTexture, const TimeSpan timeSinceLastDraw)
    {
        SfmlCanvas canvas{window};
        const Vector2u windowSize = canvas.GetSize();
        const Vector2i topLeft =
            findTileByCoordinates(camera.getWorldFromScreenCoordinates(windowSize, sf::Vector2i(0, 0)));
        const Vector2i bottomRight =
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

        drawHealthBar(canvas, player);
        for (const Object *const enemy : visibleEnemies)
        {
            drawHealthBar(canvas, *enemy);
        }

        canvas.DrawDot(RoundDown<Int32>(player.Logic.Position), Color(0, 255, 0, 255));

        for (const Object *const enemy : visibleEnemies)
        {
            canvas.DrawDot(RoundDown<Int32>(enemy->Logic.Position), Color(255, 0, 0, 255));

            if (enemy == input.selectedEnemy)
            {
                canvas.DrawRectangle(RoundDown<Int32>(enemy->Visuals.GetTopLeftPosition(enemy->Logic.Position)),
                                     enemy->Visuals.SpriteSize, Color(255, 255, 255, 255), Color(0, 0, 0, 0));
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

    Object player(VisualEntity(&wolfsheet1Texture, Vector2u(64, 64), 0, TimeSpan::FromMilliseconds(0), CutWolfTexture,
                               ObjectAnimation::Standing),
                  LogicEntity(std::make_unique<PlayerCharacter>(input.isDirectionKeyPressed, input.isAttackPressed),
                              GenerateRandomPointForSpawning(world, randomNumberGenerator), Vector2f(0, 0), true, false,
                              100, 100, ObjectActivity::Standing));

    Camera camera{player.Logic.Position};
    Debugging debugging;
    sf::Clock deltaClock;
    TimeSpan remainingSimulationTime = TimeSpan::FromMilliseconds(0);
    while (window.isOpen())
    {
        input.ProcessEvents(window, camera, world);

        const sf::Time sfmlDeltaTime = deltaClock.restart();
        const TimeSpan deltaTime = TimeSpan::FromMilliseconds(sfmlDeltaTime.asMilliseconds());
        debugging.FrameTimes[debugging.NextFrameTime] = AssertCast<float>(deltaTime.Milliseconds);
        debugging.NextFrameTime = (debugging.NextFrameTime + 1) % debugging.FrameTimes.size();

        // fix the time step to make physics and NPC behaviour independent from the frame rate
        remainingSimulationTime += deltaTime;
        UpdateWorld(remainingSimulationTime, player.Logic, world, randomNumberGenerator);

        ImGui::SFML::Update(window, sfmlDeltaTime);
        UpdateUserInterface(player.Logic, world, input, debugging);

        window.clear();

        camera.Center = player.Logic.Position;
        sf::Vector2f viewSize = sf::Vector2f(window.getSize());
        if (debugging.IsZoomedOut)
        {
            viewSize *= 2.0f;
        }
        window.setView(sf::View(ToSfml(camera.Center), viewSize));

        DrawWorld(window, camera, input, debugging, world, player, grassTexture, deltaTime);

        if (debugging.IsZoomedOut)
        {
            sf::RectangleShape cameraBorder;
            cameraBorder.setPosition(ToSfml(camera.Center - FromSfml(sf::Vector2f(window.getSize())) * 0.5f));
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
