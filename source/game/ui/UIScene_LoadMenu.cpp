#include "game/ui/UIScene_LoadMenu.hpp"

#include <cwchar>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/FJ_LoadMenu.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIControl_CheckBox.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "platform/wiiu/CProfile.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

namespace
{
struct LoadMenuSceneData
{
    int reserved0;
    int saveIndex;
    LevelGenerationOptions* generationOptions;
    int reserved12;
    EMiniGameId miniGameId;
};
}

UIScene_LoadMenu::UIScene_LoadMenu(int sceneType, void* data, UILayer* layer)
    : IUIScene_StartGame(sceneType, data, layer, true), m_seedLogic(), m_sizeLogic(),
      m_gameModeLogic(), m_difficultyLogic(), m_mainPanel(), m_gameName(), m_description(),
      m_createdMode(), m_playButton(), m_loadSettings(), m_levelIcon(), m_list(),
      m_thumbnailPending(true), m_thumbnailLoaded(false), m_loadRequested(false),
      m_thumbnailStateUnknown(false), m_thumbnailFailed(false), m_updateInProgress(false),
      m_publicGamesAllowed(true),
      m_focus(-1.0), m_childFocus(-1.0), m_resetNetherPending(false), m_updateEndPending(false),
      m_hasGameData(false), m_thumbnailData(NULL), m_thumbnailSize(0), m_thumbnailName()
{
    initialiseMovie();
    m_description.init(UIString(L""));
    m_createdMode.init(UIString(L10N::GetString(0x108a4753)));
    m_publicGamesAllowed = true;
    m_updateInProgress = true;
    m_hasGameData = false;
    m_thumbnailFailed = false;
    m_thumbnailPending = true;
    m_loadRequested = false;
    m_resetNetherPending = false;
    m_updateEndPending = false;
    m_thumbnailStateUnknown = false;
    m_launchData.mFlag58 = false;

    const LoadMenuSceneData* sceneData = static_cast<const LoadMenuSceneData*>(data);
    const int miniGameId = sceneData != NULL ? static_cast<int>(sceneData->miniGameId) : 15;
    m_launchData.mValue5C = miniGameId;
    const bool gameModeLobby = m_levelGenerationOptions != NULL &&
                               m_levelGenerationOptions->isGameModeLobby();
    m_launchData.mFlag56 = false;
    m_launchData.mFlag57 = false;
    if (!gameModeLobby && miniGameId != 15 && miniGameId > 0)
        m_launchData.mFlag57 = true;
    if (gameModeLobby && miniGameId != 15 && miniGameId > 0)
        m_launchData.mFlag56 = true;

    m_texturePackList.init(std::wstring(L10N::GetString(0xf6194eb1)), 4);
    if (m_launchData.mFlag57)
        m_launchData.mFlag0 = true;
    SetUpVariants();

    m_texturePackName.init(UIString(L""));
    m_texturePackDescription.init(UIString(L""));
    if (m_launchData.mFlag56 || m_launchData.mFlag57)
    {
        UpdateIfPublicGamesAllowed();
        m_gameTypeId = GameType::LOBBY->getId();
        m_difficulty = 3;
        MasterGameMode::SetDefaults(m_launchData);
        _initCustom();
    }
    else
    {
        m_state614 = 0;
        m_difficulty = GameSettings::GetGameSettings(m_playerId,
                                                     static_cast<eGameSetting>(3));
        wchar_t difficultyLabel[256];
        std::swprintf(difficultyLabel, 256, L"%ls: %ls", L10N::GetString(0xd324428d),
                      L10N::GetString(Difficulty::byId(m_difficulty)->getKey()));
        m_difficultySlider.init(UIString(difficultyLabel), 2, 0, 3, m_difficulty, false, 10, 1);
        m_texturePackList.init(std::wstring(L10N::GetString(0xf6194eb1)), 4);
        m_gameModeSlider.init(UIString(L10N::GetString(0x9ba782a6)), 1, 0,
                              GameType::ADVENTURE->getId(), GameType::SURVIVAL->getId(), false, 10,
                              1);
        m_playButton.init(UIString(L10N::GetString(0xbe1d5b9e)), 3);
        m_loadSettings.init(UIString(L10N::GetString(0x7be5c322)), 6);
        m_onlineGameCheckBox.SetEnable(true);
        bool initialOnlineChecked = m_launchData.mFlag0;
        if (!gProfile->IsSignedInLive(m_playerId))
            m_onlineGameCheckBox.SetEnable(false);
        if (m_launchData.mFlag17)
        {
            m_onlineGameCheckBox.SetEnable(false);
            initialOnlineChecked = false;
        }
        m_onlineGameCheckBox.init(UIString(L10N::GetString(0x2c692b72)), 5,
                                  initialOnlineChecked);

        if (m_levelGenerationOptions != NULL)
        {
            m_gameName.init(UIString(m_levelGenerationOptions->getDisplayName()));
            m_launchData.mValue1C = m_levelGenerationOptions->getTexturePackId();
            if (m_levelGenerationOptions->requiresTexturePack())
                m_texturePackList.setEnabled(false);

            gApp.SetBaseSaveName(m_levelGenerationOptions->getBaseSavePath());
            m_state62B = m_levelGenerationOptions->getLevelHasBeenInCreative();
            m_state62C = m_levelGenerationOptions->requiresTexturePack();

            TexturePack* texturePack =
                Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(
                    m_launchData.mValue1C);
            if (texturePack != NULL)
            {
                unsigned int iconSize = 0;
                const void* icon = texturePack->getPackIcon(iconSize);
                if (icon != NULL && iconSize != 0)
                {
                    wchar_t iconName[64];
                    swprintf(iconName, 64, L"loadsave");
                    registerSubstitutionTexture(
                        iconName, reinterpret_cast<unsigned char*>(const_cast<void*>(icon)),
                        iconSize, false);
                    m_levelIcon.setTextureName(iconName, true);
                }
            }

            m_createdMode.setLabel(UIString(m_state62B ? 0x61833048 : 0x108a4753), false,
                                   false);
        }
    }
    _init();
    if (sceneData != NULL)
        ::operator delete(const_cast<LoadMenuSceneData*>(sceneData));
    gConsoleUIController.TouchBoxRebuild(this);
    m_updateInProgress = false;
}

