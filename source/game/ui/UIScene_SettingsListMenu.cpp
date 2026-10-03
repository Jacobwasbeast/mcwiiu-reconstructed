#include "game/ui/UIScene_SettingsListMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/FJ_MultilistMenu.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CProfile.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

UIScene_SettingsListMenu::UIScene_SettingsListMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer)
{
    initialiseMovie();
    int* settingsData = static_cast<int*>(data);
    m_settingsMode = static_cast<unsigned int>(settingsData[1]);
    delete[] settingsData;
    Init();
}

UIScene_SettingsListMenu::~UIScene_SettingsListMenu()
{
}

unsigned int UIScene_SettingsListMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_multiList.setupControl(this, root, std::string("MultiList"));
    m_controls.push_back(&m_multiList);
    m_description.setupControl(this, root, std::string("Description"));
    m_controls.push_back(&m_description);
    return 1;
}

void UIScene_SettingsListMenu::doVerticalResizeCheck()
{
    FJ_MultilistMenu* menu = static_cast<FJ_MultilistMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->DoVerticalResizeCheck();
}

void UIScene_SettingsListMenu::HideDescription()
{
    FJ_MultilistMenu* menu = static_cast<FJ_MultilistMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->HideDescription();
}

void UIScene_SettingsListMenu::Init()
{
    wchar_t text[256];
    Minecraft* minecraft = Minecraft::GetInstance();
    m_gameStarted = minecraft->mLevel != 0;
    m_primaryPlayer = gProfile->GetPrimaryPad() == m_playerId;
    m_host = CGameNetworkManager::sInstance.IsHost();
    m_saveRequested = false;

    if (m_settingsMode == 0)
    {
        bool canChangeSkin = m_primaryPlayer;
        bool canChangeDifficulty = m_primaryPlayer;
        if (m_gameStarted)
        {
            canChangeDifficulty = false;
            if (!CGameNetworkManager::sInstance.IsHost())
                canChangeSkin = false;
        }
        bool gameStarted = g_pApp->GetGameStarted();
        if (m_primaryPlayer)
            m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0xffa0f6dc)), 0,
                GameSettings::GetGameSettings(m_playerId, eGameSetting_SplitScreenVertical) == 1);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_AUTOJUMP)), 9,
            GameSettings::GetGameSettings(m_playerId, eGameSetting_AutoJump) == 1);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_VIEW_BOBBING)), 1,
            GameSettings::GetGameSettings(m_playerId, eGameSetting_ViewBob) == 1);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_FLYING_VIEW_ROLLING)), 2,
            GameSettings::GetGameSettings(m_playerId, eGameSetting_FlyingViewRolling) != 1);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_HINTS)), 3,
            GameSettings::GetGameSettings(m_playerId, eGameSetting_Hints) == 1);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_CHECKBOX_DEATH_MESSAGES)), 4,
            GameSettings::GetGameSettings(m_playerId, eGameSetting_DeathMessages) == 1);
        if (!gameStarted)
            m_multiList.AddNewButton(std::wstring(L10N::GetString(0x1ef2b27c)), 5, false);
        if (canChangeSkin)
        {
            int skin = static_cast<int>(GameSettings::GetGameSettings(
                m_playerId, eGameSetting_Autosave));
            std::swprintf(m_skinNames[0], 256, L"%ls", L10N::GetString(IDS_SLIDER_AUTOSAVE_OFF));
            for (int index = 1; index <= 8; ++index)
                std::swprintf(m_skinNames[index], 256, L"%ls: %d %ls",
                              L10N::GetString(IDS_SLIDER_AUTOSAVE), index,
                              L10N::GetString(0xabfb1191));
            m_multiList.AddNewSlider(std::wstring(m_skinNames[skin]), 6, 0, 8, skin,
                                      false, 7, 3);
        }
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(0xa5cbebb5),
                      GameSettings::GetGameSettings(m_playerId, eGameSetting_GameSensitivity));
        m_multiList.AddNewSlider(std::wstring(text), 7, 1, 200,
                                  GameSettings::GetGameSettings(m_playerId, eGameSetting_GameSensitivity),
                                  true, 7, 3);
        if (canChangeDifficulty)
        {
            int difficulty = static_cast<int>(GameSettings::GetGameSettings(
                m_playerId, eGameSetting_Difficulty));
            for (int index = 0; index < 4; ++index)
                std::swprintf(m_difficultyNames[index], 256, L"%ls: %ls",
                              L10N::GetString(IDS_SLIDER_DIFFICULTY),
                              L10N::GetString(Difficulty::byId(index)->getKey()));
            m_multiList.AddNewSlider(std::wstring(m_difficultyNames[difficulty]), 8, 0, 3,
                                      difficulty, false, 7, 3);
        }
        doVerticalResizeCheck();
        doHorizontalResizeCheck();
        HideDescription();
        g_pApp->GetLocalPlayerCount();
        gConsoleUIController.TouchBoxRebuild(this);
        return;
    }
    if (m_settingsMode == 1)
    {
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_MUSIC),
                      GameSettings::GetGameSettings(m_playerId, eGameSetting_MusicVolume));
        m_multiList.AddNewSlider(std::wstring(text), 10, 0, 100,
                                  GameSettings::GetGameSettings(m_playerId, eGameSetting_MusicVolume),
                                  true, 7, 3);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_SOUND),
                      GameSettings::GetGameSettings(m_playerId, eGameSetting_SoundVolume));
        m_multiList.AddNewSlider(std::wstring(text), 11, 0, 100,
                                  GameSettings::GetGameSettings(m_playerId, eGameSetting_SoundVolume),
                                  true, 7, 3);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_CAVE_SOUNDS)), 12,
            GameSettings::GetGameSettings(m_playerId, eGameSetting_CaveSounds) == 1);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MINECART_SOUNDS)), 13,
            GameSettings::GetGameSettings(m_playerId, eGameSetting_MinecartSounds) == 1);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0xbce33af3)), 14,
            GameSettings::GetGameSettings(m_playerId, eGameSetting_GameChat) == 1);
        m_multiList.EnableItem(14, true, true);
        doVerticalResizeCheck();
        doHorizontalResizeCheck();
        HideDescription();
        g_pApp->GetLocalPlayerCount();
        gConsoleUIController.TouchBoxRebuild(this);
        return;
    }
    if (m_settingsMode < 3)
    {
        if (gProfile->GetPrimaryPad() == m_playerId)
        {
            m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_CHECKBOX_RENDER_CLOUDS)), 15,
                GameSettings::GetGameSettings(m_playerId, eGameSetting_Clouds) == 1);
            if (!m_gameStarted || m_primaryPlayer)
                m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0x68a6fe3f)), 16,
                    GameSettings::GetGameSettings(m_playerId, eGameSetting_CustomSkinAnim) == 1);
            std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_GAMMA),
                          GameSettings::GetGameSettings(m_playerId, eGameSetting_Gamma));
            m_multiList.AddNewSlider(std::wstring(text), 17, 0, 100,
                                      GameSettings::GetGameSettings(m_playerId, eGameSetting_Gamma),
                                      true, 7, 3);
        }
        bool showGhostOptions = !m_gameStarted;
        if (m_gameStarted && Minecraft::InMiniGame(eMiniGameId_Glide, true))
            showGhostOptions = Minecraft::GetInstance()->GetMiniGame()->GetHostOption(
                eCustomGameModeHostOption_TimeTrial) != 0;
        if (showGhostOptions)
        {
            m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0x9791b32b)), 18,
                GameSettings::GetGameSettings(m_playerId, eGameSetting_ShowGlideGhost) == 1);
            m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MG03_SHOW_GHOST_PATH)), 19,
                GameSettings::GetGameSettings(m_playerId, eGameSetting_ShowGlideGhostPath) == 1);
        }
        doVerticalResizeCheck();
        doHorizontalResizeCheck();
        HideDescription();
        g_pApp->GetLocalPlayerCount();
        gConsoleUIController.TouchBoxRebuild(this);
        return;
    }
    if (m_settingsMode != 3)
    {
        doVerticalResizeCheck();
        doHorizontalResizeCheck();
        HideDescription();
        g_pApp->GetLocalPlayerCount();
        gConsoleUIController.TouchBoxRebuild(this);
        return;
    }
    m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_CHECKBOX_DISPLAY_HUD)), 20,
        GameSettings::GetGameSettings(m_playerId, eGameSetting_DisplayHUD) == 1);
    m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_CHECKBOX_DISPLAY_HAND)), 21,
        GameSettings::GetGameSettings(m_playerId, eGameSetting_DisplayHand) == 1);
    m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0xa15f6dc)), 22,
        GameSettings::GetGameSettings(m_playerId, eGameSetting_DisplayGameMessages) == 1);
    if (m_primaryPlayer)
    {
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_CHECKBOX_DISPLAY_SAVE_ICON)), 23,
            GameSettings::GetGameSettings(m_playerId, eGameSetting_DisplaySaveIcon) == 1);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_INTERFACEOPACITY),
                      GameSettings::GetGameSettings(m_playerId, eGameSetting_InterfaceOpacity));
        m_multiList.AddNewSlider(std::wstring(text), 24, 0, 100,
                                  GameSettings::GetGameSettings(m_playerId, eGameSetting_InterfaceOpacity),
                                  true, 7, 3);
    }
    m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_IN_GAME_TOOLTIPS)), 25,
        GameSettings::GetGameSettings(m_playerId, eGameSetting_Tooltips) == 1);
    m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_CHECKBOX_ANIMATED_CHARACTER)), 26,
        GameSettings::GetGameSettings(m_playerId, eGameSetting_AnimatedCharacter) == 1);
    std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(0xd51ff648),
                  GameSettings::GetGameSettings(m_playerId, eGameSetting_InterfaceSensitivity));
    m_multiList.AddNewSlider(std::wstring(text), 27, 0, 200,
                              GameSettings::GetGameSettings(m_playerId, eGameSetting_InterfaceSensitivity),
                              true, 7, 3);
    bool primaryPlayer = m_primaryPlayer;
    if ((!m_gameStarted || (CGameNetworkManager::sInstance.IsHost()
                            && !Minecraft::InMiniGame(eMiniGameId_Any, true))) && primaryPlayer)
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0x2c94b968)), 28,
            GameSettings::GetGameSettings(m_playerId, eGameSetting_DisplayMii) == 1);
    if (m_primaryPlayer)
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0x7eb016e)), 29,
            GameSettings::GetGameSettings(m_playerId, eGameSetting_SplitscreenNicknames) == 1);
    if (g_pApp->GetGameHostOption(eGameHostOption_MiniGame) == 0)
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_CHECKBOX_CLASSIC_CRAFTING)), 30,
            GameSettings::GetGameSettings(m_playerId, eGameSetting_ClassicCrafting) == 1);
    if (!m_primaryPlayer || gProfile->IsGuest(m_playerId))
    {
        std::swprintf(text, 256, L"%ls: %d", L10N::GetString(0x6d606219),
                      GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSizeSplitscreen));
        m_multiList.AddNewSlider(std::wstring(text), 32, 1, 3,
                                  GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSizeSplitscreen) + 1,
                                  false, 7, 3);
    }
    else
    {
        std::swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_SLIDER_UISIZE),
                      GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSize));
        m_multiList.AddNewSlider(std::wstring(text), 31, 1, 3,
                                  GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSize) + 1,
                                  false, 7, 3);
        std::swprintf(text, 256, L"%ls: %d", L10N::GetString(0x6d606219),
                      GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSizeSplitscreen));
        m_multiList.AddNewSlider(std::wstring(text), 32, 1, 3,
                                  GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSizeSplitscreen) + 1,
                                  false, 7, 3);
    }
    doVerticalResizeCheck();
    doHorizontalResizeCheck();
    HideDescription();
    g_pApp->GetLocalPlayerCount();
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_SettingsListMenu::setGameSettings()
{
    if (m_settingsMode == 0)
    {
        if (m_multiList.CheckElementExists(9)) GameSettings::SetGameSettings(m_playerId, eGameSetting_AutoJump, m_multiList.GetCheckboxValue(9));
        if (m_multiList.CheckElementExists(1)) GameSettings::SetGameSettings(m_playerId, eGameSetting_ViewBob, m_multiList.GetCheckboxValue(1));
        if (m_multiList.CheckElementExists(2)) GameSettings::SetGameSettings(m_playerId, eGameSetting_FlyingViewRolling, !m_multiList.GetCheckboxValue(2));
        if (m_multiList.CheckElementExists(3)) GameSettings::SetGameSettings(m_playerId, eGameSetting_Hints, m_multiList.GetCheckboxValue(3));
        if (m_multiList.CheckElementExists(4)) GameSettings::SetGameSettings(m_playerId, eGameSetting_DeathMessages, m_multiList.GetCheckboxValue(4));
        if (m_multiList.CheckElementExists(0)) GameSettings::SetGameSettings(m_playerId, eGameSetting_SplitScreenVertical, m_multiList.GetCheckboxValue(0));
    }
    else if (m_settingsMode == 1)
    {
        if (m_multiList.CheckElementExists(12)) GameSettings::SetGameSettings(m_playerId, eGameSetting_CaveSounds, m_multiList.GetCheckboxValue(12));
        if (m_multiList.CheckElementExists(13)) GameSettings::SetGameSettings(m_playerId, eGameSetting_MinecartSounds, m_multiList.GetCheckboxValue(13));
        if (m_multiList.CheckElementExists(14)) GameSettings::SetGameSettings(m_playerId, eGameSetting_GameChat, m_multiList.GetCheckboxValue(14));
    }
    else if (m_settingsMode < 3)
    {
        if (m_multiList.CheckElementExists(15)) GameSettings::SetGameSettings(m_playerId, eGameSetting_Clouds, m_multiList.GetCheckboxValue(15));
        if (m_multiList.CheckElementExists(16)) GameSettings::SetGameSettings(m_playerId, eGameSetting_CustomSkinAnim, m_multiList.GetCheckboxValue(16));
        if (m_multiList.CheckElementExists(18)) GameSettings::SetGameSettings(m_playerId, eGameSetting_ShowGlideGhost, m_multiList.GetCheckboxValue(18));
        if (m_multiList.CheckElementExists(19)) GameSettings::SetGameSettings(m_playerId, eGameSetting_ShowGlideGhostPath, m_multiList.GetCheckboxValue(19));
    }
    else
    {
        const int ids[] = {20, 21, 22, 23, 29, 28, 26, 30, 25};
        const eGameSetting settings[] = {eGameSetting_DisplayHUD, eGameSetting_DisplayHand, eGameSetting_DisplayGameMessages, eGameSetting_DisplaySaveIcon, eGameSetting_SplitscreenNicknames, eGameSetting_DisplayMii, eGameSetting_AnimatedCharacter, eGameSetting_ClassicCrafting, eGameSetting_Tooltips};
        unsigned int i;
        for (i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i)
            if (m_multiList.CheckElementExists(ids[i]))
                GameSettings::SetGameSettings(m_playerId, settings[i], m_multiList.GetCheckboxValue(ids[i]));
    }
}

