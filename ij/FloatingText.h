#pragma once
#include "RandomNumberGenerator.h"
#include "Vector2.h"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Time.hpp>

namespace ij
{
    struct FloatingText final
    {
        std::unique_ptr<sf::Text> Text;
        sf::Time Age;
        sf::Time MaxAge;

        explicit FloatingText(const sf::String &text, const Vector2f &position, const sf::Font &font,
                              RandomNumberGenerator &random);
        void Update(const sf::Time &deltaTime);
        [[nodiscard]] bool HasExpired() const;
    };
} // namespace ij
