#pragma once
#include "Int.h"

namespace ij
{
    struct TimeSpan final
    {
        Int64 Milliseconds;

        [[nodiscard]] static TimeSpan FromMilliseconds(Int64 milliseconds) noexcept;

    private:
        TimeSpan(Int64 milliseconds) noexcept;
    };

    [[nodiscard]] bool operator>=(TimeSpan left, TimeSpan right) noexcept;
    TimeSpan &operator+=(TimeSpan &left, TimeSpan right) noexcept;
    TimeSpan &operator-=(TimeSpan &left, TimeSpan right) noexcept;
} // namespace ij
