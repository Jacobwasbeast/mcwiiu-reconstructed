#include "game/ui/UIScene_LoadMenu.hpp"

#include <cwchar>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/FJ_LoadMenu.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIControl_CheckBox.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

static LaunchMoreOptionsMenuInitData* getLoadMenuInitData(UIScene_LoadMenu* scene)
{
    return reinterpret_cast<LaunchMoreOptionsMenuInitData*>(
        reinterpret_cast<unsigned char*>(scene) + 0x494);
}

static UIControl_CheckBox* getLoadMenuOnlineCheckbox(UIScene_LoadMenu* scene)
{
    return reinterpret_cast<UIControl_CheckBox*>(
        reinterpret_cast<unsigned char*>(scene) + 0x434);
}

UIScene_LoadMenu::UIScene_LoadMenu(int sceneType, void* data, UILayer* layer)
    : IUIScene_StartGame(sceneType, data, layer, true), m_seedLogic(), m_sizeLogic(),
      m_gameModeLogic(), m_difficultyLogic(), m_mainPanel(), m_gameName(), m_createdMode(),
      m_description(), m_texturePackButton(), m_playButton(), m_levelIcon(), m_list(),
      m_thumbnailPending(false), m_thumbnailLoaded(false), m_thumbnailFailed(false),
      m_customGame(false), m_battleGame(false), m_publicGamesAllowed(true),
      m_allowPublicGames(false), m_hasVariants(false), m_resetNether(true), m_updateEnd(true),
      m_onlineGame(false), m_customOptions(false), m_hasGameData(false), m_loadRequested(false),
      m_focus(-1.0), m_childFocus(-1.0), m_resetNetherPending(false), m_updateEndPending(false),
      m_loaded(false), m_gameMode(0), m_seedValue(0), m_sizeValue(0), m_selectedItem(0),
      m_thumbnailName(), m_thumbnailData(NULL), m_thumbnailSize(0), m_miniGames()
{
    initialiseMovie();
    m_gameName.init(UIString(L""));
    m_createdMode.init(UIString(L""));
    m_description.init(UIString(L""));
    SetEnablePlayGameButton(true);
    _init();
}

UIScene_LoadMenu::~UIScene_LoadMenu()
{
    delete[] m_thumbnailData;
}

void UIScene_LoadMenu::AddVariantButtons(bool alternate)
{
    unsigned int label = m_miniGames.GetVariantPlayLabel(alternate ? 1 : 0, alternate);
    m_list.AddNewButton(std::wstring(L10N::GetString(static_cast<int>(label))),
                        alternate ? 0x12 : 0x11, true);
}

void UIScene_LoadMenu::UpdateIfPublicGamesAllowed()
{
    m_publicGamesAllowed = true;
    if (!m_customGame && !m_battleGame)
        return;
    MiniGameDef* game = MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(m_gameMode), true);
    if (game == NULL || !game->IsCRCValid())
    {
        m_publicGamesAllowed = false;
        m_allowPublicGames = false;
        return;
    }
    m_allowPublicGames = m_publicGamesAllowed;
}

void UIScene_LoadMenu::UpdateDescriptionText()
{
    if (!m_customGame && !m_battleGame)
        return;
    unsigned int id = m_miniGames.GetVariantDesc(m_selectedItem, m_sizeValue != 0);
    if (id == static_cast<unsigned int>(-1) || id == static_cast<unsigned int>(-2))
        return;
    if (g_pApp != NULL)
        m_description.setLabel(UIString(g_pApp->FormatHTMLString(m_playerId,
                                            std::wstring(L""), -1)), false, true);
}

void UIScene_LoadMenu::_initCustom()
{
    if (m_movie == NULL)
        return;
    m_customOptions = true;
    m_list.EnableItem(0x11, true, true);
    m_list.EnableItem(0x12, true, true);
    m_list.EnableItem(0x13, true, true);
    UpdateDescriptionText();
}