void UIScene_SettingsListMenu::tick()
{
    UIScene::tick();
    if (m_saveRequested)
    {
        m_saveRequested = false;
        setGameSettings();
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_LanguageSelector, 0,
                                              EUILayer_Scene,
                                              EUIGroup_PlayerDefault);
    }
}

void UIScene_SettingsListMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     false, false);
}

void UIScene_SettingsListMenu::handlePreUnloadForReload()
{
    setGameSettings();
}

void UIScene_SettingsListMenu::handleReload()
{
    Init();
}

void UIScene_SettingsListMenu::updateComponents()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLevel != 0)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        m_layer->showComponent(m_playerId, EUIScene_Logo, g_pApp->GetLocalPlayerCount() == 1);
    }
    else
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        m_layer->showComponent(m_playerId, EUIScene_Logo, true);
    }
}

std::wstring UIScene_SettingsListMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"MultilistMenu")
                                             : std::wstring(L"MultilistMenuSplit");
}

void UIScene_SettingsListMenu::handleInput(int playerId, int action, bool repeat, bool pressed,
                                           bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            setGameSettings();
            navigateBack();
            handled = true;
        }
        break;
    }
}

void UIScene_SettingsListMenu::handlePress(int, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (childId == 5)
        m_saveRequested = true;
}

