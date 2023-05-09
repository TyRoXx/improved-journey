#include "RunGame.h"
#include "DrawWorld.h"
#include "PlayerCharacter.h"
#include "UserInterface.h"
#include <iostream>

ij::WindowFunctions::~WindowFunctions()
{
}

[[nodiscard]] bool ij::RunGame(TextureLoader &textures, Canvas &canvas, const std::filesystem::path &assets,
                               WindowFunctions &window)
{
    using namespace ij;

    const std::filesystem::path wolfsheet1File = (assets / "LPC Wolfman" / "Male" / "Gray" / "Universal.png");
    assert(std::filesystem::exists(wolfsheet1File));

    const std::optional<TextureId> wolfsheet1Texture = textures.LoadFromFile(wolfsheet1File);
    if (!wolfsheet1Texture)
    {
        std::cerr << "Could not load player texture\n";
        return false;
    }

    const std::filesystem::path grassFile = (assets / "LPC Base Assets" / "tiles" / "grass.png");
    const std::optional<TextureId> grassTexture = textures.LoadFromFile(grassFile);
    if (!grassTexture)
    {
        std::cerr << "Could not load grass texture\n";
        return false;
    }

    const std::optional<std::vector<EnemyTemplate>> maybeEnemies = LoadEnemies(textures, assets);
    if (!maybeEnemies)
    {
        std::cerr << "Could not load enemies\n";
        return false;
    }

    Input input;
    StandardRandomNumberGenerator randomNumberGenerator;
    const Map map = GenerateRandomMap(randomNumberGenerator);

    constexpr float enemiesPerTile = 0.02f;
    const size_t numberOfEnemies = static_cast<size_t>(AssertCast<float>(map.Tiles.size()) * enemiesPerTile);
    World world(0, map, canvas);
    SpawnEnemies(world, numberOfEnemies, *maybeEnemies, randomNumberGenerator);

    Object player(VisualEntity(*wolfsheet1Texture, Vector2u(64, 64), 0, TimeSpan::FromMilliseconds(0), CutWolfTexture,
                               ObjectAnimation::Standing),
                  LogicEntity(std::make_unique<PlayerCharacter>(input.isDirectionKeyPressed, input.isAttackPressed),
                              GenerateRandomPointForSpawning(world, randomNumberGenerator), Vector2f(0, 0), true, false,
                              100, 100, ObjectActivity::Standing));

    Camera camera{player.Logic.Position};
    Debugging debugging;
    TimeSpan remainingSimulationTime = TimeSpan::FromMilliseconds(0);
    while (window.IsOpen())
    {
        window.ProcessEvents(input, camera, world);

        const TimeSpan deltaTime = window.RestartDeltaClock();
        debugging.FrameTimes[debugging.NextFrameTime] = AssertCast<float>(deltaTime.Milliseconds);
        debugging.NextFrameTime = (debugging.NextFrameTime + 1) % debugging.FrameTimes.size();

        // fix the time step to make physics and NPC behaviour independent from the frame rate
        remainingSimulationTime += deltaTime;
        UpdateWorld(remainingSimulationTime, player.Logic, world, randomNumberGenerator);

        window.UpdateGui(deltaTime);
        UpdateUserInterface(player.Logic, world, input, debugging);

        window.Clear();

        camera.Center = player.Logic.Position;
        const Vector2f windowSize = AssertCastVector<float>(canvas.GetSize());
        Vector2f viewSize = windowSize;
        if (debugging.IsZoomedOut)
        {
            viewSize = (viewSize * 2.0f);
        }
        window.SetView(
            Rectangle<float>(camera.Center - (windowSize / 2.0f) + ((windowSize - viewSize) / 2.0f), viewSize));

        DrawWorld(canvas, camera, input, debugging, world, player, *grassTexture, deltaTime);

        if (debugging.IsZoomedOut)
        {
            canvas.DrawRectangle(RoundDown<Int32>(camera.Center - AssertCastVector<float>(canvas.GetSize()) * 0.5f),
                                 canvas.GetSize(), Color(255, 0, 0, 255), Color(0, 0, 0, 0), 2);
        }

        window.RenderGui();
        window.Display();
    }

    return true;
}