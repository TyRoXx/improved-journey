#pragma once
#include "Camera.h"
#include "World.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <array>

namespace ij
{
    struct Input final
    {
        std::array<bool, 4> isDirectionKeyPressed = {};
        bool isAttackPressed = false;
        Object *selectedEnemy = nullptr;

        void ProcessEvents(sf::RenderWindow &window, const Camera &camera, World &world);
    };

} // namespace ij