UIScene_LoadMenu::~UIScene_LoadMenu()
{
}

void UIScene_LoadMenu::AddVariantButtons(bool alternate)
{
    unsigned int label = GetVariantPlayLabel(alternate ? 1 : 0, alternate);
    m_list.AddNewButton(std::wstring(L10N::GetString(static_cast<int>(label))),
                        alternate ? 0x12 : 0x11, true);
}

void UIScene_LoadMenu::UpdateIfPublicGamesAllowed()
{
    m_publicGamesAllowed = true;
    LaunchMoreOptionsMenuInitData* initData = &m_launchData;
    if (!m_launchData.mFlag56 && !m_launchData.mFlag57)
        return;
    MiniGameDef* game = MiniGameDef::GetCustomGameModeById(eMiniGameId_Lobby, true);
    if (!game->IsCRCValid())
    {
        m_publicGamesAllowed = false;
        initData->mFlag58 = false;
        return;
    }
    if (m_publicGamesAllowed)
    {
        game = MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(initData->mValue5C), true);
        if (!game->IsCRCValid())
        {
            m_publicGamesAllowed = false;
            initData->mFlag58 = false;
            return;
        }
        if (m_publicGamesAllowed)
        {
            if (m_levelGenerationOptions != NULL)
                m_publicGamesAllowed = m_levelGenerationOptions->isCRCValid();

            if (m_publicGamesAllowed)
            {
                std::vector<LevelRules::LevelRulesetGroup*>::iterator group =
                    initData->mLevelRulesetGroups.begin();
                while (group != initData->mLevelRulesetGroups.end() && m_publicGamesAllowed)
                {
                    LevelRules::LevelRulesetGroup* current = *group;
                    std::vector<LevelRuleset*>::iterator ruleset = current->rulesets.begin();
                    while (ruleset != current->rulesets.end() && m_publicGamesAllowed)
                    {
                        m_publicGamesAllowed = (*ruleset)->getGenerationOptions()->isCRCValid();
                        ++ruleset;
                    }
                    ++group;
                }
            }
        }
    }
    initData->mFlag58 = false;
}

