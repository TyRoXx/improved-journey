#pragma once
#include "RandomNumberGenerator.h"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

namespace ij
{
    struct FloatingText final
    {
        std::unique_ptr<sf::Text> Text;
        sf::Time Age;
        sf::Time MaxAge;

        explicit FloatingText(const sf::String &text, const sf::Vector2f &position, const sf::Font &font,
                              RandomNumberGenerator &random);
        void Update(const sf::Time &deltaTime);
        bool HasExpired() const;
    };
} // namespace ij