uint32_t UIScene_LoadMenu::LoadSaveDataThumbnailReturned(void* context, unsigned char* data,
                                                          unsigned int size)
{
    UIScene_LoadMenu* scene = static_cast<UIScene_LoadMenu*>(context);
    if (scene == NULL)
        return 0;
    if (data == NULL || size == 0)
    {
        scene->m_thumbnailFailed = true;
        scene->m_thumbnailPending = false;
        return 0;
    }
    scene->registerSubstitutionTexture(scene->m_thumbnailName, data, size, false);
    scene->m_thumbnailData = data;
    scene->m_thumbnailSize = size;
    scene->m_thumbnailLoaded = true;
    scene->m_thumbnailPending = false;
    return 0;
}

unsigned int UIScene_LoadMenu::mapElementsAndNames()
{
    IUIScene_StartGame::mapElementsAndNames();
    if (m_movie == NULL)
        return 0;
    fuiRenderNode* root = m_movie->getRootNode();
    m_mainPanel.setupControl(this, root, std::string("MainPanel"));
    m_gameName.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("GameName"));
    m_createdMode.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("CreatedMode"));
    m_description.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("Seed"));
    m_texturePackButton.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("TexturePackSelector"));
    m_playButton.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("MoreOptions"));
    m_levelIcon.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("LevelIcon"));
    m_list.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("MultiList"));
    m_controls.push_back(&m_mainPanel);
    m_controls.push_back(&m_gameName);
    m_controls.push_back(&m_createdMode);
    m_controls.push_back(&m_description);
    m_controls.push_back(&m_texturePackButton);
    m_controls.push_back(&m_playButton);
    m_controls.push_back(&m_levelIcon);
    m_controls.push_back(&m_list);
    return 1;
}

void UIScene_LoadMenu::updateTooltips()
{
    int tooltips[16] = { static_cast<int>(0x8d6323ed), 0x04f6bcf5, -1, -1, -1, -1, -1, -1,
                         -1, -1, -1, -1, -1, -1, -1, -1 };
    gConsoleUIController.SetTooltips(static_cast<unsigned int>(m_playerId), tooltips, false, false);
}

void UIScene_LoadMenu::updateComponents()
{
    if (m_layer != NULL)
    {
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x42), true);
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x43), m_publicGamesAllowed);
    }
}

unsigned int UIScene_LoadMenu::getSubSceneType() const
{
    if (m_gameMode == 0xf)
        return static_cast<unsigned int>(m_selectedItem);
    return m_battleGame ? 3 : 2;
}

