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

static std::wstring makeValueLabel(int labelId, int value, bool percent)
{
    wchar_t text[256];
    if (percent)
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(labelId), value);
    else
        std::swprintf(text, 256, L"%ls: %d", L10N::GetString(labelId), value);
    return std::wstring(text);
}

static void addCheckbox(UIControl_MultiList& list, int playerId, int labelId, int itemId,
                        eGameSetting setting, bool invert)
{
    bool value = GameSettings::GetGameSettings(playerId, setting) == 1;
    list.AddNewCheckbox(std::wstring(L10N::GetString(labelId)), itemId,
                        invert ? !value : value);
}

static void addSlider(UIControl_MultiList& list, int playerId, int labelId, int itemId,
                      int minimum, int maximum, eGameSetting setting, bool repeat, bool percent)
{
    int value = static_cast<int>(GameSettings::GetGameSettings(playerId, setting));
    list.AddNewSlider(makeValueLabel(labelId, value, percent), itemId, minimum, maximum, value,
                      repeat, 7, 3);
}

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
    Minecraft* minecraft = Minecraft::GetInstance();
    m_gameStarted = minecraft->mLevel != 0;
    m_primaryPlayer = gProfile->GetPrimaryPad() == m_playerId;
    m_host = CGameNetworkManager::sInstance.IsHost();
    m_saveRequested = false;
    m_multiList.clearList();
    int playerId = m_playerId;

    if (m_settingsMode == 0)
    {
        bool canChangeSkin = m_primaryPlayer && (!m_gameStarted || m_host);
        bool canChangeDifficulty = m_primaryPlayer && !m_gameStarted;
        if (m_primaryPlayer)
            addCheckbox(m_multiList, playerId, 0xffa0f6dc, 0, eGameSetting_SplitScreenVertical, false);
        addCheckbox(m_multiList, playerId, IDS_AUTOJUMP, 9, eGameSetting_AutoJump, false);
        addCheckbox(m_multiList, playerId, IDS_VIEW_BOBBING, 1, eGameSetting_ViewBob, false);
        addCheckbox(m_multiList, playerId, IDS_FLYING_VIEW_ROLLING, 2, eGameSetting_FlyingViewRolling, true);
        addCheckbox(m_multiList, playerId, IDS_HINTS, 3, eGameSetting_Hints, false);
        addCheckbox(m_multiList, playerId, IDS_CHECKBOX_DEATH_MESSAGES, 4, eGameSetting_DeathMessages, false);
        if (g_pApp->GetGameStarted() == 0)
            m_multiList.AddNewButton(std::wstring(L10N::GetString(0x1ef2b27c)), 5, false);
        if (canChangeSkin)
        {
            std::swprintf(m_skinNames[0], 256, L"%ls", L10N::GetString(IDS_SLIDER_AUTOSAVE_OFF));
            int index;
            for (index = 1; index <= 8; ++index)
                std::swprintf(m_skinNames[index], 256, L"%ls: %d %ls",
                              L10N::GetString(IDS_SLIDER_AUTOSAVE), index,
                              L10N::GetString(0xabfb1191));
            int skin = static_cast<int>(GameSettings::GetGameSettings(
                playerId, eGameSetting_Autosave));
            m_multiList.AddNewSlider(std::wstring(m_skinNames[skin]), 6, 0, 8, skin,
                                      false, 7, 3);
        }
        addSlider(m_multiList, playerId, 0xa5cbebb5, 7, 1, 200, eGameSetting_GameSensitivity, true, true);
        if (canChangeDifficulty)
        {
            int index;
            for (index = 0; index < 4; ++index)
                std::swprintf(m_difficultyNames[index], 256, L"%ls: %ls",
                              L10N::GetString(IDS_SLIDER_DIFFICULTY),
                              L10N::GetString(Difficulty::byId(index)->getKey()));
            int difficulty = static_cast<int>(GameSettings::GetGameSettings(
                playerId, eGameSetting_Difficulty));
            m_multiList.AddNewSlider(std::wstring(m_difficultyNames[difficulty]), 8, 0, 3,
                                      difficulty, false, 7, 3);
        }
    }
    else if (m_settingsMode == 1)
    {
        addSlider(m_multiList, playerId, IDS_SLIDER_MUSIC, 10, 0, 100, eGameSetting_MusicVolume, true, true);
        addSlider(m_multiList, playerId, IDS_SLIDER_SOUND, 11, 0, 100, eGameSetting_SoundVolume, true, true);
        addCheckbox(m_multiList, playerId, IDS_CAVE_SOUNDS, 12, eGameSetting_CaveSounds, false);
        addCheckbox(m_multiList, playerId, IDS_MINECART_SOUNDS, 13, eGameSetting_MinecartSounds, false);
        addCheckbox(m_multiList, playerId, 0xbce33af3, 14, eGameSetting_GameChat, false);
        m_multiList.EnableItem(14, true, true);
    }
    else if (m_settingsMode < 3)
    {
        if (m_primaryPlayer)
        {
            addCheckbox(m_multiList, playerId, IDS_CHECKBOX_RENDER_CLOUDS, 15, eGameSetting_Clouds, false);
            if (!m_gameStarted || m_primaryPlayer)
                addCheckbox(m_multiList, playerId, 0x68a6fe3f, 16, eGameSetting_CustomSkinAnim, false);
            addSlider(m_multiList, playerId, IDS_SLIDER_GAMMA, 17, 0, 100, eGameSetting_Gamma, true, true);
        }
        bool showOnlineOptions = !m_gameStarted;
        if (m_gameStarted && Minecraft::InMiniGame(eMiniGameId_Glide, true))
            showOnlineOptions = minecraft->GetMiniGame()->GetHostOption(
                eCustomGameModeHostOption_TimeTrial) != 0;
        if (showOnlineOptions)
        {
            addCheckbox(m_multiList, playerId, 0x9791b32b, 18, eGameSetting_ShowGlideGhost, false);
            addCheckbox(m_multiList, playerId, IDS_MG03_SHOW_GHOST_PATH, 19, eGameSetting_ShowGlideGhostPath, false);
        }
    }
    else if (m_settingsMode == 3)
    {
        addCheckbox(m_multiList, playerId, IDS_CHECKBOX_DISPLAY_HUD, 20, eGameSetting_DisplayHUD, false);
        addCheckbox(m_multiList, playerId, IDS_CHECKBOX_DISPLAY_HAND, 21, eGameSetting_DisplayHand, false);
        addCheckbox(m_multiList, playerId, 0xa15f6dc, 22, eGameSetting_DisplayGameMessages, false);
        if (m_primaryPlayer)
        {
            addCheckbox(m_multiList, playerId, IDS_CHECKBOX_DISPLAY_SAVE_ICON, 23, eGameSetting_DisplaySaveIcon, false);
            addSlider(m_multiList, playerId, IDS_SLIDER_INTERFACEOPACITY, 24, 0, 100, eGameSetting_InterfaceOpacity, true, true);
        }
        addCheckbox(m_multiList, playerId, IDS_IN_GAME_TOOLTIPS, 25, eGameSetting_Tooltips, false);
        addCheckbox(m_multiList, playerId, IDS_CHECKBOX_ANIMATED_CHARACTER, 26, eGameSetting_AnimatedCharacter, false);
        addSlider(m_multiList, playerId, 0xd51ff648, 27, 0, 200, eGameSetting_InterfaceSensitivity, true, true);
        if ((!m_gameStarted || (m_host && !Minecraft::InMiniGame(
                 eMiniGameId_Any, true))) && m_primaryPlayer)
            addCheckbox(m_multiList, playerId, 0x2c94b968, 28, eGameSetting_DisplayMii, false);
        if (m_primaryPlayer)
            addCheckbox(m_multiList, playerId, 0x7eb016e, 29, eGameSetting_SplitscreenNicknames, false);
        if (g_pApp->GetGameHostOption(eGameHostOption_MiniGame) == 0)
            addCheckbox(m_multiList, playerId, IDS_CHECKBOX_CLASSIC_CRAFTING, 30, eGameSetting_ClassicCrafting, false);
        if (!m_primaryPlayer || gConsoleUIController.IsIgnorePlayerJoinMenuDisplayed(playerId))
        {
            int value = static_cast<int>(GameSettings::GetGameSettings(
                playerId, eGameSetting_HUDSizeSplitscreen));
            m_multiList.AddNewSlider(makeValueLabel(0x6d606219, value, false), 32, 1, 3,
                                      value + 1, false, 7, 3);
        }
        else
        {
            int value = static_cast<int>(GameSettings::GetGameSettings(
                playerId, eGameSetting_HUDSize));
            m_multiList.AddNewSlider(makeValueLabel(IDS_SLIDER_UISIZE, value, false), 31, 1, 3,
                                      value + 1, false, 7, 3);
            value = static_cast<int>(GameSettings::GetGameSettings(
                playerId, eGameSetting_HUDSizeSplitscreen));
            m_multiList.AddNewSlider(makeValueLabel(0x6d606219, value, false), 32, 1, 3,
                                      value + 1, false, 7, 3);
        }
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
    if (id == 6)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_Autosave, value);
        m_multiList.SetItemLabel(6, UIString(m_skinNames[value]), false);
        g_pApp->SetAutosaveTimerTime();
    }
    else if (id == 7)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_GameSensitivity, value);
        m_multiList.SetItemLabel(7, UIString(makeValueLabel(0xa5cbebb5, value, true)), false);
    }
    else if (id == 8)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_Difficulty, value);
        m_multiList.SetItemLabel(8, UIString(m_difficultyNames[value]), false);
    }
    else if (id == 10)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_MusicVolume, value);
        m_multiList.SetItemLabel(10, UIString(makeValueLabel(IDS_SLIDER_MUSIC, value, true)), false);
    }
    else if (id == 11)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_SoundVolume, value);
        m_multiList.SetItemLabel(11, UIString(makeValueLabel(IDS_SLIDER_SOUND, value, true)), false);
    }
    else if (id == 17)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_Gamma, value);
        m_multiList.SetItemLabel(17, UIString(makeValueLabel(IDS_SLIDER_GAMMA, value, true)), false);
    }
    else if (id == 24)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_InterfaceOpacity, value);
        m_multiList.SetItemLabel(24, UIString(makeValueLabel(IDS_SLIDER_INTERFACEOPACITY, value, true)), false);
    }
    else if (id == 27)
    {
        GameSettings::SetGameSettings(m_playerId, eGameSetting_InterfaceSensitivity, value);
        m_multiList.SetItemLabel(27, UIString(makeValueLabel(0xd51ff648, value, true)), false);
    }
    else if (id == 31)
    {
        m_multiList.SetItemLabel(31, UIString(makeValueLabel(IDS_SLIDER_UISIZE, value, false)), false);
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
        m_multiList.SetItemLabel(32, UIString(makeValueLabel(0x6d606219, value, false)), false);
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
