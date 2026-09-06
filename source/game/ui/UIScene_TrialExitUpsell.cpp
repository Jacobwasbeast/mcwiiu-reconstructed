#include "game/ui/UIScene_TrialExitUpsell.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"

UIScene_TrialExitUpsell::UIScene_TrialExitUpsell(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer)
{
    initialiseMovie();
}

UIScene_TrialExitUpsell::~UIScene_TrialExitUpsell()
{
}

void UIScene_TrialExitUpsell::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, 0x2686c8e5, 0x4f6bcf5, 0xc6058d6b,
                                     -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, false, false);
}

std::wstring UIScene_TrialExitUpsell::getMoviePath()
{
    return L"TrialExitUpsell";
}

void UIScene_TrialExitUpsell::handleInput(int playerId, int action, bool repeat, bool released,
                                          bool pressed, bool&)
{
    gConsoleUIController.AnimateKeyPress(m_resolution, action, repeat, released, pressed);
    if (action == 20)
    {
        if (pressed)
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            theApp.ExitGame();
            return;
        }
    }
    else if (action == 21)
    {
        navigateBack();
        return;
    }
    else if (action == 2)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    }
}

void UIScene_TrialExitUpsell::handleAnimationEnd()
{
    gConsoleUIController.NavigateToScene(0, static_cast<EUIScene>(2), 0,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
}

int UIScene_TrialExitUpsell::getSceneType()
{
    return 0x22;
}

bool UIScene_TrialExitUpsell::hasFocus(int)
{
    return m_hasFocus;
}
