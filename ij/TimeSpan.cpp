#include "TimeSpan.h"

ij::TimeSpan ij::TimeSpan::FromMilliseconds(Int64 milliseconds) noexcept
{
    return TimeSpan{milliseconds};
}

ij::TimeSpan::TimeSpan(Int64 milliseconds) noexcept
    : Milliseconds(milliseconds)
{
}

bool ij::operator>=(TimeSpan left, TimeSpan right) noexcept
{
    return (left.Milliseconds >= right.Milliseconds);
}

ij::TimeSpan &ij::operator+=(TimeSpan &left, TimeSpan right) noexcept
{
    left.Milliseconds += right.Milliseconds;
    return left;
}

ij::TimeSpan &ij::operator-=(TimeSpan &left, TimeSpan right) noexcept
{
    left.Milliseconds -= right.Milliseconds;
    return left;
}
