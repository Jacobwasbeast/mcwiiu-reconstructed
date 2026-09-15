#include "game/ui/UIScene_SettingsListMenu.hpp"

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
                        int settingId, bool invert)
{
    bool value = GameSettings::GetGameSettings(playerId, static_cast<eGameSetting>(settingId)) == 1;
    list.AddNewCheckbox(std::wstring(L10N::GetString(labelId)), itemId,
                        invert ? !value : value);
}

static void addSlider(UIControl_MultiList& list, int playerId, int labelId, int itemId,
                      int minimum, int maximum, int settingId, bool repeat, bool percent)
{
    int value = static_cast<int>(GameSettings::GetGameSettings(
        playerId, static_cast<eGameSetting>(settingId)));
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
    m_primaryPlayer = g_pApp->GetPrimaryUserIndex() == m_stageWidth;
    m_host = CGameNetworkManager::sInstance.IsHost();
    m_saveRequested = false;
    m_multiList.clearList();
    int playerId = m_stageWidth;

    if (m_settingsMode == 0)
    {
        bool canChangeSkin = m_primaryPlayer && (!m_gameStarted || m_host);
        bool canChangeDifficulty = m_primaryPlayer && !m_gameStarted;
        if (m_primaryPlayer)
            addCheckbox(m_multiList, playerId, 0xffa0f6dc, 0, 0xb, false);
        addCheckbox(m_multiList, playerId, 0x48cfbb9, 9, 0x2c, false);
        addCheckbox(m_multiList, playerId, 0x387f56e3, 1, 6, false);
        addCheckbox(m_multiList, playerId, 0x212bf23f, 2, 0x28, true);
        addCheckbox(m_multiList, playerId, 0x3b5c8f8c, 3, 0xf, false);
        addCheckbox(m_multiList, playerId, 0xa38b3363, 4, 0x1a, false);
        if (g_pApp->GetGameStarted() == 0)
            m_multiList.AddNewButton(std::wstring(L10N::GetString(0x1ef2b27c)), 5, false);
        if (canChangeSkin)
        {
            std::swprintf(m_skinNames[0], 256, L"%ls", L10N::GetString(0xf8005810));
            int index;
            for (index = 1; index <= 8; ++index)
                std::swprintf(m_skinNames[index], 256, L"%ls: %d %ls",
                              L10N::GetString(0xa1070c48), index,
                              L10N::GetString(0xabfb1191));
            int skin = static_cast<int>(GameSettings::GetGameSettings(
                playerId, static_cast<eGameSetting>(0xd)));
            m_multiList.AddNewSlider(std::wstring(m_skinNames[skin]), 6, 0, 8, skin,
                                      false, 7, 3);
        }
        addSlider(m_multiList, playerId, 0xa5cbebb5, 7, 1, 200, 4, true, true);
        if (canChangeDifficulty)
        {
            int index;
            for (index = 0; index < 4; ++index)
                std::swprintf(m_difficultyNames[index], 256, L"%ls: %ls",
                              L10N::GetString(0xd324428d),
                              L10N::GetString(Difficulty::byId(index)->getKey()));
            int difficulty = static_cast<int>(GameSettings::GetGameSettings(
                playerId, static_cast<eGameSetting>(3)));
            m_multiList.AddNewSlider(std::wstring(m_difficultyNames[difficulty]), 8, 0, 3,
                                      difficulty, false, 7, 3);
        }
    }
    else if (m_settingsMode == 1)
    {
        addSlider(m_multiList, playerId, 0x67b4208f, 10, 0, 100, 0, true, true);
        addSlider(m_multiList, playerId, 0x7f9deef5, 11, 0, 100, 1, true, true);
        addCheckbox(m_multiList, playerId, 0xadd6f672, 12, 0x21, false);
        addCheckbox(m_multiList, playerId, 0xeb601b20, 13, 0x23, false);
        addCheckbox(m_multiList, playerId, 0xbce33af3, 14, 0x22, false);
        m_multiList.EnableItem(14, true, true);
    }
    else if (m_settingsMode < 3)
    {
        if (m_primaryPlayer)
        {
            addCheckbox(m_multiList, playerId, 0xac39846f, 15, 0x12, false);
            if (!m_gameStarted || m_primaryPlayer)
                addCheckbox(m_multiList, playerId, 0x68a6fe3f, 16, 0x19, false);
            addSlider(m_multiList, playerId, 0xab5a4f89, 17, 0, 100, 2, true, true);
        }
        bool showOnlineOptions = !m_gameStarted;
        if (m_gameStarted && Minecraft::InMiniGame(eMiniGameId_Glide, true))
            showOnlineOptions = minecraft->GetMiniGame()->GetHostOption(
                static_cast<eCustomGameModeHostOption>(0xe)) != 0;
        if (showOnlineOptions)
        {
            addCheckbox(m_multiList, playerId, 0x9791b32b, 18, 0x2a, false);
            addCheckbox(m_multiList, playerId, 0xce0f55b3, 19, 0x2b, false);
        }
    }
    else if (m_settingsMode == 3)
    {
        addCheckbox(m_multiList, playerId, 0xc799c804, 20, 0x17, false);
        addCheckbox(m_multiList, playerId, 0xa905ca9e, 21, 0x18, false);
        addCheckbox(m_multiList, playerId, 0xa15f6dc, 22, 0x26, false);
        if (m_primaryPlayer)
        {
            addCheckbox(m_multiList, playerId, 0x2b7ff90, 23, 0x27, false);
            addSlider(m_multiList, playerId, 0x707f7ffa, 24, 0, 100, 0x10, true, true);
        }
        addCheckbox(m_multiList, playerId, 0xfe9791a3, 25, 0x11, false);
        addCheckbox(m_multiList, playerId, 0x3a3fcf71, 26, 0x1d, false);
        addSlider(m_multiList, playerId, 0xd51ff648, 27, 0, 200, 5, true, true);
        if ((!m_gameStarted || (m_host && !Minecraft::InMiniGame(
                 eMiniGameId_Any, true))) && m_primaryPlayer)
            addCheckbox(m_multiList, playerId, 0x2c94b968, 28, 0xc, false);
        if (m_primaryPlayer)
            addCheckbox(m_multiList, playerId, 0x7eb016e, 29, 0xe, false);
        if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1f)) == 0)
            addCheckbox(m_multiList, playerId, 0x58f11ec9, 30, 0x20, false);
        if (!m_primaryPlayer || gConsoleUIController.IsIgnorePlayerJoinMenuDisplayed(playerId))
        {
            int value = static_cast<int>(GameSettings::GetGameSettings(
                playerId, static_cast<eGameSetting>(0x1c)));
            m_multiList.AddNewSlider(makeValueLabel(0x6d606219, value, false), 32, 1, 3,
                                      value + 1, false, 7, 3);
        }
        else
        {
            int value = static_cast<int>(GameSettings::GetGameSettings(
                playerId, static_cast<eGameSetting>(0x1b)));
            m_multiList.AddNewSlider(makeValueLabel(0x36cb3767, value, false), 31, 1, 3,
                                      value + 1, false, 7, 3);
            value = static_cast<int>(GameSettings::GetGameSettings(
                playerId, static_cast<eGameSetting>(0x1c)));
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
        if (m_multiList.CheckElementExists(9)) GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0x2c), m_multiList.GetCheckboxValue(9));
        if (m_multiList.CheckElementExists(1)) GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(6), m_multiList.GetCheckboxValue(1));
        if (m_multiList.CheckElementExists(2)) GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0x28), !m_multiList.GetCheckboxValue(2));
        if (m_multiList.CheckElementExists(3)) GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0xf), m_multiList.GetCheckboxValue(3));
        if (m_multiList.CheckElementExists(4)) GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0x1a), m_multiList.GetCheckboxValue(4));
        if (m_multiList.CheckElementExists(0)) GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0xb), m_multiList.GetCheckboxValue(0));
    }
    else if (m_settingsMode == 1)
    {
        if (m_multiList.CheckElementExists(12)) GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0x21), m_multiList.GetCheckboxValue(12));
        if (m_multiList.CheckElementExists(13)) GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0x23), m_multiList.GetCheckboxValue(13));
        if (m_multiList.CheckElementExists(14)) GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0x22), m_multiList.GetCheckboxValue(14));
    }
    else if (m_settingsMode < 3)
    {
        if (m_multiList.CheckElementExists(15)) GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0x12), m_multiList.GetCheckboxValue(15));
        if (m_multiList.CheckElementExists(16)) GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0x19), m_multiList.GetCheckboxValue(16));
        if (m_multiList.CheckElementExists(18)) GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0x2a), m_multiList.GetCheckboxValue(18));
        if (m_multiList.CheckElementExists(19)) GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0x2b), m_multiList.GetCheckboxValue(19));
    }
    else
    {
        const int ids[] = {20, 21, 22, 23, 29, 28, 26, 30, 25};
        const int settings[] = {0x17, 0x18, 0x26, 0x27, 0xe, 0xc, 0x1d, 0x20, 0x11};
        unsigned int i;
        for (i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i)
            if (m_multiList.CheckElementExists(ids[i]))
                GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(settings[i]), m_multiList.GetCheckboxValue(ids[i]));
    }
}

