#include "game/ui/UIScene_LoadMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
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
#include "game/core/StringConversion.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "vendor/_SaveListDetails.hpp"
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
    int playerId;
    int saveIndex;
    LevelGenerationOptions* generationOptions;
    _SaveListDetails* saveDetails;
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
      m_focus(-1.0), m_childFocus(-1.0), m_resetNetherPending(true), m_updateEndPending(true),
      m_hasGameData(false), m_thumbnailData(NULL), m_thumbnailSize(0), m_thumbnailName()
{
    initialiseMovie();
    m_description.init(UIString(L""));
    m_createdMode.init(UIString(L10N::GetString(IDS_CREATED_IN_SURVIVAL)));
    m_publicGamesAllowed = true;
    m_updateInProgress = true;
    m_hasGameData = false;
    m_thumbnailFailed = false;
    m_thumbnailPending = true;
    m_loadRequested = false;
    m_resetNetherPending = true;
    m_updateEndPending = true;
    m_thumbnailStateUnknown = false;
    m_launchData.mMatchmaking = false;

    const LoadMenuSceneData* sceneData = static_cast<const LoadMenuSceneData*>(data);
    const EMiniGameId miniGameId = sceneData != NULL ? sceneData->miniGameId : eMiniGameId_Any;
    m_launchData.mMiniGameId = miniGameId;
    const bool gameModeLobby = m_levelGenerationOptions != NULL &&
                               m_levelGenerationOptions->isGameModeLobby();
    m_launchData.mHostMiniGame = false;
    m_launchData.mJoinMiniGame = false;
    if (!gameModeLobby && miniGameId != eMiniGameId_Any && miniGameId > eMiniGameId_Lobby)
        m_launchData.mJoinMiniGame = true;
    if (gameModeLobby && miniGameId != eMiniGameId_Any && miniGameId > eMiniGameId_Lobby)
        m_launchData.mHostMiniGame = true;

    m_texturePackList.init(std::wstring(L10N::GetString(0xf6194eb1)), 4);
    if (m_launchData.mJoinMiniGame)
        m_launchData.mOnlineGame = true;
    SetUpVariants();

    m_texturePackName.init(UIString(L""));
    m_texturePackDescription.init(UIString(L""));
    if (m_launchData.mHostMiniGame || m_launchData.mJoinMiniGame)
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
                                                     eGameSetting_Difficulty);
        wchar_t difficultyLabel[256];
        std::swprintf(difficultyLabel, 256, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY),
                      L10N::GetString(Difficulty::byId(m_difficulty)->getKey()));
        m_difficultySlider.init(UIString(difficultyLabel), 2, 0, 3, m_difficulty, false, 10, 1);
        m_texturePackList.init(std::wstring(L10N::GetString(0xf6194eb1)), 4);
        m_gameModeSlider.init(UIString(L10N::GetString(IDS_GAMEMODE_SURVIVAL)), 1, 0,
                              GameType::ADVENTURE->getId(), GameType::SURVIVAL->getId(), false, 10,
                              1);
        m_playButton.init(UIString(L10N::GetString(IDS_MORE_OPTIONS)), 3);
        m_loadSettings.init(UIString(L10N::GetString(IDS_LOAD)), 6);
        m_onlineGameCheckBox.SetEnable(true);
        bool initialOnlineChecked = m_launchData.mOnlineGame;
        if (!gProfile->IsSignedInLive(m_playerId))
            m_onlineGameCheckBox.SetEnable(false);
        if (m_launchData.mOnlineSettingChangedBySystem)
        {
            m_onlineGameCheckBox.SetEnable(false);
            initialOnlineChecked = false;
        }
        m_onlineGameCheckBox.init(UIString(L10N::GetString(IDS_ONLINE_GAME)), 5,
                                  initialOnlineChecked);

        if (m_levelGenerationOptions == NULL)
        {
            if (sceneData->saveDetails != NULL)
            {
                wchar_t thumbnailName[64];
                wcsncpy(thumbnailName, convUtf8ToWstring(sceneData->saveDetails->m_filename).c_str(), 32);
                m_thumbnailName = thumbnailName;
                if (sceneData->saveDetails->m_thumbnail == NULL)
                {
                    SAVE_DETAILS* saves = gStorage->ReturnSavesInfo();
                    gStorage->LoadSaveDataThumbnail(&saves->saves[m_saveIndex], LoadSaveDataThumbnailReturned,
                                                    reinterpret_cast<void*>(GetCallbackUniqueId()));
                    m_loadRequested = true;
                }
                else
                {
                    m_thumbnailData = sceneData->saveDetails->m_thumbnail;
                    m_thumbnailSize = sceneData->saveDetails->m_thumbnailSize;
                    m_thumbnailLoaded = true;
                }
                m_gameName.init(UIString(sceneData->saveDetails->m_title));
            }
        }
        else
        {
            m_gameName.init(UIString(m_levelGenerationOptions->getDisplayName()));
            m_launchData.mTexturePackId = m_levelGenerationOptions->getTexturePackId();
            if (m_levelGenerationOptions->requiresTexturePack())
                m_texturePackList.setEnabled(false);

            gApp.SetBaseSaveName(m_levelGenerationOptions->getBaseSavePath());
            m_state62B = m_levelGenerationOptions->getLevelHasBeenInCreative();
            m_state62C = m_levelGenerationOptions->requiresTexturePack();

            TexturePack* texturePack =
                Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(
                    m_launchData.mTexturePackId);
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

            m_createdMode.setLabel(UIString(m_state62B ? IDS_CREATED_IN_CREATIVE : IDS_CREATED_IN_SURVIVAL), false,
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
    if (!m_launchData.mHostMiniGame && !m_launchData.mJoinMiniGame)
        return;
    MiniGameDef* game = MiniGameDef::GetCustomGameModeById(eMiniGameId_Lobby, true);
    if (!game->IsCRCValid())
    {
        m_publicGamesAllowed = false;
        initData->mMatchmaking = false;
        return;
    }
    if (m_publicGamesAllowed)
    {
        game = MiniGameDef::GetCustomGameModeById(initData->mMiniGameId, true);
        if (!game->IsCRCValid())
        {
            m_publicGamesAllowed = false;
            initData->mMatchmaking = false;
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
    initData->mMatchmaking = false;
}

void UIScene_LoadMenu::UpdateDescriptionText()
{
    const int childFocus = static_cast<int>(m_childFocus);
    if (m_childFocus == -1.0 || (!m_launchData.mHostMiniGame && !m_launchData.mJoinMiniGame))
        return;

    int descriptionId = -1;
    if (childFocus == 3 || childFocus == 16 || childFocus == 17 || childFocus == 18)
    {
        descriptionId = static_cast<int>(GetVariantDesc(
            static_cast<int>(m_launchData.mMatchmakingFilters[1]), m_launchData.mHostMiniGame));
    }
    else if (childFocus == 19)
    {
        descriptionId = static_cast<int>(GetVariantDesc(
            2, m_launchData.mHostMiniGame));
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
                      g_pApp->GetHTMLColour(eMinecraftColour_HTMLColor_White));
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

    if (initData->mHostMiniGame)
    {
        MiniGameDef* mode = MiniGameDef::GetCustomGameModeById(
            initData->mMiniGameId, true);
        mode->SetDefaultMatchmakingSearchData(initData->mJoinMiniGame, initData);
        m_list.init(7);
        if (mode->HasTeamGameOption())
        {
            bool teamChecked = initData->mPlayerSettings.mHostOptions.Get(eCustomGameModeHostOption_IsTeams) != 0;
            m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(0xef2a2f6a))), 9,
                                  teamChecked);
        }

        bool allowOnlineExtras = initData->mOnlineGame && m_onlineEnabled;
        bool signedInLive = gProfile->IsSignedInLive(m_playerId);
        if (!signedInLive)
            allowOnlineExtras = false;
        if (initData->mOnlineSettingChangedBySystem)
            allowOnlineExtras = false;

        m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(IDS_ONLINE_GAME))), 5,
                              allowOnlineExtras);
        m_list.EnableItem(5, m_onlineEnabled, true);

        bool publicChecked = initData->mMatchmaking && m_publicGamesAllowed;
        m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(0x3fc1a0bd))), 10,
                              publicChecked);

        bool resetNetherChecked = initData->mAllowFriendsOfFriends && allowOnlineExtras;
        m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(0x4d7ec0cc))), 0xb,
                              resetNetherChecked);

        game = GetMiniGame();
        if (game->GetId() == 3 && initData->mHostMiniGame)
        {
            m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(0x2e1306d9))), 0x15,
                                  initData->mTimeTrial);
        }

        game = GetMiniGame();
        if (game->HasMultipleMaps() && initData->mMiniGameId != eMiniGameId_Tumble)
        {
            m_list.AddNewButton(std::wstring(L10N::GetString(static_cast<int>(IDS_MG_SELECT_MAPS))), 0xd, true);
            m_list.EnableItem(0xd, true, true);
        }

        game = GetMiniGame();
        if (game->HasGameVariants())
        {
            int value = initData->mMatchmakingFilters[1];
            m_sizeLogic = SliderLogic(0x10, static_cast<int>(0x5bd476fb),
                                      eCustomGameModeHostOption_ModeVariant,
                                      initData->mMiniGameId, true);
            m_sizeLogic.AddTo(m_list, value);
            int clamped = value - 1;
            if (clamped < 0)
                clamped = 0;
            initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, clamped);
        }

        game = GetMiniGame();
        if (game->GetId() != 3)
        {
            m_list.AddNewButton(std::wstring(L10N::GetString(static_cast<int>(IDS_MORE_OPTIONS))), 3, true);
            m_list.EnableItem(3, true, true);
        }

        AddVariantButtons(true);

        bool enableTen = allowOnlineExtras && m_publicGamesAllowed;
        m_list.EnableItem(10, enableTen, true);

        bool enableEleven = allowOnlineExtras && !initData->mMatchmaking;
        m_list.EnableItem(0xb, enableEleven, true);

        bool checkTen = allowOnlineExtras && initData->mMatchmaking && m_publicGamesAllowed;
        m_list.SetCheckboxValue(10, checkTen, true);

        bool checkEleven = allowOnlineExtras && !initData->mMatchmaking && initData->mAllowFriendsOfFriends;
        m_list.SetCheckboxValue(0xb, checkEleven, true);
    }

    if (initData->mJoinMiniGame)
    {
        MiniGameDef* mode = MiniGameDef::GetCustomGameModeById(
            initData->mMiniGameId, true);
        mode->SetDefaultMatchmakingSearchData(initData->mJoinMiniGame, initData);

        game = GetMiniGame();
        if (game->HasGameVariants())
        {
            int value = initData->mMatchmakingFilters[1];
            m_sizeLogic = SliderLogic(0x10, static_cast<int>(0x5bd476fb),
                                      eCustomGameModeHostOption_ModeVariant,
                                      initData->mMiniGameId, false);
            m_sizeLogic.AddTo(m_list, value);
            int clamped = value - 1;
            if (clamped < 0)
                clamped = 0;
            initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, clamped);
        }

        game = GetMiniGame();
        if (game->GetId() != 3 && initData->mMiniGameId != eMiniGameId_Tumble)
        {
            m_list.AddNewButton(std::wstring(L10N::GetString(static_cast<int>(IDS_MORE_OPTIONS))), 3, true);
        }

        AddVariantButtons(false);
    }

    if (!initData->mHostMiniGame)
    {
        if (!initData->mJoinMiniGame)
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

int UIScene_LoadMenu::LoadSaveDataThumbnailReturned(void* context, unsigned char* data, unsigned int size)
{
    UIScene_LoadMenu* scene = static_cast<UIScene_LoadMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context))));
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
    gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1,
                                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                   false, false);
}