void UIScene_LoadMenu::tick()
{
    IUIScene_StartGame::tick();
    if (m_customGame || m_battleGame)
    {
        UpdateIfPublicGamesAllowed();
        MiniGameDef* game = MiniGameDef::GetCustomGameModeById(
            static_cast<EMiniGameId>(m_gameMode), true);
        if (game != NULL && game->HasGameVariants())
        {
            int value = static_cast<int>(m_list.GetSliderValue(0x10));
            if (value != m_sizeValue)
            {
                m_list.SetSliderValue(0x10, m_sizeValue, true);
                m_gameModeLogic.Update(m_list, m_sizeValue);
            }
        }
    }
    if (m_loadRequested)
    {
        m_loadRequested = false;
        gConsoleUIController.NavigateToScene(m_playerId, static_cast<EUIScene>(0x4c), NULL,
                                              static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    }
}

std::wstring UIScene_LoadMenu::getMoviePath()
{
    return std::wstring(L"LoadMenu");
}

void UIScene_LoadMenu::handleReload()
{
    IUIScene_StartGame::handleReload();
    if (m_customGame || m_battleGame)
        _initCustom();
}

void UIScene_LoadMenu::handleInput(int, int action, bool repeat, bool released,
                                   bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (m_loaded)
        return;
    if (action == 0x15 && released)
    {
        if (g_pApp != NULL)
            g_pApp->SetCorruptSaveDeleted(false);
        navigateBack();
        handled = true;
        return;
    }
    if (action >= 4 && action <= 0x14)
        sendInputToMovie(action, repeat, released, pressed);
    handled = true;
}

void UIScene_LoadMenu::handlePress(int first, int second)
{
    if (m_loaded)
        return;
    gConsoleUIController.PlayUISFX(NULL);
    if (m_customGame || m_battleGame)
        first = second;
    LaunchMoreOptionsMenuInitData* initData = getLoadMenuInitData(this);
    if (first == 3)
    {
        if (!m_customGame)
        {
            bool checked = getLoadMenuOnlineCheckbox(this)->TouchIsChecked();
            if (initData->mFlag0 != checked)
            {
                initData->mFlag0 = checked;
                if (!checked)
                {
                    initData->mFlag1 = false;
                    initData->mFlag2 = false;
                }
            }
        }
        gConsoleUIController.NavigateToScene(m_playerId, static_cast<EUIScene>(0x2b), initData,
                                              static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
        return;
    }
    if (first == 4)
    {
        gConsoleUIController.PlayUISFX(NULL);
        UpdateCurrentTexturePack(second);
        return;
    }
    if (first == 0xd)
    {
        gConsoleUIController.NavigateToScene(m_playerId, static_cast<EUIScene>(0x3f), initData,
                                              static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
        return;
    }
    if (first < 6 || (first > 6 && first < 0x11) || first > 0x14)
        return;

    if (m_customGame || m_battleGame)
    {
        if (!MiniGameDef::IsCustomGame(static_cast<EMiniGameId>(m_gameMode),
                                       initData->mPlayerSettings))
            MasterGameMode::SetDefaults(*initData);

        if (m_customGame && m_gameMode == 1 &&
            GameSettings::GetBattleMiniGameMapSelection(m_playerId) != 0)
        {
            std::vector<LevelRules::LevelRulesetGroup*>& groups = initData->mLevelRulesetGroups;
            std::vector<LevelRules::LevelRulesetGroup*>::iterator group = groups.begin();
            while (group != groups.end())
            {
                LevelRules::LevelRulesetGroup* current = *group;
                LevelRuleset* ruleset = current->rulesets.front();
                LevelGenerationOptions* options = ruleset->getGenerationOptions();
                bool available = options != NULL && options->getPack() != NULL &&
                                 options->getPack()->hasPurchasedPack();
                if (!available &&
                    !gApp.mDLCManager.IsMiniGameTemporarilyFree(static_cast<int>(ruleset->getRuleType())))
                    group = groups.erase(group);
                else
                    ++group;
            }
            if (groups.empty())
            {
                int messageParameter = static_cast<int>(0xb9a1bf4d);
                gConsoleUIController.RequestAlertMessage(
                    static_cast<int>(0xac598f61), static_cast<int>(0xb9bf8844),
                    &messageParameter, 1, static_cast<unsigned int>(m_playerId), 0, NULL, NULL,
                    true);
                return;
            }
        }
    }

    bool launchOnline = initData->mFlag0;
    if (!launchOnline)
    {
        initData->mFlag1 = false;
        m_onlineGame = false;
        initData->mFlag2 = false;
    }
    else if (m_onlineGame)
    {
        initData->mFlag1 = false;
        initData->mFlag2 = false;
    }
    if (!m_customGame)
    {
        bool checked = getLoadMenuOnlineCheckbox(this)->TouchIsChecked();
        if (initData->mFlag0 != checked)
        {
            initData->mFlag0 = checked;
            if (!checked)
            {
                initData->mFlag1 = false;
                initData->mFlag2 = false;
            }
        }
    }
    StartSharedLaunchFlow();
}

void UIScene_LoadMenu::handleCheckboxToggled(int id, bool checked)
{
    gConsoleUIController.PlayUISFX(NULL);
    UIScene::handleCheckboxToggled(id, checked);
}

void UIScene_LoadMenu::handleCheckboxElementToggled(int, int second, bool checked)
{
    gConsoleUIController.PlayUISFX(NULL);
    if (second == 5)
        m_allowPublicGames = checked;
    else if (second == 10)
        m_resetNether = checked;
    else if (second == 0xb)
        m_updateEnd = checked;
    m_list.SetCheckboxValue(second, checked, true);
}

void UIScene_LoadMenu::handleSliderElementMove(int, int second, int value)
{
    gConsoleUIController.PlayUISFX(NULL);
    if (second == 0xe)
    {
        m_seedValue = value;
        m_seedLogic.Update(m_list, value);
    }
    else if (second == 0x10)
    {
        m_sizeValue = value;
        m_gameModeLogic.Update(m_list, value);
        UpdateDescriptionText();
    }
}

void UIScene_LoadMenu::handleGainFocus(bool hadFocus)
{
    if (hadFocus && !m_customGame)
        m_allowPublicGames = m_publicGamesAllowed;
    if (!m_customGame && !m_battleGame)
        return;
    UpdateIfPublicGamesAllowed();
    if (!m_customOptions)
        m_list.EnableItem(10, m_allowPublicGames && m_publicGamesAllowed, true);
}

void UIScene_LoadMenu::handleFocusChange(int focus, int childFocus)
{
    m_focus = static_cast<double>(focus);
    m_childFocus = static_cast<double>(childFocus);
    if (m_customGame || m_battleGame)
        UpdateDescriptionText();
    if (focus == 7)
        m_list.updateChildFocus(childFocus);
}

void UIScene_LoadMenu::handleTimerComplete(int timerId)
{
    if ((m_customGame || m_battleGame) && timerId == 0)
        handleGainFocus(false);
    IUIScene_StartGame::handleTimerComplete(timerId);
}

void UIScene_LoadMenu::checkDialogReturned(bool& value, C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(5))
    {
        checkStateAndStartGame();
        return;
    }
    if (result == static_cast<C4JStorage::EMessageResult>(4))
    {
        value = false;
        checkStateAndStartGame();
        return;
    }
    m_loaded = false;
    m_updateEndPending = true;
    m_resetNetherPending = true;
}

uint32_t UIScene_LoadMenu::CheckResetNetherReturned(void* context, int,
                                                    C4JStorage::EMessageResult result)
{
    UIScene* callbackScene = gConsoleUIController.GetSceneFromCallbackId(
        static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context)));
    UIScene_LoadMenu* scene = static_cast<UIScene_LoadMenu*>(callbackScene);
    if (scene != NULL)
    {
        scene->m_resetNetherPending = false;
        scene->checkDialogReturned(scene->m_resetNether, result);
    }
    return 0;
}

