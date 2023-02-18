#include "Input.h"
#include "FromSfml.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

void ij::Input::ProcessEvents(sf::RenderWindow &window, const Camera &camera, World &world)
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
            const Vector2f pointInWorld = camera.getWorldFromScreenCoordinates(
                FromSfml(window.getSize()), sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            selectedEnemy = FindEnemyByPosition(world, pointInWorld);
        }
    }
}