void UIScene_LoadMenu::UpdateDescriptionText()
{
    const int childFocus = static_cast<int>(m_childFocus);
    if (m_childFocus == -1.0 || (!m_launchData.mFlag56 && !m_launchData.mFlag57))
        return;

    int descriptionId = -1;
    if (childFocus == 3 || childFocus == 16 || childFocus == 17 || childFocus == 18)
    {
        descriptionId = static_cast<int>(GetVariantDesc(
            static_cast<int>(m_launchData.mValue64), m_launchData.mFlag56));
    }
    else if (childFocus == 19)
    {
        descriptionId = static_cast<int>(GetVariantDesc(
            2, m_launchData.mFlag56));
    }

    std::wstring description;
    if (descriptionId != -1 && descriptionId != -2 && g_pApp != NULL)
    {
        description = g_pApp->FormatHTMLString(
            m_playerId, std::wstring(L10N::GetString(descriptionId)), -1);
    }
    if (g_pApp != NULL)
    {
        wchar_t colourTag[64];
        std::swprintf(colourTag, 64, L"<font color=\"#%08x\">",
                      g_pApp->GetHTMLColour(static_cast<eMinecraftColour>(0x1b5)));
        description.insert(0, colourTag);
        m_texturePackDescription.setLabel(UIString(description), false, false);
    }
}

