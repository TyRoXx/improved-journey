#include "RandomNumberGenerator.h"

ij::RandomNumberGenerator::~RandomNumberGenerator() = default;

ij::StandardRandomNumberGenerator::StandardRandomNumberGenerator()
    : engine(seedGenerator())
{
}

ij::Int32 ij::StandardRandomNumberGenerator::GenerateInt32(Int32 minimum, Int32 maximum)
{
    std::uniform_int_distribution<Int32> distribution(minimum, maximum);
    return distribution(engine);
}

size_t ij::StandardRandomNumberGenerator::GenerateSize(size_t minimum, size_t maximum)
{
    std::uniform_int_distribution<size_t> distribution(minimum, maximum);
    return distribution(engine);
}
