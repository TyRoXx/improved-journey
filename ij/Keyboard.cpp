#include "Keyboard.h"

void ij::keyboard::UpdateInput(Input &input, const Event &event)
{
    if (event.IsDown)
    {
        switch (event.PressedKey)
        {
        case Key::W:
            input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Up)] = true;
            break;
        case Key::A:
            input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Left)] = true;
            break;
        case Key::S:
            input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Down)] = true;
            break;
        case Key::D:
            input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Right)] = true;
            break;
        case Key::Space:
            input.isAttackPressed = true;
            break;
        case Key::F1:
            break;
        }
    }
    else
    {
        switch (event.PressedKey)
        {
        case Key::W:
            input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Up)] = false;
            break;
        case Key::A:
            input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Left)] = false;
            break;
        case Key::S:
            input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Down)] = false;
            break;
        case Key::D:
            input.isDirectionKeyPressed[AssertCast<size_t>(Direction::Right)] = false;
            break;
        case Key::Space:
            input.isAttackPressed = false;
            break;
        case Key::F1:
            input.isDebugModeOn = !input.isDebugModeOn;
            break;
        }
    }
}
