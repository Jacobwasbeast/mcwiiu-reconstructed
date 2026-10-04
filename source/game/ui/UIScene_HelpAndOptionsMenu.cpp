#include "game/ui/UIScene_HelpAndOptionsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
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

UIScene_HelpAndOptionsMenu::UIScene_HelpAndOptionsMenu(int playerId, void*, UILayer* layer)
    : UIScene(playerId, layer)
{
    initialiseMovie();
    m_unknownFlag = Minecraft::GetInstance()->mLevel == NULL;
    m_buttons[0].init(UIString(IDS_CHANGE_SKIN), 0);
    m_buttons[1].init(UIString(IDS_HOW_TO_PLAY), 1);
    m_buttons[2].init(UIString(IDS_CONTROLS), 2);
    m_buttons[3].init(UIString(IDS_SETTINGS), 3);
    m_buttons[4].init(UIString(IDS_CREDITS), 4);
    m_buttons[6].init(UIString(IDS_DEBUG_SETTINGS), 6);
    removeControl(&m_buttons[7], false);
    removeControl(&m_buttons[5], false);
    removeControl(&m_buttons[6], false);
    if (m_playerId != gProfile->GetPrimaryPad() || !m_unknownFlag)
        removeControl(&m_buttons[5], false);
    if (gApp.GetLocalPlayerCount() > 1) {
        removeControl(&m_buttons[4], false);
        if (m_playerId != gProfile->GetPrimaryPad())
            removeControl(&m_buttons[5], false);
    }
    Minecraft* minecraft = Minecraft::GetInstance();
    if (!gProfile->IsFullVersion()
        || (Minecraft::InMiniGame(eMiniGameId_Any, true) && minecraft->mLocalPlayers[playerId].get() != NULL
            && minecraft->mLocalPlayers[playerId].get()->GetGameType() != GameType::LOBBY
            && !gApp.DebugArtToolsOn()))
        removeControl(&m_buttons[0], false);
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_HelpAndOptionsMenu::~UIScene_HelpAndOptionsMenu() {}

unsigned int UIScene_HelpAndOptionsMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    {
        m_buttons[0].setupControl(this, root, std::string("Button1"));
        m_controls.push_back(&m_buttons[0]);
    }
    {
        m_buttons[1].setupControl(this, root, std::string("Button2"));
        m_controls.push_back(&m_buttons[1]);
    }
    {
        m_buttons[2].setupControl(this, root, std::string("Button3"));
        m_controls.push_back(&m_buttons[2]);
    }
    {
        m_buttons[3].setupControl(this, root, std::string("Button4"));
        m_controls.push_back(&m_buttons[3]);
    }
    {
        m_buttons[4].setupControl(this, root, std::string("Button5"));
        m_controls.push_back(&m_buttons[4]);
    }
    {
        m_buttons[5].setupControl(this, root, std::string("Button6"));
        m_controls.push_back(&m_buttons[5]);
    }
    {
        m_buttons[6].setupControl(this, root, std::string("Button7"));
        m_controls.push_back(&m_buttons[6]);
    }
    {
        m_buttons[7].setupControl(this, root, std::string("ButtonUpsell"));
        m_controls.push_back(&m_buttons[7]);
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
    if (minecraft->mLevel == NULL) {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        showSecond = true;
    } else {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        showSecond = gApp.GetLocalPlayerCount() == 1;
    }
    m_layer->showComponent(m_playerId, EUIScene_Logo, showSecond);
}

std::wstring UIScene_HelpAndOptionsMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"HelpAndOptionsMenuSplit") : std::wstring(L"HelpAndOptionsMenu");
}

void UIScene_HelpAndOptionsMenu::handleReload()
{
    removeControl(&m_buttons[6], false);
    Minecraft* minecraft = Minecraft::GetInstance();
    if (m_playerId != gProfile->GetPrimaryPad() || minecraft->mLevel != NULL)
        removeControl(&m_buttons[5], false);
    if (gApp.GetLocalPlayerCount() > 1) {
        removeControl(&m_buttons[4], false);
        if (gProfile->GetPrimaryPad() != m_playerId)
            removeControl(&m_buttons[5], false);
    }
    gProfile->IsFullVersion();
    bool keepButton1 = gProfile->IsFullVersion() &&
        (!Minecraft::InMiniGame(eMiniGameId_Any, true) ||
         minecraft->mLocalPlayers[m_playerId]->GetGameType() == GameType::LOBBY);
    if (!keepButton1)
        removeControl(&m_buttons[0], false);
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_HelpAndOptionsMenu::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed && !repeat)
            navigateBack();
        break;
    case EControllerActions_MenuOk:
        if (pressed)
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        sendInputToMovie(action, repeat, pressed, released);
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_HelpAndOptionsMenu::handlePress(int controlId, int)
{
    if (static_cast<unsigned int>(controlId) > 6)
        return;
    static const unsigned char scenes[7] = {
        EUIScene_SkinSelectMenu, EUIScene_HowToPlayMenu, EUIScene_ControlsMenu,
        EUIScene_SettingsMenu, EUIScene_Credits, EUIScene_ReinstallMenu, 13
    };
    gConsoleUIController.NavigateToScene(m_playerId, static_cast<EUIScene>(scenes[controlId]), 0, EUILayer_Scene, EUIGroup_PlayerDefault);
}

int UIScene_HelpAndOptionsMenu::getSceneType() { return EUIScene_HelpAndOptionsMenu; }
