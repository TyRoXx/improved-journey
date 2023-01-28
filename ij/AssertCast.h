#pragma once
#include <cassert>

namespace ij
{
    template <class To, class From>
    [[nodiscard]] To AssertCast(const From &from)
    {
        assert(from == static_cast<From>(static_cast<To>(from)));
        return static_cast<To>(from);
    }
} // namespace ij
