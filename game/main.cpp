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

    const auto grassFile = (assets / "LPC Base Assets" / "tiles" / "grass.png");
    sf::Texture grassTexture;
    if (!grassTexture.loadFromFile(grassFile.string()))
    {
        return 1;
    }

    sf::Sprite wolf(wolfsheet1Texture);
    sf::Int32 wolfAnimationTime = 0;
    sf::Vector2f wolfPosition;
    std::array<bool, 4> isDirectionKeyPressed = {};
    Direction lastDirectionPressedDown = Direction::Down;

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
        const bool isWolfMoving = std::ranges::any_of(isDirectionKeyPressed, std::identity());
        Direction wolfDirection = lastDirectionPressedDown;
        if (isWolfMoving)
        {
            if (!isDirectionKeyPressed[static_cast<size_t>(wolfDirection)])
            {
                wolfDirection = static_cast<Direction>(std::ranges::find(isDirectionKeyPressed, true) -
                                                       isDirectionKeyPressed.begin());
            }
            const float velocity = 120;
            const auto change = DirectionToVector(wolfDirection) * deltaTime.asSeconds() * velocity;
            wolfPosition.x += change.x;
            wolfPosition.y += change.y;
        }

        ImGui::SFML::Update(window, deltaTime);

        window.clear();

        for (sf::Int32 y = 0; y < 30; ++y)
        {
            for (sf::Int32 x = 0; x < 30; ++x)
            {
                sf::Sprite grass(grassTexture);
                grass.setTextureRect(sf::IntRect(((x + y) % 3) * 32, 160, 32, 32));
                grass.setPosition(sf::Vector2f(static_cast<float>(x) * 32.0f, static_cast<float>(y) * 32.0f));
                window.draw(grass);
            }
        }

        if (isWolfMoving)
        {
            wolfAnimationTime += deltaTime.asMilliseconds();
        }

        wolf.setTextureRect(sf::IntRect(isWolfMoving ? (64 * ((wolfAnimationTime / 80) % 9)) : 0,
                                        64 * (static_cast<int>(wolfDirection) + 8), 64, 64));
        wolf.setPosition(wolfPosition);
        window.draw(wolf);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}
