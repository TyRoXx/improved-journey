#pragma once
#include "Input.h"

namespace ij
{
    namespace keyboard
    {
        enum class Key
        {
            W,
            A,
            S,
            D,
            Space
        };

        struct Event final
        {
            Key PressedKey;
            bool IsDown;

            Event(Key pressedKey, bool isDown) noexcept
                : PressedKey(pressedKey)
                , IsDown(isDown)
            {
            }
        };

        void UpdateInput(Input &input, const Event &event);
    } // namespace keyboard
} // namespace ij