void UIScene_LoadMenu::updateComponents()
{
    if (m_layer != NULL)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        m_layer->showComponent(m_playerId, EUIScene_Logo, m_publicGamesAllowed);
    }
}

unsigned int UIScene_LoadMenu::getSubSceneType() const
{
    if (m_launchData.mMiniGameId == eMiniGameId_Any)
        return static_cast<unsigned int>(m_state62C);
    return m_launchData.mJoinMiniGame ? 3 : 2;
}

void UIScene_LoadMenu::tick()
{
    m_updateInProgress = true;
    IUIScene_StartGame::tick();
    m_updateInProgress = false;
    if (gConsoleUIController.IsSceneInStack(m_playerId, EUIScene_LaunchMoreOptionsMenu))
    {
        MiniGameDef* game = MiniGameDef::GetCustomGameModeById(
            m_launchData.mMiniGameId, true);
        if (game->HasGameVariants() && m_list.IsSlider(0x10))
        {
            int value = static_cast<int>(m_list.GetSliderValue(0x10));
            if (value != m_launchData.mMatchmakingFilters[1])
            {
                m_list.SetSliderValue(0x10, m_launchData.mMatchmakingFilters[1], true);
                m_sizeLogic.Update(m_list, m_launchData.mMatchmakingFilters[1]);
            }
        }
    }
    if (m_loadRequested)
    {
        m_loadRequested = false;
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_Timer, NULL,
                                              EUILayer_Scene, EUIGroup_PlayerDefault);
    }

    if (m_thumbnailFailed)
    {
        gConsoleUIController.NavigateBack(m_playerId, false, EUIScene_LoadCreateJoinMenu,
                                          EUILayer_Count);
        return;
    }
    if (!m_thumbnailLoaded)
        return;

    m_thumbnailLoaded = false;
    m_levelIcon.setTextureName(m_thumbnailName, true);

    CMinecraftApp::PngTagData tagData;
    gApp.GetImageTextData(m_thumbnailData, m_thumbnailSize, &tagData);
    tagData.flags |= CMinecraftApp::PngTagData::eFlag_NumLoads;
    ++tagData.numLoads;
    gApp.SetCurrentImageTextData(&tagData);

    if ((tagData.flags & CMinecraftApp::PngTagData::eFlag_TexturePack) != 0)
        m_launchData.mTexturePackId = tagData.texturePack;

    if ((tagData.flags & CMinecraftApp::PngTagData::eFlag_Seed) == 0)
    {
        m_description.setLabel(UIString(L""), false, false);
    }
    else
    {
        wchar_t text[256];
        swprintf(text, 256, L"%ls: %lli", L10N::GetString(IDS_SEED), tagData.seed);
        m_description.setLabel(UIString(text), false, false);
    }

    if ((tagData.flags & CMinecraftApp::PngTagData::eFlag_HostOptions) != 0)
    {
        unsigned int hostOptions = tagData.hostOptions;
        m_launchData.mPvP = gApp.GetGameHostOption(hostOptions, eGameHostOption_PvP) != 0;
        m_launchData.mTrustPlayers = gApp.GetGameHostOption(hostOptions, eGameHostOption_TrustPlayers) != 0;
        m_launchData.mFireSpreads = gApp.GetGameHostOption(hostOptions, eGameHostOption_FireSpreads) != 0;
        m_launchData.mTNT = gApp.GetGameHostOption(hostOptions, eGameHostOption_TNT) != 0;
        m_launchData.mHostPrivileges = gApp.GetGameHostOption(hostOptions, eGameHostOption_CheatsEnabled) != 0;
        m_launchData.mDisableSaving = gApp.GetGameHostOption(hostOptions, eGameHostOption_DisableSaving) != 0;
        m_launchData.mCurrentWorldSize = gApp.GetGameHostOption(hostOptions, eGameHostOption_WorldSize);
        m_launchData.mWorldSize = m_launchData.mCurrentWorldSize;
        m_launchData.mMobGriefing = gApp.GetGameHostOption(hostOptions, eGameHostOption_MobGriefing) != 0;
        m_launchData.mKeepInventory = gApp.GetGameHostOption(hostOptions, eGameHostOption_KeepInventory) != 0;
        m_launchData.mDoMobSpawning = gApp.GetGameHostOption(hostOptions, eGameHostOption_DoMobSpawning) != 0;
        m_launchData.mDoMobLoot = gApp.GetGameHostOption(hostOptions, eGameHostOption_DoMobLoot) != 0;
        m_launchData.mDoTileDrops = gApp.GetGameHostOption(hostOptions, eGameHostOption_DoTileDrops) != 0;
        m_launchData.mNaturalRegeneration = gApp.GetGameHostOption(hostOptions, eGameHostOption_NaturalRegeneration) != 0;
        m_launchData.mDoDaylightCycle = gApp.GetGameHostOption(hostOptions, eGameHostOption_DoDaylightCycle) != 0;
        m_launchData.mDoWeatherCycle = gApp.GetGameHostOption(hostOptions, eGameHostOption_DoWeatherCycle) != 0;
        if (!m_launchData.mHostPrivileges)
        {
            m_launchData.mKeepInventory = false;
            m_launchData.mMobGriefing = true;
            m_launchData.mDoMobSpawning = true;
            m_launchData.mDoDaylightCycle = true;
        }
        if (gApp.GetGameHostOption(hostOptions, eGameHostOption_WasntSaveOwner) != 0)
            m_state629 = false;

        m_state62B = gApp.GetGameHostOption(hostOptions, eGameHostOption_HasBeenInCreative) != 0;
        m_createdMode.setLabel(UIString(L10N::GetString(m_state62B ? IDS_CREATED_IN_CREATIVE : IDS_CREATED_IN_SURVIVAL)), false, false);

        unsigned int gameMode = gApp.GetGameHostOption(hostOptions, eGameHostOption_GameType);
        if (gameMode == 1)
        {
            m_gameModeSlider.setLabel(UIString(L10N::GetString(IDS_GAMEMODE_CREATIVE)), false, false);
            m_isCreative = true;
            m_gameTypeId = GameType::CREATIVE->getId();
        }
        else if (gameMode == 2)
        {
            m_gameModeSlider.setLabel(UIString(L10N::GetString(IDS_GAMEMODE_ADVENTURE)), false, false);
            m_isCreative = false;
            m_gameTypeId = GameType::ADVENTURE->getId();
        }
        else
        {
            m_gameModeSlider.setLabel(UIString(L10N::GetString(IDS_GAMEMODE_SURVIVAL)), false, false);
            m_isCreative = false;
            m_gameTypeId = GameType::SURVIVAL->getId();
        }
        m_gameModeSlider.SetSliderValue(m_gameTypeId);

        unsigned int onlineSetting = GameSettings::GetGameSettings(m_playerId, eGameSetting_Online);
        if (gApp.GetGameHostOption(hostOptions, eGameHostOption_FriendsOfFriends) != 0 && !m_onlineEnabled
            && onlineSetting == 0)
        {
            m_launchData.mAllowFriendsOfFriends = true;
        }

        m_difficulty = gApp.GetGameHostOption(eGameHostOption_Difficulty);
        m_difficultySlider.SetSliderValue(m_difficulty);
        m_difficultySlider.handleSliderMove(m_difficulty);
        wchar_t text[256];
        swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY),
                 L10N::GetString(Difficulty::byId(m_difficulty)->getKey()));
        m_difficultySlider.setLabel(UIString(text), false, false);
    }

    if ((tagData.flags & CMinecraftApp::PngTagData::eFlag_ExtraData) != 0
        && gApp.GetExtraGameData(tagData.extraData, eGameExtraData_SaveVersion) > EXTRA_DATA_SAVE_VERSION)
    {
        m_hasGameData = true;
        m_loadSettings.setEnable(false, true);
    }

    m_selectedSlot = getCurrentTextureSelectorIndex(m_launchData.mTexturePackId);
    UpdateTexturePackDescription(m_launchData.mTexturePackId);
    m_texturePackList.selectSlot(m_selectedSlot);
    gConsoleUIController.NavigateBack(m_playerId, false, static_cast<EUIScene>(getSceneType()),
                                      EUILayer_Count);
}