void UIScene_LoadMenu::_initCustom()
{
    LaunchMoreOptionsMenuInitData* initData = &m_launchData;

    static_cast<FJ_LoadMenu*>(m_movie->getRootNode()->getFuiNode())->InitCustomLoadMenu();
    m_description.removeUINodeRef();
    m_texturePackList.removeUINodeRef();
    m_gameModeSlider.removeUINodeRef();
    m_onlineGameCheckBox.removeUINodeRef();
    m_playButton.removeUINodeRef();
    m_loadSettings.removeUINodeRef();
    m_difficultySlider.removeUINodeRef();
    m_texturePackName.removeUINodeRef();
    m_texturePackIcon.removeUINodeRef();

    MiniGameDef* game = GetMiniGame();
    m_gameName.init(UIString(L10N::GetString(static_cast<int>(game->GetName()))));
    std::wstring textureName = game->getTextureName();
    m_levelIcon.setTextureName(textureName, true);
    m_createdMode.init(UIString(L""));

    if (initData->mFlag56)
    {
        MiniGameDef* mode = MiniGameDef::GetCustomGameModeById(
            static_cast<EMiniGameId>(initData->mValue5C), true);
        mode->SetDefaultMatchmakingSearchData(initData->mFlag57, initData);
        m_list.init(7);
        if (mode->HasTeamGameOption())
        {
            bool teamChecked = initData->mPlayerSettings.Get(eCustomGameModeHostOption_IsTeams) != 0;
            m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(0xef2a2f6a))), 9,
                                  teamChecked);
        }

        bool allowOnlineExtras = initData->mFlag0 && m_onlineEnabled;
        bool signedInLive = gProfile->IsSignedInLive(m_playerId);
        if (!signedInLive)
            allowOnlineExtras = false;
        if (initData->mFlag17)
            allowOnlineExtras = false;

        m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(0x2c692b72))), 5,
                              allowOnlineExtras);
        m_list.EnableItem(5, m_onlineEnabled, true);

        bool publicChecked = initData->mFlag58 && m_publicGamesAllowed;
        m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(0x3fc1a0bd))), 10,
                              publicChecked);

        bool resetNetherChecked = initData->mFlag2 && allowOnlineExtras;
        m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(0x4d7ec0cc))), 0xb,
                              resetNetherChecked);

        game = GetMiniGame();
        if (game->GetId() == 3 && initData->mFlag56)
        {
            m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(0x2e1306d9))), 0x15,
                                  initData->mFlag68);
        }

        game = GetMiniGame();
        if (game->HasMultipleMaps() && initData->mValue5C != 2)
        {
            m_list.AddNewButton(std::wstring(L10N::GetString(static_cast<int>(0x88c46bf1))), 0xd, true);
            m_list.EnableItem(0xd, true, true);
        }

        game = GetMiniGame();
        if (game->HasGameVariants())
        {
            int value = initData->mValue64;
            m_sizeLogic = SliderLogic(0x10, static_cast<int>(0x5bd476fb),
                                      eCustomGameModeHostOption_ModeVariant,
                                      static_cast<EMiniGameId>(initData->mValue5C), true);
            m_sizeLogic.AddTo(m_list, value);
            int clamped = value - 1;
            if (clamped < 0)
                clamped = 0;
            initData->mPlayerSettings.Set(eCustomGameModeHostOption_GameVariant, clamped);
        }

        game = GetMiniGame();
        if (game->GetId() != 3)
        {
            m_list.AddNewButton(std::wstring(L10N::GetString(static_cast<int>(0xbe1d5b9e))), 3, true);
            m_list.EnableItem(3, true, true);
        }

        AddVariantButtons(true);

        bool enableTen = allowOnlineExtras && m_publicGamesAllowed;
        m_list.EnableItem(10, enableTen, true);

        bool enableEleven = allowOnlineExtras && !initData->mFlag58;
        m_list.EnableItem(0xb, enableEleven, true);

        bool checkTen = allowOnlineExtras && initData->mFlag58 && m_publicGamesAllowed;
        m_list.SetCheckboxValue(10, checkTen, true);

        bool checkEleven = allowOnlineExtras && !initData->mFlag58 && initData->mFlag2;
        m_list.SetCheckboxValue(0xb, checkEleven, true);
    }

    if (initData->mFlag57)
    {
        MiniGameDef* mode = MiniGameDef::GetCustomGameModeById(
            static_cast<EMiniGameId>(initData->mValue5C), true);
        mode->SetDefaultMatchmakingSearchData(initData->mFlag57, initData);

        game = GetMiniGame();
        if (game->HasGameVariants())
        {
            int value = initData->mValue64;
            m_sizeLogic = SliderLogic(0x10, static_cast<int>(0x5bd476fb),
                                      eCustomGameModeHostOption_ModeVariant,
                                      static_cast<EMiniGameId>(initData->mValue5C), false);
            m_sizeLogic.AddTo(m_list, value);
            int clamped = value - 1;
            if (clamped < 0)
                clamped = 0;
            initData->mPlayerSettings.Set(eCustomGameModeHostOption_GameVariant, clamped);
        }

        game = GetMiniGame();
        if (game->GetId() != 3 && initData->mValue5C != 2)
        {
            m_list.AddNewButton(std::wstring(L10N::GetString(static_cast<int>(0xbe1d5b9e))), 3, true);
        }

        AddVariantButtons(false);
    }

    if (!initData->mFlag56)
    {
        if (!initData->mFlag57)
        {
            int count = static_cast<int>(m_list.getItemCount());
            m_list.setCurrentSelection(count - 1, false, true);
            UpdateDescriptionText();
            static_cast<FJ_LoadMenu*>(m_movie->getRootNode()->getFuiNode())
                ->RescaleCustomLoadMenu(false);
        }
        else
        {
            int index = m_list.getListIndex(0x11);
            m_list.setCurrentSelection(index, false, true);
            m_childFocus = 17.0;
            UpdateDescriptionText();
            static_cast<FJ_LoadMenu*>(m_movie->getRootNode()->getFuiNode())
                ->RescaleCustomLoadMenu(false);
        }
    }
    else
    {
        int index = m_list.getListIndex(0x12);
        m_list.setCurrentSelection(index, false, true);
        m_childFocus = 18.0;
        UpdateDescriptionText();
        static_cast<FJ_LoadMenu*>(m_movie->getRootNode()->getFuiNode())
            ->RescaleCustomLoadMenu(false);
    }
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
    m_texturePackList.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("TexturePackSelector"));
    m_gameModeSlider.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("GameModeToggle"));
    m_onlineGameCheckBox.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("CheckboxOnline"));
    m_playButton.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("MoreOptions"));
    m_loadSettings.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("LoadSettings"));
    m_difficultySlider.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("Difficulty"));
    m_levelIcon.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("LevelIcon"));
    m_list.setupControl(this, m_mainPanel.getFuiRenderNode(), std::string("MultiList"));
    m_controls.push_back(&m_mainPanel);
    m_controls.push_back(&m_gameName);
    m_controls.push_back(&m_createdMode);
    m_controls.push_back(&m_description);
    m_controls.push_back(&m_texturePackList);
    m_controls.push_back(&m_gameModeSlider);
    m_controls.push_back(&m_onlineGameCheckBox);
    m_controls.push_back(&m_playButton);
    m_controls.push_back(&m_loadSettings);
    m_controls.push_back(&m_difficultySlider);
    m_controls.push_back(&m_levelIcon);
    m_controls.push_back(&m_list);
    return 1;
}

