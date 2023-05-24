#include "FloatingText.h"
#include "AssertCast.h"

ij::FloatingText::FloatingText(Canvas &canvas, const std::string &text, const Vector2f &position, FontId font,
                               RandomNumberGenerator &random)
    : VisualItem(canvas.CreateText(text, font,
                                   position + Vector2f(AssertCast<float>(20 - random.GenerateInt32(0, 39)),
                                                       AssertCast<float>(-100 + random.GenerateInt32(0, 39))),
                                   Color(255, 0, 0, 255), Color(0, 0, 0, 255), 1))
    , Age(TimeSpan::FromMilliseconds(0))
    , MaxAge(TimeSpan::FromMilliseconds(random.GenerateInt32(5000, 10'000)))
{
}

void ij::FloatingText::Update(const TimeSpan deltaTime)
{
    Age += deltaTime;
    VisualItem.SetPosition(VisualItem.GetPosition() +
                           Vector2f(0, AssertCast<float>(deltaTime.Milliseconds) / 1000.0f * -10));
}

bool ij::FloatingText::HasExpired() const
{
    return (Age >= MaxAge);
}