void UIScene_SettingsListMenu::tick()
{
    UIScene::tick();
    if (m_saveRequested)
    {
        m_saveRequested = false;
        setGameSettings();
        gConsoleUIController.NavigateToScene(m_stageWidth, static_cast<EUIScene>(0x4f), 0,
                                              static_cast<EUILayer>(6),
                                              static_cast<EUIGroup>(7));
    }
}

void UIScene_SettingsListMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_stageWidth, 0x8d6323ed, 0x4f6bcf5e, -1, -1, -1,
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
        m_layer->showComponent(m_stageWidth, static_cast<EUIScene>(0x42), false);
        m_layer->showComponent(m_stageWidth, static_cast<EUIScene>(0x43), g_pApp->GetLocalPlayerCount() == 1);
    }
    else
    {
        m_layer->showComponent(m_stageWidth, static_cast<EUIScene>(0x42), true);
        m_layer->showComponent(m_stageWidth, static_cast<EUIScene>(0x43), true);
    }
}

std::wstring UIScene_SettingsListMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"MultilistMenu")
                                             : std::wstring(L"MultilistMenuSplit");
}

void UIScene_SettingsListMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                           bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
    if (action < 4 || (action > 9 && action < 14) || (action > 15 && action != 20 && action != 21))
        return;
    if (action == 21)
    {
        if (!released)
            return;
        setGameSettings();
        navigateBack();
        handled = true;
        return;
    }
    sendInputToMovie(action, repeat, released, pressed);
}