std::wstring UIScene_LoadMenu::getMoviePath()
{
    return std::wstring(L"LoadMenu");
}

void UIScene_LoadMenu::handleReload()
{
    IUIScene_StartGame::handleReload();
    if (m_launchData.mHostMiniGame || m_launchData.mJoinMiniGame)
        _initCustom();
}

void UIScene_LoadMenu::handleInput(int, int action, bool repeat, bool released,
                                   bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (m_actionPending)
        return;
    if (action == EControllerActions_MenuCancel && released)
    {
        if (g_pApp != NULL)
            g_pApp->SetCorruptSaveDeleted(false);
        navigateBack();
        handled = true;
        return;
    }
    if (action >= EControllerActions_MenuUp && action <= EControllerActions_MenuOk)
        sendInputToMovie(action, repeat, released, pressed);
    handled = true;
}

void UIScene_LoadMenu::handlePress(int controlId, int childId)
{
    if (m_actionPending)
        return;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (m_launchData.mHostMiniGame || m_launchData.mJoinMiniGame)
        controlId = childId;
    LaunchMoreOptionsMenuInitData* initData = &m_launchData;
    if (controlId == 3)
    {
        if (!m_launchData.mHostMiniGame)
        {
            bool checked = m_onlineGameCheckBox.TouchIsChecked();
            if (initData->mOnlineGame != checked)
            {
                initData->mOnlineGame = checked;
                if (!checked)
                {
                    initData->mInviteOnly = false;
                    initData->mAllowFriendsOfFriends = false;
                }
            }
        }
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_LaunchMoreOptionsMenu, initData,
                                              EUILayer_Scene, EUIGroup_PlayerDefault);
        return;
    }
    if (controlId == 4)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
        UpdateCurrentTexturePack(childId);
        return;
    }
    if (controlId == 0xd)
    {
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_MapSelectMenu, initData,
                                              EUILayer_Scene, EUIGroup_PlayerDefault);
        return;
    }
    if (controlId < 6 || (controlId > 6 && controlId < 0x11) || controlId > 0x14)
        return;

    if (m_launchData.mHostMiniGame || m_launchData.mJoinMiniGame)
    {
        if (!MiniGameDef::IsCustomGame(m_launchData.mMiniGameId,
                                       initData->mPlayerSettings))
            MasterGameMode::SetDefaults(*initData);

        if (m_launchData.mHostMiniGame && m_launchData.mMiniGameId == eMiniGameId_Battle &&
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
                int messageParameter = static_cast<int>(IDS_CONFIRM_OK);
                gConsoleUIController.RequestAlertMessage(
                    static_cast<int>(IDS_MG_NO_MAPS_SELECTED_TITLE), static_cast<int>(IDS_MG_NO_MAPS_SELECTED_TEXT),
                    &messageParameter, 1, static_cast<unsigned int>(m_playerId), 0, NULL, NULL,
                    true);
                return;
            }
        }
    }

    bool launchOnline = initData->mOnlineGame;
    if (!launchOnline)
    {
        initData->mInviteOnly = false;
        initData->mAllowFriendsOfFriends = false;
    }
    else if (initData->mInviteOnly)
    {
        initData->mInviteOnly = false;
        initData->mAllowFriendsOfFriends = false;
    }
    if (!m_launchData.mHostMiniGame)
    {
        bool checked = m_onlineGameCheckBox.TouchIsChecked();
        if (initData->mOnlineGame != checked)
        {
            initData->mOnlineGame = checked;
            if (!checked)
            {
                initData->mInviteOnly = false;
                initData->mAllowFriendsOfFriends = false;
            }
        }
    }
    StartSharedLaunchFlow();
}