void UIScene_SettingsListMenu::handleCheckboxElementToggled(int, int, bool)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void UIScene_SettingsListMenu::handleSliderElementMove(int, int id, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    wchar_t text[256];
    if (id == 6)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_Autosave, value);
        m_multiList.SetItemLabel(6, UIString(m_skinNames[value]), false);
        g_pApp->SetAutosaveTimerTime();
    }
    else if (id == 7)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_GameSensitivity, value);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(0xa5cbebb5),
                      GameSettings::GetGameSettings(m_playerId, eGameSetting_GameSensitivity));
        m_multiList.SetItemLabel(7, UIString(text), false);
    }
    else if (id == 8)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_Difficulty, value);
        m_multiList.SetItemLabel(8, UIString(m_difficultyNames[value]), false);
    }
    else if (id == 10)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_MusicVolume, value);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_MUSIC), value);
        m_multiList.SetItemLabel(10, UIString(text), false);
    }
    else if (id == 11)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_SoundVolume, value);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_SOUND), value);
        m_multiList.SetItemLabel(11, UIString(text), false);
    }
    else if (id == 17)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_Gamma, value);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_GAMMA), value);
        m_multiList.SetItemLabel(17, UIString(text), false);
    }
    else if (id == 24)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_InterfaceOpacity, value);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_INTERFACEOPACITY), value);
        m_multiList.SetItemLabel(24, UIString(text), false);
    }
    else if (id == 27)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_InterfaceSensitivity, value);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(0xd51ff648), value);
        m_multiList.SetItemLabel(27, UIString(text), false);
    }
    else if (id == 31)
    {
        std::swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_SLIDER_UISIZE), value);
        m_multiList.SetItemLabel(31, UIString(text), false);
        int oldValue = static_cast<int>(GameSettings::GetGameSettings(
            m_playerId, eGameSetting_HUDSize));
        if (value != oldValue + 1)
        {
            GameSettings::SetGameSettings(m_playerId, eGameSetting_HUDSize, value - 1);
            gConsoleUIController.UpdateSelectedItemPos(m_playerId);
        }
    }
    else if (id == 32)
    {
        std::swprintf(text, 256, L"%ls: %d", L10N::GetString(0x6d606219), value);
        m_multiList.SetItemLabel(32, UIString(text), false);
        int oldValue = static_cast<int>(GameSettings::GetGameSettings(
            m_playerId, eGameSetting_HUDSizeSplitscreen));
        if (value != oldValue + 1)
        {
            GameSettings::SetGameSettings(m_playerId, eGameSetting_HUDSizeSplitscreen, value - 1);
            gConsoleUIController.UpdateSelectedItemPos(m_playerId);
        }
    }
}

int UIScene_SettingsListMenu::getSceneType()
{
    return EUIScene_SettingsListMenu;
}
