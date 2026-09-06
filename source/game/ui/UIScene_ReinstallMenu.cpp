#include "game/ui/UIScene_ReinstallMenu.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"

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

    m_button1.setupControl(this, root, std::string("Button1"));
    m_controls.push_back(&m_button1);
    m_button2.setupControl(this, root, std::string("Button2"));
    m_controls.push_back(&m_button2);
    m_button3.setupControl(this, root, std::string("Button3"));
    m_controls.push_back(&m_button3);
    m_button4.setupControl(this, root, std::string("Button4"));
    m_controls.push_back(&m_button4);
    m_button5.setupControl(this, root, std::string("Button5"));
    m_controls.push_back(&m_button5);
    m_button6.setupControl(this, root, std::string("Button6"));
    m_controls.push_back(&m_button6);

    return 1;
}

void UIScene_ReinstallMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_sceneType, 0x8d6323ed, 0x4f6bcf5, 0x21c0a77,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, false, false);
}

void UIScene_ReinstallMenu::updateComponents()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mPendingLevelChange == 0)
    {
        m_layer->showComponent(m_sceneType, static_cast<EUIScene>(0x42), true);
        m_layer->showComponent(m_sceneType, static_cast<EUIScene>(0x43), true);
    }
    else
    {
        m_layer->showComponent(m_sceneType, static_cast<EUIScene>(0x42), false);
        m_layer->showComponent(m_sceneType, static_cast<EUIScene>(0x43), false);
    }
}

std::wstring UIScene_ReinstallMenu::getMoviePath()
{
    if (g_pApp->GetLocalPlayerCount() < 2)
        return std::wstring(L"ReinstallMenu");
    return std::wstring(L"ReinstallSplit");
}

void UIScene_ReinstallMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                        bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
    if (action > 3)
    {
        if (action < 6 || action == 0x14)
            sendInputToMovie(action, repeat, released, pressed);
        else if (action == 0x15 && pressed && !released)
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