void UIScene_LoadMenu::handleCheckboxToggled(int id, bool checked)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    UIScene::handleCheckboxToggled(id, checked);
}

void UIScene_LoadMenu::handleCheckboxElementToggled(int, int second, bool checked)
{
    if (m_updateInProgress)
        return;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (second < 10)
    {
        if (second == 5)
        {
            m_launchData.mOnlineGame = checked;
            m_list.EnableItem(10, checked && m_publicGamesAllowed, false);
            if (GetMiniGame()->HasMultipleMaps())
                m_list.EnableItem(0xd, true, false);

            bool allowOnline = checked && m_onlineEnabled &&
                               gProfile->IsSignedInLive(m_playerId) && !m_launchData.mOnlineSettingChangedBySystem;
            m_list.EnableItem(0xb, allowOnline && !m_launchData.mMatchmaking, false);
            m_list.SetCheckboxValue(10, checked && m_launchData.mMatchmaking && m_publicGamesAllowed,
                                    false);
            m_list.SetCheckboxValue(0xb, checked && !m_launchData.mMatchmaking && m_launchData.mAllowFriendsOfFriends,
                                    false);
            return;
        }
        if (second == 8)
        {
            m_launchData.mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_CentreSpawn, checked);
            return;
        }
        if (second == 9)
        {
            m_launchData.mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_IsTeams, checked);
            return;
        }
        return;
    }

    if (second < 0xb)
    {
        m_launchData.mMatchmaking = checked;
        if (checked)
            m_launchData.mOnlineGame = true;
        if (GetMiniGame()->HasMultipleMaps())
            m_list.EnableItem(0xd, true, false);

        bool allowOnline = m_launchData.mOnlineGame && m_onlineEnabled &&
                           gProfile->IsSignedInLive(m_playerId) && !m_launchData.mOnlineSettingChangedBySystem;
        if (GetMiniGame()->GetId() == 3 && m_launchData.mHostMiniGame)
        {
            if (checked)
                m_launchData.mTimeTrial = false;
            m_list.SetCheckboxValue(0x15,
                                    !m_launchData.mMatchmaking && !m_launchData.mOnlineGame &&
                                        m_launchData.mTimeTrial,
                                    false);
        }
        m_list.EnableItem(0xb, allowOnline && !m_launchData.mMatchmaking, false);
        m_list.SetCheckboxValue(0xb, allowOnline && !m_launchData.mMatchmaking && m_launchData.mAllowFriendsOfFriends,
                                false);
        return;
    }

    if (second == 0xb)
    {
        m_launchData.mAllowFriendsOfFriends = checked;
        return;
    }
    if (second != 0x15)
        return;

    m_launchData.mTimeTrial = checked;
    m_launchData.mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_TimeTrial, checked);
    bool allowOnline = m_launchData.mOnlineGame && m_onlineEnabled &&
                       gProfile->IsSignedInLive(m_playerId) && !m_launchData.mOnlineSettingChangedBySystem;
    m_list.EnableItem(5, !checked && m_onlineEnabled, false);
    m_list.EnableItem(10, !checked && allowOnline && m_publicGamesAllowed, false);
    m_list.EnableItem(0xb, !checked && allowOnline && !m_launchData.mMatchmaking, false);
    m_list.SetCheckboxValue(5, !checked && allowOnline, false);
    m_list.SetCheckboxValue(10, !checked && allowOnline && m_launchData.mMatchmaking &&
                                    m_publicGamesAllowed,
                            false);
    m_list.SetCheckboxValue(0xb, !checked && allowOnline && !m_launchData.mMatchmaking &&
                                    m_launchData.mAllowFriendsOfFriends,
                            false);
}