void UIScene_LoadMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, 0x8d6323ed, 0x4f6bcf5e, -1, -1, -1, -1,
                                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                   false, false);
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
    if (m_launchData.mValue5C == 0xf)
        return static_cast<unsigned int>(m_state62C);
    return m_launchData.mFlag57 ? 3 : 2;
}

void UIScene_LoadMenu::tick()
{
    m_updateInProgress = true;
    IUIScene_StartGame::tick();
    m_updateInProgress = false;
    if (gConsoleUIController.IsSceneInStack(m_playerId, static_cast<EUIScene>(0x2b)))
    {
        MiniGameDef* game = MiniGameDef::GetCustomGameModeById(
            static_cast<EMiniGameId>(m_launchData.mValue5C), true);
        if (game != NULL && game->HasGameVariants())
        {
            int value = static_cast<int>(m_list.GetSliderValue(0x10));
            if (value != m_launchData.mValue64)
            {
                m_list.SetSliderValue(0x10, m_launchData.mValue64, true);
                m_sizeLogic.Update(m_list, m_launchData.mValue64);
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
    if (m_launchData.mFlag56 || m_launchData.mFlag57)
        _initCustom();
}

void UIScene_LoadMenu::handleInput(int, int action, bool repeat, bool released,
                                   bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (m_actionPending)
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
    if (m_actionPending)
        return;
    gConsoleUIController.PlayUISFX(NULL);
    if (m_launchData.mFlag56 || m_launchData.mFlag57)
        first = second;
    LaunchMoreOptionsMenuInitData* initData = &m_launchData;
    if (first == 3)
    {
        if (!m_launchData.mFlag56)
        {
            bool checked = m_onlineGameCheckBox.TouchIsChecked();
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

    if (m_launchData.mFlag56 || m_launchData.mFlag57)
    {
        if (!MiniGameDef::IsCustomGame(static_cast<EMiniGameId>(m_launchData.mValue5C),
                                       initData->mPlayerSettings))
            MasterGameMode::SetDefaults(*initData);

        if (m_launchData.mFlag56 && m_launchData.mValue5C == 1 &&
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
        initData->mFlag2 = false;
    }
    else if (initData->mFlag1)
    {
        initData->mFlag1 = false;
        initData->mFlag2 = false;
    }
    if (!m_launchData.mFlag56)
    {
        bool checked = m_onlineGameCheckBox.TouchIsChecked();
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
    if (m_updateInProgress)
        return;
    gConsoleUIController.PlayUISFX(NULL);
    if (second < 10)
    {
        if (second == 5)
        {
            m_launchData.mFlag0 = checked;
            m_list.EnableItem(10, checked && m_publicGamesAllowed, false);
            if (GetMiniGame()->HasMultipleMaps())
                m_list.EnableItem(0xd, true, false);

            bool allowOnline = checked && m_onlineEnabled &&
                               gProfile->IsSignedInLive(m_playerId) && !m_launchData.mFlag17;
            m_list.EnableItem(0xb, allowOnline && !m_launchData.mFlag58, false);
            m_list.SetCheckboxValue(10, checked && m_launchData.mFlag58 && m_publicGamesAllowed,
                                    false);
            m_list.SetCheckboxValue(0xb, checked && !m_launchData.mFlag58 && m_launchData.mFlag2,
                                    false);
            return;
        }
        if (second == 8)
        {
            m_launchData.mPlayerSettings.Set(eCustomGameModeHostOption(4), checked);
            return;
        }
        if (second == 9)
        {
            m_launchData.mPlayerSettings.Set(eCustomGameModeHostOption(8), checked);
            return;
        }
        return;
    }

    if (second < 0xb)
    {
        m_launchData.mFlag58 = checked;
        if (checked)
            m_launchData.mFlag0 = true;
        if (GetMiniGame()->HasMultipleMaps())
            m_list.EnableItem(0xd, true, false);

        bool allowOnline = m_launchData.mFlag0 && m_onlineEnabled &&
                           gProfile->IsSignedInLive(m_playerId) && !m_launchData.mFlag17;
        if (GetMiniGame()->GetId() == 3 && m_launchData.mFlag56)
        {
            if (checked)
                m_launchData.mFlag68 = false;
            m_list.SetCheckboxValue(0x15,
                                    !m_launchData.mFlag58 && !m_launchData.mFlag0 &&
                                        m_launchData.mFlag68,
                                    false);
        }
        m_list.EnableItem(0xb, allowOnline && !m_launchData.mFlag58, false);
        m_list.SetCheckboxValue(0xb, allowOnline && !m_launchData.mFlag58 && m_launchData.mFlag2,
                                false);
        return;
    }

    if (second == 0xb)
    {
        m_launchData.mFlag2 = checked;
        return;
    }
    if (second != 0x15)
        return;

    m_launchData.mFlag68 = checked;
    m_launchData.mPlayerSettings.Set(eCustomGameModeHostOption(0xe), checked);
    bool allowOnline = m_launchData.mFlag0 && m_onlineEnabled &&
                       gProfile->IsSignedInLive(m_playerId) && !m_launchData.mFlag17;
    m_list.EnableItem(5, !checked && m_onlineEnabled, false);
    m_list.EnableItem(10, !checked && allowOnline && m_publicGamesAllowed, false);
    m_list.EnableItem(0xb, !checked && allowOnline && !m_launchData.mFlag58, false);
    m_list.SetCheckboxValue(5, !checked && allowOnline, false);
    m_list.SetCheckboxValue(10, !checked && allowOnline && m_launchData.mFlag58 &&
                                    m_publicGamesAllowed,
                            false);
    m_list.SetCheckboxValue(0xb, !checked && allowOnline && !m_launchData.mFlag58 &&
                                    m_launchData.mFlag2,
                            false);
}

void UIScene_LoadMenu::handleSliderElementMove(int, int second, int value)
{
    gConsoleUIController.PlayUISFX(NULL);
    if (second == 0xe)
    {
        m_launchData.mValue60 = value;
        m_seedLogic.Update(m_list, value);
    }
    else if (second == 0x10)
    {
        m_launchData.mValue64 = value;
        int variant = value - 1;
        if (variant < 0)
            variant = 0;
        m_launchData.mPlayerSettings.Set(eCustomGameModeHostOption_GameVariant, variant);
        m_sizeLogic.Update(m_list, value);
        UpdateDescriptionText();
    }
}

void UIScene_LoadMenu::handleGainFocus(bool hadFocus)
{
    LaunchMoreOptionsMenuInitData* initData = &m_launchData;
    if (hadFocus && !initData->mFlag56)
        m_onlineGameCheckBox.setChecked(initData->mFlag0);
    if (!initData->mFlag56 && !initData->mFlag57)
        return;
    UpdateIfPublicGamesAllowed();
    if (initData->mFlag56)
    {
        bool allowOnlineExtras = initData->mFlag0 && m_onlineEnabled;
        if (!gProfile->IsSignedInLive(m_playerId))
            allowOnlineExtras = false;
        if (initData->mFlag17)
            allowOnlineExtras = false;
        if (!initData->mFlag68)
        {
            m_list.EnableItem(10, allowOnlineExtras && m_publicGamesAllowed, true);
            m_list.SetCheckboxValue(
                10, initData->mFlag0 && initData->mFlag58 && m_publicGamesAllowed, true);
        }
    }
}

void UIScene_LoadMenu::handleFocusChange(int focus, int childFocus)
{
    m_focus = static_cast<double>(focus);
    m_childFocus = static_cast<double>(childFocus);
    if (m_launchData.mFlag56 || m_launchData.mFlag57)
        UpdateDescriptionText();
    if (focus == 7)
        m_list.updateChildFocus(childFocus);
}

void UIScene_LoadMenu::handleTimerComplete(int timerId)
{
    if ((m_launchData.mFlag56 || m_launchData.mFlag57) && timerId == 0)
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
    m_actionPending = false;
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
        scene->checkDialogReturned(scene->m_launchData.mFlagD, result);
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
        scene->checkDialogReturned(scene->m_launchData.mFlagE, result);
    }
    return 0;
}

void UIScene_LoadMenu::checkStateAndStartGame()
{
    LaunchMoreOptionsMenuInitData* initData = &m_launchData;
    if (initData->mFlagD && m_resetNetherPending)
    {
        int parameters[2] = { static_cast<int>(0x9405ecf0), static_cast<int>(0xc13fdfc6) };
        gConsoleUIController.RequestAlertMessage(
            static_cast<int>(0xe5d1250e), static_cast<int>(0x79240783), parameters, 2,
            static_cast<unsigned int>(m_playerId),
            reinterpret_cast<int (*)(void*, int, C4JStorage::EMessageResult)>(CheckResetNetherReturned),
            reinterpret_cast<void*>(GetCallbackUniqueId()), NULL, true);
        return;
    }
    if (initData->mFlagE && m_updateEndPending)
    {
        int parameters[2] = { static_cast<int>(0x9dcd2f11), static_cast<int>(0xd75a3077) };
        gConsoleUIController.RequestAlertMessage(
            static_cast<int>(0x50d08380), static_cast<int>(0x6fdf0a5d), parameters, 2,
            static_cast<unsigned int>(m_playerId),
            reinterpret_cast<int (*)(void*, int, C4JStorage::EMessageResult)>(CheckUpdateTheEndReturned),
            reinterpret_cast<void*>(GetCallbackUniqueId()), NULL, true);
        return;
    }
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
    if (!m_launchData.mFlag56 && !m_launchData.mFlag57)
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
    if (!m_launchData.mFlag56 && !m_launchData.mFlag57 && m_movie != NULL)
        navigateBack();
}

int UIScene_LoadMenu::getSceneType()
{
    return 0x23;
}

