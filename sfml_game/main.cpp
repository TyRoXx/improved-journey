#include "FromSfml.h"
#include "ToSfml.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <array>
#include <fmt/format.h>
#include <ij/AssertCast.h>
#include <ij/Bot.h>
#include <ij/DrawWorld.h>
#include <ij/FloatingText.h>
#include <ij/LogicEntity.h>
#include <ij/Map.h>
#include <ij/ObjectAnimation.h>
#include <ij/PlayerCharacter.h>
#include <ij/RandomNumberGenerator.h>
#include <ij/RunGame.h>
#include <ij/TextureCutter.h>
#include <ij/UserInterface.h>
#include <ij/VisualEntity.h>
#include <ij/World.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>

namespace ij
{
    void ProcessEvents(Input &input, sf::RenderWindow &window, const Camera &camera, World &world)
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
                    input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Up)] = true;
                    break;
                case sf::Keyboard::A:
                    input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Left)] = true;
                    break;
                case sf::Keyboard::S:
                    input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Down)] = true;
                    break;
                case sf::Keyboard::D:
                    input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Right)] = true;
                    break;
                case sf::Keyboard::Space:
                    input.isAttackPressed = true;
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
                    input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Up)] = false;
                    break;
                case sf::Keyboard::A:
                    input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Left)] = false;
                    break;
                case sf::Keyboard::S:
                    input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Down)] = false;
                    break;
                case sf::Keyboard::D:
                    input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Right)] = false;
                    break;
                case sf::Keyboard::Space:
                    input.isAttackPressed = false;
                    break;
                default:
                    break;
                }
            }
            else if (!ImGui::GetIO().WantCaptureMouse && (event.type == sf::Event::MouseButtonPressed) &&
                     (event.mouseButton.button == sf::Mouse::Button::Left))
            {
                const Vector2f pointInWorld = camera.getWorldFromScreenCoordinates(
                    FromSfml(window.getSize()), Vector2i(event.mouseButton.x, event.mouseButton.y));
                input.selectedEnemy = FindEnemyByPosition(world, pointInWorld);
            }
        }
    }

    struct SfmlTextureManager final : TextureLoader
    {
        [[nodiscard]] std::optional<TextureId> LoadFromFile(const std::filesystem::path &textureFile) override;
        const sf::Texture &GetTexture(const TextureId &id) const;

    private:
        std::vector<sf::Texture> _textures;
    };

    std::optional<TextureId> SfmlTextureManager::LoadFromFile(const std::filesystem::path &textureFile)
    {
        sf::Texture loading;
        if (!loading.loadFromFile(textureFile.string()))
        {
            return std::nullopt;
        }
        TextureId result{AssertCast<UInt32>(_textures.size())};
        _textures.emplace_back(std::move(loading));
        return result;
    }

    const sf::Texture &SfmlTextureManager::GetTexture(const TextureId &id) const
    {
        assert(id.Value < _textures.size());
        return _textures[id.Value];
    }

    sf::Color ToSfml(const Color &value) noexcept
    {
        return sf::Color(value.Red, value.Green, value.Blue, value.Alpha);
    }

    struct SfmlCanvas final : Canvas
    {
        sf::RenderWindow &Window;
        SfmlTextureManager &Textures;
        std::vector<std::unique_ptr<sf::Text>> Texts;
        const sf::Font &Font0;

        explicit SfmlCanvas(sf::RenderWindow &window, SfmlTextureManager &textures, const sf::Font &font0)
            : Window(window)
            , Textures(textures)
            , Font0(font0)
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

        void DrawRectangle(const Vector2i &topLeft, const Vector2u &size, const Color outline, const Color fill,
                           float outlineThickness) override
        {
            sf::RectangleShape rect;
            rect.setPosition(sf::Vector2f(ToSfml(topLeft)));
            rect.setSize(sf::Vector2f(ToSfml(size)));
            rect.setFillColor(ToSfml(fill));
            rect.setOutlineColor(ToSfml(outline));
            rect.setOutlineThickness(outlineThickness);
            Window.draw(rect);
        }

        void DrawSprite(const Sprite &sprite) override
        {
            sf::Sprite sfmlSprite(Textures.GetTexture(sprite.Texture));
            sfmlSprite.setPosition(ToSfml(AssertCastVector<float>(sprite.Position)));
            sfmlSprite.setColor(ToSfml(sprite.ColorMultiplier));
            sfmlSprite.setTextureRect(sf::IntRect(ToSfml(AssertCastVector<Int32>(sprite.TextureTopLeft)),
                                                  ToSfml(AssertCastVector<Int32>(sprite.TextureSize))));
            Window.draw(sfmlSprite);
        }

        [[nodiscard]] Text CreateText(const std::string &content, FontId font, const Vector2f &position,
                                      Color fillColor, Color outlineColor, float outlineThickness) override
        {
            assert(font == 0);
            auto instance = std::make_unique<sf::Text>(content, Font0, 14);
            instance->setPosition(ToSfml(position));
            instance->setFillColor(ToSfml(fillColor));
            instance->setOutlineColor(ToSfml(outlineColor));
            instance->setOutlineThickness(outlineThickness);
            const auto foundEmptySlot = std::find(Texts.begin(), Texts.end(), nullptr);
            if (foundEmptySlot == Texts.end())
            {
                const TextId id = Texts.size();
                Texts.emplace_back(std::move(instance));
                return Text(*this, id);
            }
            const TextId id = AssertCast<size_t>(std::distance(Texts.begin(), foundEmptySlot));
            *foundEmptySlot = std::move(instance);
            return Text(*this, id);
        }

        void SetTextPosition(TextId id, const Vector2f &position) override
        {
            assert(id < Texts.size());
            const auto &slot = Texts[id];
            assert(slot);
            slot->setPosition(ToSfml(position));
        }

        Vector2f GetTextPosition(TextId id) override
        {
            assert(id < Texts.size());
            const auto &slot = Texts[id];
            assert(slot);
            return FromSfml(slot->getPosition());
        }

        void DeleteText(TextId id) override
        {
            assert(id < Texts.size());
            auto &slot = Texts[id];
            assert(slot);
            slot.reset();
        }

        void DrawText(TextId id) override
        {
            assert(id < Texts.size());
            const auto &slot = Texts[id];
            assert(slot);
            Window.draw(*slot);
        }

        void SetView(const Rectangle<float> &view) override
        {
            Window.setView(sf::View(ToSfml(view.Position + (view.Size / 2.0f)), ToSfml(view.Size)));
        }
    };

    struct SfmlWindowFunctions : WindowFunctions
    {
        explicit SfmlWindowFunctions(sf::RenderWindow &sfml)
            : _sfml(sfml)
        {
        }

        [[nodiscard]] bool IsOpen() override
        {
            return _sfml.isOpen();
        }

        void ProcessEvents(Input &input, const Camera &camera, World &world) override
        {
            ij::ProcessEvents(input, _sfml, camera, world);
        }

        void UpdateGui(TimeSpan deltaTime) override
        {
            ImGui::SFML::Update(_sfml, sf::milliseconds(AssertCast<Int32>(deltaTime.Milliseconds)));
        }

        void Clear() override
        {
            _sfml.clear();
        }

        void RenderGui() override
        {
            ImGui::SFML::Render(_sfml);
        }

        void Display() override
        {
            _sfml.display();
        }

        [[nodiscard]] TimeSpan RestartDeltaClock() override
        {
            // TODO: account for the microseconds lost by rounding
            return TimeSpan::FromMilliseconds(_deltaClock.restart().asMilliseconds());
        }

    private:
        sf::RenderWindow &_sfml;
        sf::Clock _deltaClock;
    };
} // namespace ij

int main()
{
    using namespace ij;
    sf::RenderWindow window(sf::VideoMode(1200, 800), "Improved Journey");
    window.setFramerateLimit(FrameRate);
    if (!ImGui::SFML::Init(window))
    {
        std::cerr << "Could not initialize ImGui::SFML\n";
        return false;
    }

    const std::filesystem::path assets =
        std::filesystem::current_path().parent_path().parent_path() / "improved-journey" / "assets";

    sf::Font font;
    if (!font.loadFromFile((assets / "Roboto-Font" / "Roboto-Light.ttf").string()))
    {
        std::cerr << "Could not load font\n";
        return false;
    }

    SfmlTextureManager textures;
    SfmlCanvas canvas{window, textures, font};
    SfmlWindowFunctions windowFunctions{window};
    const bool success = RunGame(textures, canvas, assets, windowFunctions);
    ImGui::SFML::Shutdown();
    return !success;
}
