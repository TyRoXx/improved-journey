#pragma once
#include "RandomNumberGenerator.h"
#include "TimeSpan.h"
#include "Vector2.h"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>

namespace ij
{
    struct FloatingText final
    {
        std::unique_ptr<sf::Text> Text;
        TimeSpan Age;
        TimeSpan MaxAge;

        explicit FloatingText(const sf::String &text, const Vector2f &position, const sf::Font &font,
                              RandomNumberGenerator &random);
        void Update(TimeSpan deltaTime);
        [[nodiscard]] bool HasExpired() const;
    };
} // namespace ij
