#pragma once
#include <cstdint>

namespace ij
{
    struct Color final
    {
        std::uint8_t Red, Green, Blue, Alpha;

        Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) noexcept;
    };

} // namespace ij
