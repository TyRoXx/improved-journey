#include "RandomNumberGenerator.h"

ij::RandomNumberGenerator::~RandomNumberGenerator()
{
}

ij::StandardRandomNumberGenerator::StandardRandomNumberGenerator()
    : engine(seedGenerator())
{
}

sf::Int32 ij::StandardRandomNumberGenerator::GenerateInt32(sf::Int32 minimum, sf::Int32 maximum)
{
    std::uniform_int_distribution<sf::Int32> distribution(minimum, maximum);
    return distribution(engine);
}

size_t ij::StandardRandomNumberGenerator::GenerateSize(size_t minimum, size_t maximum)
{
    std::uniform_int_distribution<size_t> distribution(minimum, maximum);
    return distribution(engine);
}
