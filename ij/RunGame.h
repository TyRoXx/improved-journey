#pragma once
#include "Camera.h"
#include "EnemyTemplate.h"
#include "Input.h"

namespace ij
{
    struct WindowFunctions
    {
        virtual ~WindowFunctions() = 0;
        [[nodiscard]] virtual bool IsOpen() = 0;
        virtual void ProcessEvents(Input &input, const Camera &camera, World &world) = 0;
        virtual void UpdateGui(TimeSpan deltaTime) = 0;
        virtual void Clear() = 0;
        virtual void SetView(const Rectangle<float> &view) = 0;
        virtual void RenderGui() = 0;
        virtual void Display() = 0;
        [[nodiscard]] virtual TimeSpan RestartDeltaClock() = 0;
    };

    [[nodiscard]] bool RunGame(TextureLoader &textures, Canvas &canvas, const std::filesystem::path &assets,
                               WindowFunctions &window);
} // namespace ij
