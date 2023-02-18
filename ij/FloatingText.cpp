#include "FloatingText.h"
#include "AssertCast.h"
#include "ToSfml.h"

ij::FloatingText::FloatingText(const sf::String &text, const Vector2f &position, const sf::Font &font,
                               RandomNumberGenerator &random)
    : Text(std::make_unique<sf::Text>(text, font, 14u))
    , Age(TimeSpan::FromMilliseconds(0))
    , MaxAge(TimeSpan::FromMilliseconds(random.GenerateInt32(5000, 10'000)))
{
    Text->setPosition(ToSfml(position + Vector2f(AssertCast<float>(20 - random.GenerateInt32(0, 39)),
                                                 AssertCast<float>(-100 + random.GenerateInt32(0, 39)))));
    Text->setFillColor(sf::Color::Red);
    Text->setOutlineColor(sf::Color::Black);
    Text->setOutlineThickness(1);
}

void ij::FloatingText::Update(const TimeSpan deltaTime)
{
    Age += deltaTime;
    Text->setPosition(Text->getPosition() + sf::Vector2f(0, AssertCast<float>(deltaTime.Milliseconds) / 1000.0f * -10));
}

bool ij::FloatingText::HasExpired() const
{
    return (Age >= MaxAge);
}
