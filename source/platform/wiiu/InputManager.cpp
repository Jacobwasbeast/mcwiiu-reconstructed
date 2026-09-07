#include "platform/wiiu/InputManager.hpp"
#include "platform/wiiu/CInput.hpp"

bool InputManager_IsShowingErrorViewer()
{
    return g_pInput->IsShowingErrorViewer() != 0;
}

bool InputManager_IsShowingOnscreenKeyboard()
{
    return g_pInput->IsShowingOnscreenKeyboard() != 0;
}
