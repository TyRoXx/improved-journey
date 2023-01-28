#include "ObjectAnimation.h"
#include "Unreachable.h"

[[nodiscard]] const char *ij::GetObjectAnimationName(const ObjectAnimation animation)
{
    switch (animation)
    {
    case ObjectAnimation::Standing:
        return "Standing";
    case ObjectAnimation::Walking:
        return "Walking";
    case ObjectAnimation::Attacking:
        return "Attacking";
    case ObjectAnimation::Dead:
        return "Dead";
    }
    IJ_UNREACHABLE();
}
