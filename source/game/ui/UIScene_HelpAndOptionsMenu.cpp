#include "game/ui/UIScene_HelpAndOptionsMenu.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

UIScene_HelpAndOptionsMenu::UIScene_HelpAndOptionsMenu(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer), m_unknownFlag(false)
{
    initialiseMovie();
    m_unknownFlag = Minecraft::GetInstance()->mPendingLevelChange == 0;
    m_button1.init(UIString(0x5268d042), 0);
    m_button2.init(UIString(0x15f695fb), 1);
    m_button3.init(UIString(0x80fed3f0), 2);
    m_button4.init(UIString(0xfccd64b9), 3);
    m_button5.init(UIString(0x4e960eb2), 4);
    m_button7.init(UIString(0x7693b9cf), 6);
    removeControl(&m_buttonUpsell, false);
    removeControl(&m_button6, false);
    removeControl(&m_button7, false);
    if (m_playerId != Minecraft::GetInstance()->mLocalPlayerIdx || !m_unknownFlag)
        removeControl(&m_button6, false);
    if (g_pApp->GetLocalPlayerCount() > 1) {
        removeControl(&m_button5, false);
        if (m_playerId != Minecraft::GetInstance()->mLocalPlayerIdx)
            removeControl(&m_button6, false);
    }
    if (!Minecraft::InMiniGame(static_cast<EMiniGameId>(15), true) && !g_pApp->DebugArtToolsOn())
        removeControl(&m_button1, false);
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_HelpAndOptionsMenu::~UIScene_HelpAndOptionsMenu() {}

unsigned int UIScene_HelpAndOptionsMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    const char* names[] = {"Button1", "Button2", "Button3", "Button4", "Button5", "Button6", "Button7", "ButtonUpsell"};
    UIControl_Button* buttons[] = {&m_button1, &m_button2, &m_button3, &m_button4, &m_button5, &m_button6, &m_button7, &m_buttonUpsell};
    for (int i = 0; i < 8; ++i) {
        buttons[i]->setupControl(this, root, std::string(names[i]));
        m_controls.push_back(buttons[i]);
    }
    return 1;
}

void UIScene_HelpAndOptionsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, 0x8d6323ed, 0x4f6bcf5, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_HelpAndOptionsMenu::updateComponents()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    bool showSecond = false;
    if (minecraft->mPendingLevelChange == 0) {
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x42), true);
        showSecond = true;
    } else {
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x42), false);
        showSecond = g_pApp->GetLocalPlayerCount() == 1;
    }
    m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x43), showSecond);
}

std::wstring UIScene_HelpAndOptionsMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"HelpAndOptionsMenu") : std::wstring(L"HelpAndOptionsMenuSplit");
}

void UIScene_HelpAndOptionsMenu::handleReload()
{
    removeControl(&m_button7, false);
    Minecraft* minecraft = Minecraft::GetInstance();
    if (m_playerId != minecraft->mLocalPlayerIdx || minecraft->mPendingLevelChange != 0)
        removeControl(&m_button6, false);
    if (g_pApp->GetLocalPlayerCount() > 1) {
        removeControl(&m_button5, false);
        if (m_playerId != minecraft->mLocalPlayerIdx)
            removeControl(&m_button6, false);
    }
    if (!Minecraft::InMiniGame(static_cast<EMiniGameId>(15), true))
        removeControl(&m_button1, false);
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_HelpAndOptionsMenu::handleInput(int, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (action > 3) {
        if (action > 5 && action == 0x14 && pressed)
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        if (action == 0x15) {
            if (!pressed || released)
                return;
            navigateBack();
            return;
        }
        sendInputToMovie(action, released, pressed, handled);
    }
}

void UIScene_HelpAndOptionsMenu::handlePress(int first, int)
{
    if (first > 6)
        return;
    static const EUIScene scenes[7] = {
        static_cast<EUIScene>(0x26), static_cast<EUIScene>(0x11), static_cast<EUIScene>(0x12),
        static_cast<EUIScene>(0x15), static_cast<EUIScene>(0x17), static_cast<EUIScene>(0x19),
        static_cast<EUIScene>(0x0d)
    };
    gConsoleUIController.NavigateToScene(m_playerId, scenes[first], 0, static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
}

int UIScene_HelpAndOptionsMenu::getSceneType() { return 15; }
