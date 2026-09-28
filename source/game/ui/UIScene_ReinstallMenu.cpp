#include "game/ui/UIScene_ReinstallMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/GameSettings.hpp"

UIScene_ReinstallMenu::UIScene_ReinstallMenu(int type, void*, UILayer* layer)
    : UIScene(type, layer)
{
    initialiseMovie();
}

UIScene_ReinstallMenu::~UIScene_ReinstallMenu()
{
}

unsigned int UIScene_ReinstallMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();

    static const char* const names[6] = { "Button1", "Button2", "Button3",
                                          "Button4", "Button5", "Button6" };
    for (int i = 0; i < 6; ++i)
    {
        m_buttons[i].setupControl(this, root, std::string(names[i]));
        m_controls.push_back(&m_buttons[i]);
    }

    return 1;
}

void UIScene_ReinstallMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, (int)IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, IDS_TOOLTIPS_SELECTDEVICE,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, false, false);
}

void UIScene_ReinstallMenu::updateComponents()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mPendingLevelChange == 0)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        m_layer->showComponent(m_playerId, EUIScene_Logo, true);
    }
    else
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        m_layer->showComponent(m_playerId, EUIScene_Logo, false);
    }
}

std::wstring UIScene_ReinstallMenu::getMoviePath()
{
    if (g_pApp->GetLocalPlayerCount() < 2)
        return std::wstring(L"ReinstallMenu");
    return std::wstring(L"ReinstallSplit");
}

void UIScene_ReinstallMenu::handleInput(int, int action, bool repeat, bool pressed,
                                        bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    case EControllerActions_MenuCancel:
        if (pressed && !repeat)
            navigateBack();
        break;
    }
}

void UIScene_ReinstallMenu::handlePress(int, int)
{
}

int UIScene_ReinstallMenu::getSceneType()
{
    return EUIScene_ReinstallMenu;
}