void UIScene_SettingsListMenu::handlePress(int, int second)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (second == 5)
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
        GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0xd), value);
        m_multiList.SetItemLabel(6, UIString(m_skinNames[value]), false);
        g_pApp->SetAutosaveTimerTime();
    }
    else if (id == 7)
    {
        GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(4), value);
        m_multiList.SetItemLabel(7, UIString(makeValueLabel(0xa5cbebb5, value, true)), false);
    }
    else if (id == 8)
    {
        GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(3), value);
        m_multiList.SetItemLabel(8, UIString(m_difficultyNames[value]), false);
    }
    else if (id == 10)
    {
        GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0), value);
        m_multiList.SetItemLabel(10, UIString(makeValueLabel(0x67b4208f, value, true)), false);
    }
    else if (id == 11)
    {
        GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(1), value);
        m_multiList.SetItemLabel(11, UIString(makeValueLabel(0x7f9deef5, value, true)), false);
    }
    else if (id == 17)
    {
        GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(2), value);
        m_multiList.SetItemLabel(17, UIString(makeValueLabel(0xab5a4f89, value, true)), false);
    }
    else if (id == 24)
    {
        GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0x10), value);
        m_multiList.SetItemLabel(24, UIString(makeValueLabel(0x707f7ffa, value, true)), false);
    }
    else if (id == 27)
    {
        GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(5), value);
        m_multiList.SetItemLabel(27, UIString(makeValueLabel(0xd51ff648, value, true)), false);
    }
    else if (id == 31)
    {
        m_multiList.SetItemLabel(31, UIString(makeValueLabel(0x36cb3767, value, false)), false);
        int oldValue = static_cast<int>(GameSettings::GetGameSettings(
            m_stageWidth, static_cast<eGameSetting>(0x1b)));
        if (value != oldValue + 1)
        {
            GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0x1b), value - 1);
            gConsoleUIController.UpdateSelectedItemPos(m_stageWidth);
        }
    }
    else if (id == 32)
    {
        m_multiList.SetItemLabel(32, UIString(makeValueLabel(0x6d606219, value, false)), false);
        int oldValue = static_cast<int>(GameSettings::GetGameSettings(
            m_stageWidth, static_cast<eGameSetting>(0x1c)));
        if (value != oldValue + 1)
        {
            GameSettings::SetGameSettings(m_stageWidth, static_cast<eGameSetting>(0x1c), value - 1);
            gConsoleUIController.UpdateSelectedItemPos(m_stageWidth);
        }
    }
}

int UIScene_SettingsListMenu::getSceneType()
{
    return 0x13;
}