void UIScene_LoadMenu::handleSliderElementMove(int, int second, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    if (second == 0xe)
    {
        m_launchData.mMatchmakingFilters[0] = value;
        m_seedLogic.Update(m_list, value);
    }
    else if (second == 0x10)
    {
        m_launchData.mMatchmakingFilters[1] = value;
        int variant = value - 1;
        if (variant < 0)
            variant = 0;
        m_launchData.mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, variant);
        m_sizeLogic.Update(m_list, value);
        UpdateDescriptionText();
    }
}

void UIScene_LoadMenu::handleGainFocus(bool hadFocus)
{
    LaunchMoreOptionsMenuInitData* initData = &m_launchData;
    if (hadFocus && !initData->mHostMiniGame)
        m_onlineGameCheckBox.setChecked(initData->mOnlineGame);
    if (!initData->mHostMiniGame && !initData->mJoinMiniGame)
        return;
    UpdateIfPublicGamesAllowed();
    if (initData->mHostMiniGame)
    {
        bool allowOnlineExtras = initData->mOnlineGame && m_onlineEnabled;
        if (!gProfile->IsSignedInLive(m_playerId))
            allowOnlineExtras = false;
        if (initData->mOnlineSettingChangedBySystem)
            allowOnlineExtras = false;
        if (!initData->mTimeTrial)
        {
            m_list.EnableItem(10, allowOnlineExtras && m_publicGamesAllowed, true);
            m_list.SetCheckboxValue(
                10, initData->mOnlineGame && initData->mMatchmaking && m_publicGamesAllowed, true);
        }
    }
}

