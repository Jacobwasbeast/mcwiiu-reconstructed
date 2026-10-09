#include "platform/wiiu/InputManager.hpp"
#include "platform/wiiu/CInput.hpp"

bool InputManager_IsShowingErrorViewer()
{
    return g_pInputInstance->IsShowingErrorViewer() != 0;
}

bool InputManager_IsShowingOnscreenKeyboard()
{
    return g_pInputInstance->IsShowingOnscreenKeyboard() != 0;
}
