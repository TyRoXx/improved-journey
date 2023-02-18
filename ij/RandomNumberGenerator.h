#pragma once
#include "Int.h"
#include <random>

namespace ij
{
    struct RandomNumberGenerator
    {
        virtual ~RandomNumberGenerator();
        virtual Int32 GenerateInt32(Int32 minimum, Int32 maximum) = 0;
        virtual size_t GenerateSize(size_t minimum, size_t maximum) = 0;
    };

    struct StandardRandomNumberGenerator final : RandomNumberGenerator
    {
        std::random_device seedGenerator;
        std::default_random_engine engine;

        StandardRandomNumberGenerator();
        Int32 GenerateInt32(Int32 minimum, Int32 maximum) override;
        size_t GenerateSize(size_t minimum, size_t maximum) override;
    };
} // namespace ij
