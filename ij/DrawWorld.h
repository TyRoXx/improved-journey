#pragma once
#include "World.h"
#include <array>

namespace ij
{
    struct Canvas;
    struct Camera;
    struct Input;

    struct Debugging
    {
        size_t enemiesDrawnLastFrame = 0;
        size_t tilesDrawnLastFrame = 0;
        std::array<float, 5 *FrameRate> FrameTimes = {};
        size_t NextFrameTime = 0;
        bool IsZoomedOut = false;
    };

    void DrawWorld(Canvas &canvas, const Camera &camera, const Input &input, Debugging &debugging, World &world,
                   Object &player, const TextureId grassTexture, const TimeSpan timeSinceLastDraw);
} // namespace ij