void UIScene_LoadMenu::handleFocusChange(int focus, int childFocus)
{
    m_focus = static_cast<double>(focus);
    m_childFocus = static_cast<double>(childFocus);
    if (m_launchData.mHostMiniGame || m_launchData.mJoinMiniGame)
        UpdateDescriptionText();
    if (focus == 7)
        m_list.updateChildFocus(childFocus);
}

void UIScene_LoadMenu::handleTimerComplete(int timerId)
{
    if ((m_launchData.mHostMiniGame || m_launchData.mJoinMiniGame) && timerId == 0)
        handleGainFocus(false);
    IUIScene_StartGame::handleTimerComplete(timerId);
}

void UIScene_LoadMenu::checkDialogReturned(bool& value, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Decline)
    {
        checkStateAndStartGame();
        return;
    }
    if (result == C4JStorage::EMessageResult_Accept)
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
        scene->checkDialogReturned(scene->m_launchData.mResetNether, result);
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
        scene->checkDialogReturned(scene->m_launchData.mResetEnd, result);
    }
    return 0;
}

void UIScene_LoadMenu::checkStateAndStartGame()
{
    LaunchMoreOptionsMenuInitData* initData = &m_launchData;
    if (initData->mResetNether && m_resetNetherPending)
    {
        int parameters[2] = { static_cast<int>(IDS_DONT_RESET_NETHER), static_cast<int>(IDS_RESET_NETHER) };
        gConsoleUIController.RequestAlertMessage(
            static_cast<int>(IDS_RESETNETHER_TITLE), static_cast<int>(IDS_RESETNETHER_TEXT), parameters, 2,
            static_cast<unsigned int>(m_playerId),
            reinterpret_cast<int (*)(void*, int, C4JStorage::EMessageResult)>(CheckResetNetherReturned),
            reinterpret_cast<void*>(GetCallbackUniqueId()), NULL, true);
        return;
    }
    if (initData->mResetEnd && m_updateEndPending)
    {
        int parameters[2] = { static_cast<int>(IDS_DONT_RESET_THE_END), static_cast<int>(IDS_RESET_THE_END) };
        gConsoleUIController.RequestAlertMessage(
            static_cast<int>(IDS_RESET_THE_END_TITLE), static_cast<int>(IDS_RESET_THE_END_TEXT), parameters, 2,
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
    if (!m_launchData.mHostMiniGame && !m_launchData.mJoinMiniGame)
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
    if (!m_launchData.mHostMiniGame && !m_launchData.mJoinMiniGame && m_movie != NULL)
        navigateBack();
}

int UIScene_LoadMenu::getSceneType()
{
    return EUIScene_LoadMenu;
}

