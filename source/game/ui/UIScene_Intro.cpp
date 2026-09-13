#include "game/ui/UIScene_Intro.hpp"

#include <string>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_Intro.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern "C" void OSReport(const char*, ...);
#define INTRO_TRACE(step) OSReport("[boot][intro_trace] %s\n", step)

uint8_t g_introRunning = 0;

UIScene_Intro::UIScene_Intro(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer)
{
    INTRO_TRACE("ctor entered");
    initialiseMovie();
    INTRO_TRACE("initialiseMovie done");
    m_introFocus = false;
    m_inputHandled = false;
    INTRO_TRACE("flags set");
    setIntroPlatform();
    INTRO_TRACE("setIntroPlatform done");
    m_touchControl.init(0);
}

UIScene_Intro::~UIScene_Intro()
{
}

void UIScene_Intro::setIntroPlatform()
{
    fuiRenderNode* root = m_movie->getRootNode();
    FJ_Intro* intro = static_cast<FJ_Intro*>(root->getFuiNode());
    intro->SetIntroPlatform(6, true, false);
}

std::wstring UIScene_Intro::getMoviePath()
{
    return std::wstring(L"Intro");
}

void UIScene_Intro::handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                                bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (action != 20 || m_inputHandled)
        return;

    m_inputHandled = true;
    if (gApp.HasNoSpaceForSaveQuota() == 0)
    {
        int primaryPlayer = g_pApp->GetPrimaryUserIndex();
        uint32_t optionsStatus = g_pApp->GetOptionsCallbackStatus(primaryPlayer);
        if (optionsStatus == 0)
        {
            m_inputHandled = false;
            g_introRunning = 1;
            return;
        }
        if (optionsStatus == 4 || optionsStatus == 6)
        {
            if (GameSettings::GetGameSettings(primaryPlayer, static_cast<eGameSetting>(0x1e)) == 0)
                gConsoleUIController.NavigateToScene(primaryPlayer, static_cast<EUIScene>(0x4d), NULL,
                                                      static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
            else
                gConsoleUIController.NavigateToScene(primaryPlayer, static_cast<EUIScene>(2), NULL,
                                                      static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
            g_introRunning = 1;
            return;
        }
        gConsoleUIController.NavigateToScene(primaryPlayer, static_cast<EUIScene>(0x4d), NULL,
                                              static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    }
    g_introRunning = 1;
}

void UIScene_Intro::handleAnimationEnd()
{
    g_introRunning = 1;
    if (m_inputHandled)
        return;

    m_inputHandled = true;
    if (gApp.HasNoSpaceForSaveQuota() != 0)
        return;

    int primaryPlayer = g_pApp->GetPrimaryUserIndex();
    uint32_t optionsStatus = g_pApp->GetOptionsCallbackStatus(primaryPlayer);
    if (optionsStatus == 0)
    {
        gConsoleUIController.NavigateToScene(primaryPlayer, static_cast<EUIScene>(2), NULL,
                                              static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
        return;
    }
    if (optionsStatus == 4 || optionsStatus == 6)
    {
        if (GameSettings::GetGameSettings(primaryPlayer, static_cast<eGameSetting>(0x1e)) == 0)
        {
            gConsoleUIController.NavigateToScene(primaryPlayer, static_cast<EUIScene>(0x4d), NULL,
                                                  static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
            return;
        }
        gConsoleUIController.NavigateToScene(primaryPlayer, static_cast<EUIScene>(2), NULL,
                                              static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
        return;
    }
    gConsoleUIController.NavigateToScene(primaryPlayer, static_cast<EUIScene>(0x4d), NULL,
                                          static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
}

void UIScene_Intro::handleGainFocus(bool)
{
    if (!m_introFocus)
        return;
    gConsoleUIController.NavigateToScene(0, static_cast<EUIScene>(3), NULL, static_cast<EUILayer>(6),
                                          static_cast<EUIGroup>(7));
}

void UIScene_Intro::handleReload()
{
    setIntroPlatform();
}

void UIScene_Intro::handleTouchInput(unsigned int playerId, int, int, int, bool, bool, bool pressed)
{
    if (pressed)
    {
        bool handled = false;
        handleInput(playerId, 20, false, true, false, handled);
    }
}

unsigned int UIScene_Intro::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    std::string name("TouchToSkip");
    m_touchControl.setupControl(this, root, name);
    m_controls.push_back(&m_touchControl);
    return 1;
}

int UIScene_Intro::getSceneType()
{
    return 1;
}

bool UIScene_Intro::hasFocus(int)
{
    return m_hasFocus;
}
