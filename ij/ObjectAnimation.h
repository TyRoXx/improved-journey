#pragma once

namespace ij
{
    enum class ObjectAnimation
    {
        Standing,
        Walking,
        Attacking,
        Dead
    };

    [[nodiscard]] const char *GetObjectAnimationName(ObjectAnimation animation);
} // namespace ij