uint32_t UIScene_LoadMenu::CheckUpdateTheEndReturned(void* context, int,
                                                      C4JStorage::EMessageResult result)
{
    UIScene* callbackScene = gConsoleUIController.GetSceneFromCallbackId(
        static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context)));
    UIScene_LoadMenu* scene = static_cast<UIScene_LoadMenu*>(callbackScene);
    if (scene != NULL)
    {
        scene->m_updateEndPending = false;
        scene->checkDialogReturned(scene->m_updateEnd, result);
    }
    return 0;
}

void UIScene_LoadMenu::checkStateAndStartGame()
{
    if (m_resetNether && m_resetNetherPending)
        return;
    if (m_updateEnd && m_updateEndPending)
        return;
    m_resetNetherPending = true;
    m_updateEndPending = true;
    LaunchGame();
}

UIControl* UIScene_LoadMenu::GetMainPanel()
{
    return &m_mainPanel;
}

void UIScene_LoadMenu::SetEnablePlayGameButton(bool enabled)
{
    if (!m_customGame && !m_battleGame)
        m_playButton.setEnable(enabled, true);
    else
    {
        m_list.EnableItem(0x11, enabled, true);
        m_list.EnableItem(0x12, enabled, true);
        m_list.EnableItem(0x13, enabled, true);
    }
}

void UIScene_LoadMenu::HandleSaveDeviceRemoved(int)
{
    if (!m_customGame && !m_battleGame && m_movie != NULL)
        navigateBack();
}

int UIScene_LoadMenu::getSceneType()
{
    return 0x23;
}


