#include "Color.h"

ij::Color::Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) noexcept
    : Red(red)
    , Green(green)
    , Blue(blue)
    , Alpha(alpha)
{
}
