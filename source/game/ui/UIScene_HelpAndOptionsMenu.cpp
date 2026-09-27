#include "game/ui/UIScene_HelpAndOptionsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/GameSettings.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

UIScene_HelpAndOptionsMenu::UIScene_HelpAndOptionsMenu(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer), m_unknownFlag(false)
{
    initialiseMovie();
    m_unknownFlag = Minecraft::GetInstance()->mPendingLevelChange == 0;
    m_button1.init(UIString(IDS_CHANGE_SKIN), 0);
    m_button2.init(UIString(IDS_HOW_TO_PLAY), 1);
    m_button3.init(UIString(IDS_CONTROLS), 2);
    m_button4.init(UIString(IDS_SETTINGS), 3);
    m_button5.init(UIString(IDS_CREDITS), 4);
    m_button7.init(UIString(IDS_DEBUG_SETTINGS), 6);
    removeControl(&m_buttonUpsell, false);
    removeControl(&m_button6, false);
    removeControl(&m_button7, false);
    if (m_playerId != gProfile->GetPrimaryPad() || !m_unknownFlag)
        removeControl(&m_button6, false);
    if (g_pApp->GetLocalPlayerCount() > 1) {
        removeControl(&m_button5, false);
        if (m_playerId != gProfile->GetPrimaryPad())
            removeControl(&m_button6, false);
    }
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) && !g_pApp->DebugArtToolsOn())
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
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_HelpAndOptionsMenu::updateComponents()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    bool showSecond = false;
    if (minecraft->mPendingLevelChange == 0) {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        showSecond = true;
    } else {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        showSecond = g_pApp->GetLocalPlayerCount() == 1;
    }
    m_layer->showComponent(m_playerId, EUIScene_Logo, showSecond);
}

std::wstring UIScene_HelpAndOptionsMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"HelpAndOptionsMenu") : std::wstring(L"HelpAndOptionsMenuSplit");
}

void UIScene_HelpAndOptionsMenu::handleReload()
{
    removeControl(&m_button7, false);
    Minecraft* minecraft = Minecraft::GetInstance();
    if (m_playerId != gProfile->GetPrimaryPad() || minecraft->mPendingLevelChange != 0)
        removeControl(&m_button6, false);
    if (g_pApp->GetLocalPlayerCount() > 1) {
        removeControl(&m_button5, false);
        if (gProfile->GetPrimaryPad() != m_playerId)
            removeControl(&m_button6, false);
    }
    gProfile->IsFullVersion();
    bool keepButton1 = gProfile->IsFullVersion() &&
        Minecraft::InMiniGame(eMiniGameId_Any, true) &&
        minecraft->mLocalPlayers[m_playerId]->GetGameType() == GameType::LOBBY;
    if (!keepButton1)
        removeControl(&m_button1, false);
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_HelpAndOptionsMenu::handleInput(int, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (action < EControllerActions_MenuUp)
        return;
    if (action == EControllerActions_MenuCancel)
    {
        if (pressed && !released)
            navigateBack();
        return;
    }
    if (action == EControllerActions_MenuOk && pressed)
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    sendInputToMovie(action, repeat, released, pressed);
}

void UIScene_HelpAndOptionsMenu::handlePress(int controlId, int)
{
    if (controlId > 6)
        return;
    static const EUIScene scenes[7] = {
        EUIScene_SkinSelectMenu, EUIScene_HowToPlayMenu, EUIScene_ControlsMenu,
        EUIScene_SettingsMenu, EUIScene_Credits, EUIScene_TutorialPopup,
        static_cast<EUIScene>(0x0d)
    };
    gConsoleUIController.NavigateToScene(m_playerId, scenes[controlId], 0, EUILayer_Scene, EUIGroup_PlayerDefault);
}

int UIScene_HelpAndOptionsMenu::getSceneType() { return EUIScene_HelpAndOptionsMenu; }
