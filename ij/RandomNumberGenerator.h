#pragma once
#include <SFML/Config.hpp>
#include <random>

namespace ij
{
    struct RandomNumberGenerator
    {
        virtual ~RandomNumberGenerator();
        virtual sf::Int32 GenerateInt32(sf::Int32 minimum, sf::Int32 maximum) = 0;
        virtual size_t GenerateSize(size_t minimum, size_t maximum) = 0;
    };

    struct StandardRandomNumberGenerator final : RandomNumberGenerator
    {
        std::random_device seedGenerator;
        std::default_random_engine engine;

        StandardRandomNumberGenerator();
        sf::Int32 GenerateInt32(sf::Int32 minimum, sf::Int32 maximum) override;
        size_t GenerateSize(size_t minimum, size_t maximum) override;
    };
} // namespace ij
