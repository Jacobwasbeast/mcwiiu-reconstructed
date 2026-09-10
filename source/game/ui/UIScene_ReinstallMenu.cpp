#include "game/ui/UIScene_ReinstallMenu.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

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
    gConsoleUIController.SetTooltips(m_playerId, (int)0x8d6323ed, 0x4f6bcf5e, 0x21c0a77d,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, false, false);
}

void UIScene_ReinstallMenu::updateComponents()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mPendingLevelChange == 0)
    {
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x42), true);
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x43), true);
    }
    else
    {
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x42), false);
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x43), false);
    }
}

std::wstring UIScene_ReinstallMenu::getMoviePath()
{
    if (g_pApp->GetLocalPlayerCount() < 2)
        return std::wstring(L"ReinstallMenu");
    return std::wstring(L"ReinstallSplit");
}

void UIScene_ReinstallMenu::handleInput(int, int action, bool repeat, bool released,
                                        bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (action > 3)
    {
        if (action < 6 || action == 0x14)
            sendInputToMovie(action, repeat, released, pressed);
        else if (action == 0x15 && released && !repeat)
            navigateBack();
    }
}

void UIScene_ReinstallMenu::handlePress(int, int)
{
}

int UIScene_ReinstallMenu::getSceneType()
{
    return 37;
}
