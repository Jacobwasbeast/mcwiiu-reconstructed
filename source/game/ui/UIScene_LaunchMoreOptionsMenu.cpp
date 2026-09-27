#include "game/ui/UIScene_LaunchMoreOptionsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/L10N.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/StringUtils.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/FJ_LaunchMoreOptionsMenu.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/SuperflatMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/client/render/Renderer.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/GameSettings.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

static std::wstring makeValueLabel(int labelId, int value)
{
    wchar_t text[256];
    std::swprintf(text, 256, L"%ls: %d", L10N::GetString(labelId), value);
    return std::wstring(text);
}

static std::wstring makeTextLabel(int labelId, int valueId)
{
    wchar_t text[256];
    std::swprintf(text, 256, L"%ls: %ls", L10N::GetString(labelId), L10N::GetString(valueId));
    return std::wstring(text);
}

static std::wstring makeArenaSizeLabel(int value)
{
    static const float displayValues[] = { 1.0f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f };
    wchar_t display[64];
    wchar_t text[256];
    std::swprintf(display, 64, L"%g", displayValues[value]);
    std::swprintf(text, 256, L"%ls: x%ls", L10N::GetString(0x36df2177), display);
    return std::wstring(text);
}

UIScene_LaunchMoreOptionsMenu::UIScene_LaunchMoreOptionsMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), m_worldSize(), m_worldType(), m_gameMode(), m_lootSet(),
      m_foodBalance(), m_mapSize(), m_roundLength(), m_worldTab(), m_gameTab(), m_worldOptions(),
      m_gameOptions(), m_worldBackground(), m_gameBackground(), m_worldOptionsList(),
      m_gameOptionsList(), m_worldLabel(), m_gameLabel(), m_resizeLabel(), m_worldDescription(),
      m_gameDescription(), m_worldTabLabel(), m_gameTabLabel(), m_textPanelLabel(), m_optionsLabel(),
      m_initData(static_cast<LaunchMoreOptionsMenuInitData*>(data)), m_online(false),
      m_keyboardActive(false), m_menuType(0), m_requestKeyboard(false), m_updateCheats(false),
      m_updateOnline(false), m_seed()
{
    initialiseMovie();
    m_online = gProfile->IsSignedInLive(m_initData->mPad) &&
               gProfile->AllowedToPlayMultiplayer(m_initData->mPad);
    m_menuType = static_cast<int>(m_initData->mNewWorld);
    _init();
    addTimer(0, 100);
}

UIScene_LaunchMoreOptionsMenu::~UIScene_LaunchMoreOptionsMenu()
{
}

void UIScene_LaunchMoreOptionsMenu::EnableCustomOptions(bool enabled)
{
    EMiniGameId miniGameId = m_initData->mMiniGameId;
    if (miniGameId != eMiniGameId_Battle && miniGameId != eMiniGameId_Tumble)
        return;
    if (m_initData->mHostMiniGame)
    {
        m_gameOptionsList.EnableItem(0x22, enabled, true);
        m_gameOptionsList.EnableItem(0x1e, enabled, true);
        miniGameId = m_initData->mMiniGameId;
    }
    if (miniGameId == eMiniGameId_Battle)
    {
        if (m_initData->mJoinMiniGame || !m_initData->mHostMiniGame)
            return;
        m_gameOptionsList.EnableItem(0x1f, enabled, true);
        m_gameOptionsList.EnableItem(0x20, enabled, true);
        m_gameOptionsList.EnableItem(0x21, enabled, true);
        m_gameOptionsList.EnableItem(0x23, enabled, true);
        m_gameOptionsList.EnableItem(0x24, enabled, true);
        m_gameOptionsList.EnableItem(0x25, enabled, true);
        m_gameOptionsList.EnableItem(0x27, enabled, true);
        m_gameOptionsList.EnableItem(0x28, enabled, true);
        m_gameOptionsList.EnableItem(0x29, enabled, true);
        unsigned int noSmallSkins = m_initData->mPlayerSettings.mHostOptions.Get(eCustomGameModeHostOption_NoSmallSkins);
        m_gameOptionsList.SetCheckboxValue(0x1f, noSmallSkins == 0, true);
        m_gameOptionsList.SetCheckboxValue(0x28, m_initData->mPlayerSettings.mTakeEverythingEnabled, true);
        m_gameOptionsList.SetCheckboxValue(0x29, m_initData->mPlayerSettings.mChestRefillEnabled, true);
    }
    else if (miniGameId == eMiniGameId_Tumble && !m_initData->mJoinMiniGame && m_initData->mHostMiniGame)
    {
        m_gameOptionsList.EnableItem(0x20, enabled, true);
        m_gameOptionsList.EnableItem(0x2b, enabled, true);
        m_gameOptionsList.EnableItem(0x2c, enabled, true);
        m_gameOptionsList.EnableItem(0x2a, enabled, true);
    }
}

void UIScene_LaunchMoreOptionsMenu::UpdateCheats()
{
    bool cheatsEnabled = m_initData->mHostPrivileges;
    if (!m_initData->mHostMiniGame && !m_initData->mJoinMiniGame)
    {
        m_gameOptionsList.EnableItem(0xc, cheatsEnabled, true);
        m_gameOptionsList.EnableItem(0xd, cheatsEnabled, true);
        m_gameOptionsList.EnableItem(0xf, cheatsEnabled, true);
        m_gameOptionsList.EnableItem(0x12, cheatsEnabled, true);
        m_gameOptionsList.EnableItem(0x13, cheatsEnabled, true);
    }
    if (!cheatsEnabled)
    {
        m_initData->mDoMobSpawning = true;
        m_initData->mMobGriefing = true;
        m_initData->mKeepInventory = false;
        m_initData->mDoDaylightCycle = true;
        m_initData->mDoWeatherCycle = true;
        if (!m_initData->mHostMiniGame && !m_initData->mJoinMiniGame)
        {
            m_gameOptionsList.SetCheckboxValue(0xc, false, true);
            m_gameOptionsList.SetCheckboxValue(0xd, m_initData->mDoMobSpawning, true);
            m_gameOptionsList.SetCheckboxValue(0xf, m_initData->mMobGriefing, true);
            m_gameOptionsList.SetCheckboxValue(0x12, m_initData->mDoDaylightCycle, true);
            m_gameOptionsList.SetCheckboxValue(0x13, m_initData->mDoWeatherCycle, true);
        }
    }
}

void UIScene_LaunchMoreOptionsMenu::UpdateDescriptionText()
{
    if (m_focusedItemId == -1)
        return;

    static const unsigned int descriptionIds[] = {
        0x1a93e279, 0x0f7151d3, 0x7b994768, 0x2df040b0, 0x2bdbddba, 0xf2e6dbcb,
        0xc85a085b, 0x8f41adb5, 0xa112a17c, 0xcb6d8de1, 0x30f1cb60, 0xc61e984d,
        0xe0163d88, 0xcf045afa, 0x03d32989, 0x544ad58e, 0xaf1fca77, 0x33b6226b,
        0x80e6ea53, 0x2bd798a9, 0xc36b9de0, 0x6d6eba08, 0xa6874a79, 0x8f88ae9a,
        IDS_DESC_BIOME_SCALE, 0x814065bc, 0x4f8bedac, 0x75cf2428, 0x4c17ef92, IDS_MASHUP_CHOOSE_BETWEEN_SEVERAL_VARIANTS_SAME_CORE_GAME_TEXT,
        0xfe516d5e, 0xbe68971f, 0, 0, IDS_MG_LIVES_PER_ROUND_DESC, 0x5e1bb988, 0x2a5c55c8,
        0xb66f0561, 0xf7c16b2c, 0xe7ba74fc, 0xe8f6de23, 0xbfd9bf09, 0xa541f7ca,
        0x0d7dae19, 0x35ef3122
    };
    const int index = m_focusedItemId;
    if (index < 0 || index >= static_cast<int>(sizeof(descriptionIds) / sizeof(descriptionIds[0])))
        return;
    const unsigned int descriptionId = descriptionIds[index];
    std::wstring description;
    if (descriptionId != static_cast<unsigned int>(-1) && descriptionId != static_cast<unsigned int>(-2))
        description = g_pApp->FormatHTMLString(m_playerId, L10N::GetString(static_cast<int>(descriptionId)), -1);
    if (descriptionId == 0xc36b9de0u)
        description = replaceAll(description, L"{*DISABLES_ACHIEVEMENTS*}", L10N::GetString(IDS_HOST_OPTION_DISABLES_ACHIEVEMENTS));
    const bool hiDef = gRenderer->IsHiDef();
    if (!hiDef)
        gRenderer->IsWidescreen();
    if (index == 0x15)
    {
        if (!m_initData->mFlatWorld)
            description = L10N::GetString(0xaa2e57af);
        else
            description = std::wstring(L"\n\n") + FlatGeneratorInfo(&m_initData->mSuperflatConfig).generateDescription();
    }
    else if (index == 0x20)
        description = MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)->GetLootSetDescription(m_gameOptionsList.GetSliderValue(0x20));
    else if (index == 0x21)
        description = MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)->GetFoodBalanceIndexDescription(m_gameOptionsList.GetSliderValue(0x21));
    if (descriptionId == 0xc36b9de0u)
        description = replaceAll(description, L"{*DISABLES_ACHIEVEMENTS*}", L10N::GetString(IDS_HOST_OPTION_DISABLES_ACHIEVEMENTS));
    wchar_t prefix[64];
    swprintf(prefix, 64, L"<font color=\"#%08x\">", g_pApp->GetHTMLColour(eMinecraftColour_HTMLColor_White));
    description.insert(0, prefix);
    UIString value(description);
    if (!m_initData->mHostMiniGame && !m_initData->mJoinMiniGame && m_menuType == 1)
        m_worldDescription.setLabel(value, false, false);
    else
        m_gameDescription.setLabel(value, false, false);
    if (m_initData->mNewWorld)
        m_worldOptionsList.SetItemLabel(0x16, UIString(m_initData->mSeed), false);
}
void UIScene_LaunchMoreOptionsMenu::_init()
{
    mapElementsAndNames();

    m_worldLabel.init(UIString(L10N::GetString(IDS_WORLD_OPTIONS)));
    m_gameLabel.init(UIString(L10N::GetString(IDS_GAME_OPTIONS)));
    m_focusedControlId = -1;
    m_focusedItemId = -1;

    const EMiniGameId miniGameId = m_initData->mMiniGameId;
    const bool custom = m_initData->mHostMiniGame;
    const bool joining = m_initData->mJoinMiniGame;
    const std::wstring blank(L" ");
    const std::wstring section(L10N::GetString(IDS_GAME_OPTIONS));

    if (!custom && !joining)
    {
        m_worldOptionsList.AddNewLabel(std::wstring(L10N::GetString(0xa60b975e)), 0x34);
        m_worldOptionsList.AddNewTextInput(m_initData->mSeed, 0x16, 0x3c);
        m_worldOptionsList.AddNewLabel(std::wstring(L10N::GetString(0xae4164b2)), 0x34);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_FIND_BALANCED)), 0x17,
                                          m_initData->mFindBalancedSeed);
        m_worldOptionsList.AddNewLabel(blank, 0x34);
        m_worldOptionsList.AddNewLabel(std::wstring(L10N::GetString(IDS_BIOME_SCALE)), 0x34);
        const int biomeLabels[] = { static_cast<int>(IDS_SMALL), static_cast<int>(IDS_MEDIUM), static_cast<int>(IDS_LARGE) };
        m_worldOptionsList.AddNewSlider(std::wstring(L10N::GetString(biomeLabels[m_initData->mBiomeScale])), 0x18, 0, 2,
                                        m_initData->mBiomeScale, false, 7, 3);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_GENERATE_STRUCTURES)), 10,
                                          m_initData->mTrustPlayers);
        m_worldOptionsList.EnableItem(10, !m_initData->mFireSpreads, true);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_BONUS_CHEST)), 0xb,
                                          m_initData->mBonusChest);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_SUPERFLAT_WORLD)), 0x14,
                                          m_initData->mFlatWorld);
        m_worldOptionsList.EnableItem(0x14, !m_initData->mWorldTypeLocked, true);
        m_worldOptionsList.AddNewButton(std::wstring(L10N::GetString(0x9cfef661)), 0x15, true);
        m_worldOptionsList.EnableItem(0x15, m_initData->mFlatWorld && !m_initData->mWorldTypeLocked, true);
        m_worldOptionsList.AddNewLabel(blank, 0x34);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_FIRE_SPREADS)), 5,
                                          m_initData->mFireSpreads);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_TNT_EXPLODES)), 6,
                                          m_initData->mTNT);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_TRUST_PLAYERS)), 4,
                                          m_initData->mTrustPlayers);
        if (m_initData->mNewWorld)
            m_focusedItemId = 0x16;
    }

    if (custom || joining)
    {
        m_gameOptionsList.AddNewLabel(section, 0x34);
        MiniGameDef* game = MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(miniGameId), true);
        if (game->HasGameVariants())
        {
            m_foodBalance = SliderLogic(0x1d, 0x5bd476fb, eCustomGameModeHostOption_ModeVariant,
                                         static_cast<EMiniGameId>(miniGameId), !joining);
            m_foodBalance.AddTo(m_gameOptionsList, m_initData->mMatchmakingFilters[1]);
            int modeVariant = m_initData->mMatchmakingFilters[1] - 1;
            if (modeVariant < 0)
                modeVariant = 0;
            m_initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, modeVariant);
        }
        if (game->HasMultipleMaps() && miniGameId != eMiniGameId_Tumble)
        {
            m_gameOptionsList.AddNewButton(std::wstring(L10N::GetString(IDS_MG_SELECT_MAPS)), 0x1b, true);
            m_gameOptionsList.EnableItem(0x1b, true, true);
        }
        if (game->HasRoundLengthOption())
        {
            m_lootSet = SliderLogic(0x1c, 0x908811f4, eCustomGameModeHostOption_MapVariant,
                                    static_cast<EMiniGameId>(miniGameId), !joining);
            m_lootSet.AddTo(m_gameOptionsList, m_initData->mMatchmakingFilters[0]);
        }
        if (game->HasCentralSpawnOption())
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0xef2a2f6a)), 0x1a,
                                             m_initData->mPlayerSettings.mHostOptions.Get(eCustomGameModeHostOption_CentreSpawn) != 0);
        if (miniGameId == eMiniGameId_Battle || miniGameId == eMiniGameId_Tumble)
        {
            m_gameOptionsList.AddNewLabel(blank, 0x34);
            m_gameOptionsList.AddNewLabel(std::wstring(L10N::GetString(IDS_MG_CUSTOM_OPTIONS)), 0x34);
            const MiniGameDef* miniGame = game;
            const int lives = static_cast<int>(m_initData->mPlayerSettings.mLivesPerRound);
            const std::wstring livesLabel = lives == 0
                ? makeTextLabel(IDS_MG_LIVES_PER_ROUND, IDS_MG_INFINITE)
                : makeValueLabel(IDS_MG_LIVES_PER_ROUND, lives);
            m_gameOptionsList.AddNewSlider(livesLabel, 0x22,
                                           miniGame->HasFeatureSupportForInfiniteLives() ? 0 : 1,
                                           10, lives, false, 7, 3);
            m_worldType = SliderLogic(0x1e, 0x901c1fe8,
                                     eCustomGameModeHostOption_SpectateMode,
                                     static_cast<EMiniGameId>(miniGameId), false);
            m_worldType.AddTo(m_gameOptionsList,
                               m_initData->mPlayerSettings.mHostOptions.Get(eCustomGameModeHostOption_SpectateMode));
        }
        if (miniGameId == eMiniGameId_Battle)
        {
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0xa329d6e9)), 0x1f,
                                             m_initData->mPlayerSettings.mHostOptions.Get(eCustomGameModeHostOption_NoSmallSkins) == 0);
            const int lootSet = static_cast<int>(m_initData->mPlayerSettings.mLootSetId);
            m_gameOptionsList.AddNewSlider(
                std::wstring(L10N::GetString(0x5ccdb447)) + L": " + game->GetLootSetText(lootSet),
                0x20, 0, 5, lootSet, false, 7, 3);
            const int foodBalance = m_initData->mPlayerSettings.mFoodBalance;
            m_gameOptionsList.AddNewSlider(
                std::wstring(L10N::GetString(0x1cbfab4a)) + L": " + game->GetFoodBalanceIndexText(foodBalance),
                0x21, 0, 7, foodBalance, false, 7, 3);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x7356945d)), 0x28,
                                             m_initData->mPlayerSettings.mTakeEverythingEnabled);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0xaa27b127)), 0x29,
                                             m_initData->mPlayerSettings.mChestRefillEnabled);
        }
        else if (miniGameId == eMiniGameId_Tumble)
        {
            const int lootSet = static_cast<int>(m_initData->mPlayerSettings.mLootSetId);
            m_gameOptionsList.AddNewSlider(
                std::wstring(L10N::GetString(0x5ccdb447)) + L": " + game->GetLootSetText(lootSet),
                0x20, 0, 3, lootSet, false, 7, 3);
            int arenaSize = 0;
            if (m_initData->mPlayerSettings.mArenaSizeMultiplier.isSet())
            {
                const float value = m_initData->mPlayerSettings.mArenaSizeMultiplier.get();
                if (value == 0.5f)
                    arenaSize = 1;
                else if (value == 0.625f)
                    arenaSize = 2;
                else if (value == 0.75f)
                    arenaSize = 3;
                else if (value == 0.875f)
                    arenaSize = 4;
                else if (value == 1.0f)
                    arenaSize = 5;
            }
            const std::wstring arenaLabel = !m_initData->mPlayerSettings.mArenaSizeMultiplier.isSet()
                ? makeTextLabel(0x36df2177, IDS_MG_AUTO)
                : makeArenaSizeLabel(arenaSize);
            m_gameOptionsList.AddNewSlider(arenaLabel, 0x2b, 0, 5, arenaSize, true, 7, 3);

            const int layerCount = m_initData->mPlayerSettings.mLayerCount.isSet()
                ? static_cast<int>(m_initData->mPlayerSettings.mLayerCount.get()) : 0;
            const std::wstring layerLabel = layerCount < 1
                ? makeTextLabel(0x50849384, IDS_MG_AUTO)
                : makeValueLabel(0x50849384, layerCount);
            m_gameOptionsList.AddNewSlider(layerLabel, 0x2c, 0, 8, layerCount, false, 7, 3);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x40ea8efc)), 0x2a,
                                             m_initData->mPlayerSettings.mSpectatorParticipation);
        }
    }
    else
    {
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_ONLINE_GAME)), 0, m_initData->mOnlineGame);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x4d7ec0cc)), 2, m_initData->mAllowFriendsOfFriends);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_PLAYER_VS_PLAYER)), 3, m_initData->mPvP);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_HOST_PRIVILEGES)), 7, m_initData->mHostPrivileges);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_DAYLIGHT_CYCLE)), 0x12, m_initData->mDoDaylightCycle);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_WEATHER_CYCLE)), 0x13, m_initData->mDoWeatherCycle);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_KEEP_INVENTORY)), 0xc, m_initData->mKeepInventory);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_SPAWNING)), 0xd, m_initData->mDoMobSpawning);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_GRIEFING)), 0xf, m_initData->mMobGriefing);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_LOOT)), 0xe, m_initData->mDoMobLoot);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_TILE_DROPS)), 0x10, m_initData->mDoTileDrops);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x6f7d8591)), 0x11, m_initData->mNaturalRegeneration);
    }

    static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->SetMenuType(!m_initData->mNewWorld);
    if (m_initData->mHostMiniGame || m_initData->mJoinMiniGame)
        static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->HideWorldTab();
    if (!m_initData->mHostMiniGame && !m_initData->mJoinMiniGame)
    {
        if (!m_online)
        {
            m_gameOptionsList.EnableItem(0, false, true);
            m_gameOptionsList.EnableItem(2, false, true);
        }
        if (!m_initData->mOnlineGame)
        {
            m_gameOptionsList.EnableItem(2, false, true);
            m_gameOptionsList.EnableItem(0, false, true);
        }
    }

    UpdateCheats();
    UpdateDescriptionText();
    m_worldTab.init(0x34);
    m_gameTab.init(0x35);
    updateTooltips();
    updateComponents();
    gConsoleUIController.TouchBoxRebuild(this);
}

unsigned int UIScene_LaunchMoreOptionsMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_gameLabel.setupControl(this, root, std::string("LabelGame"));
    m_worldLabel.setupControl(this, root, std::string("LabelWorld"));
    m_worldOptions.setupControl(this, root, std::string("WorldOptions"));
    m_worldOptionsList.setupControl(this, m_worldOptions.getFuiRenderNode(), std::string("WorldOptionsList"));
    m_worldTab.setupControl(this, m_worldOptions.getFuiRenderNode(), std::string("TouchTabWorld"));
    m_worldDescription.setupControl(this, m_worldOptions.getFuiRenderNode(), std::string("Description_WorldOptions"));
    m_worldTabLabel.setupControl(this, m_worldOptions.getFuiRenderNode(), std::string("WorldSize"));
    m_resizeLabel.setupControl(this, root, std::string("ResizeLabel"));
    m_gameOptions.setupControl(this, root, std::string("GameOptions"));
    m_gameOptionsList.setupControl(this, m_gameOptions.getFuiRenderNode(), std::string("GameOptionsList"));
    m_gameTab.setupControl(this, m_gameOptions.getFuiRenderNode(), std::string("TouchTabGame"));
    m_gameDescription.setupControl(this, m_gameOptions.getFuiRenderNode(), std::string("Description_GameOptions"));
    m_controls.push_back(&m_gameLabel);
    m_controls.push_back(&m_worldLabel);
    m_controls.push_back(&m_worldOptions);
    m_controls.push_back(&m_worldOptionsList);
    m_controls.push_back(&m_worldTab);
    m_controls.push_back(&m_worldDescription);
    m_controls.push_back(&m_worldTabLabel);
    m_controls.push_back(&m_resizeLabel);
    m_controls.push_back(&m_gameOptions);
    m_controls.push_back(&m_gameOptionsList);
    m_controls.push_back(&m_gameTab);
    m_controls.push_back(&m_gameDescription);
    return 1;
}

void UIScene_LaunchMoreOptionsMenu::updateTooltips()
{
    if (!m_initData->mHostMiniGame && !m_initData->mJoinMiniGame)
    {
        const int tabLabel = m_menuType == 0 ? IDS_WORLD_OPTIONS : IDS_GAME_OPTIONS;
        gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -2, tabLabel, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
    }
    else
    {
        gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
    }
}

void UIScene_LaunchMoreOptionsMenu::updateComponents()
{
    if (m_layer != NULL)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        m_layer->showComponent(m_playerId, EUIScene_Logo, false);
    }
}

std::wstring UIScene_LaunchMoreOptionsMenu::getMoviePath()
{
    if (g_pApp != NULL && g_pApp->GetLocalPlayerCount() > 1)
        return std::wstring(L"LaunchMoreOptionsMenuSplit");
    return std::wstring(L"LaunchMoreOptionsMenu");
}

int UIScene_LaunchMoreOptionsMenu::KeyboardCompleteSeedCallback(void* data, bool accepted)
{
    UIScene_LaunchMoreOptionsMenu* scene = static_cast<UIScene_LaunchMoreOptionsMenu*>(data);
    scene->m_keyboardActive = false;
    if (accepted)
    {
        wchar_t text[128];
        g_pInput->GetText(text, 0x7f);
        scene->m_worldOptionsList.SetItemLabel(0x16, UIString(text), false);
        scene->m_initData->mSeed.assign(text);
        scene->m_initData->mSettingsChanged = true;
    }
    return 0;
}

int UIScene_LaunchMoreOptionsMenu::HandleRequestKeyboard()
{
    m_keyboardActive = true;
    m_seed.assign(m_initData->mSeed);
    if (m_seed == L" ")
        m_seed.assign(L"");
    const wchar_t* title = L10N::GetString(0xa60b975e);
    g_pInput->RequestKeyboard(title, m_seed.c_str(), m_playerId, 0x3c,
                              KeyboardCompleteSeedCallback, this,
                              C_4JInput::EKeyboardMode_0);
    return 0;
}

void UIScene_LaunchMoreOptionsMenu::UpdateOnline()
{
    if (m_initData->mHostMiniGame && m_initData->mMatchmaking)
        return;
    if (m_initData->mJoinMiniGame)
        return;
    m_gameOptionsList.EnableItem(2, m_initData->mOnlineGame, true);
}

void UIScene_LaunchMoreOptionsMenu::tick()
{
    UIScene::tick();
    bool online = gProfile->IsSignedInLive(m_initData->mPad) &&
                  gProfile->AllowedToPlayMultiplayer(m_initData->mPad);
    if (online != m_online)
    {
        m_gameOptionsList.EnableItem(0, online, true);
        m_gameOptionsList.EnableItem(2, online, true);
        if (online)
        {
            m_gameOptionsList.SetCheckboxValue(0, true, true);
            m_gameOptionsList.SetCheckboxValue(2, true, true);
        }
        m_online = online;
    }
    if (m_requestKeyboard)
    {
        HandleRequestKeyboard();
        m_requestKeyboard = false;
    }
    if (m_updateCheats)
    {
        UpdateCheats();
        m_updateCheats = false;
    }
    if (m_updateOnline)
    {
        UpdateOnline();
        m_updateOnline = false;
    }
}

void UIScene_LaunchMoreOptionsMenu::handleDestroy()
{
    if (g_pInput != NULL)
        g_pInput->DestroyKeyboard();
}

void UIScene_LaunchMoreOptionsMenu::handleReload()
{
    int savedItemId = m_focusedItemId;
    _init();
    static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->SetActiveTab(m_menuType, false);
    m_focusedItemId = savedItemId;
    if (m_menuType == 0)
    {
        int selection = m_gameOptionsList.getListIndex(savedItemId);
        m_gameOptionsList.setCurrentSelection(selection, false, true);
    }
    else
    {
        int selection = m_worldOptionsList.getListIndex(savedItemId);
        m_worldOptionsList.setCurrentSelection(selection, false, true);
    }
}

void UIScene_LaunchMoreOptionsMenu::handleInput(int, int action, bool repeat, bool released,
                                                bool pressed, bool& handled)
{
    if (m_keyboardActive)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
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
        sendInputToMovie(action, repeat, released, pressed);
        handled = true;
        break;
    case EControllerActions_MenuRightScroll:
    case EControllerActions_MenuLeftScroll:
        if (released)
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
            m_menuType = m_menuType == 0;
            updateTooltips();
            static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->ChangeTab();
            gConsoleUIController.TouchBoxRebuild(this);
        }
        break;
    case EControllerActions_MenuCancel:
        if (released)
        {
            if (m_initData->mHostMiniGame || m_initData->mJoinMiniGame)
                MasterGameMode::SaveDefaults(*m_initData);
            navigateBack();
            handled = true;
        }
        break;
    }
}

void UIScene_LaunchMoreOptionsMenu::handleFocusChange(int focus, int childFocus)
{
    m_focusedItemId = childFocus;
    m_focusedControlId = focus;
    UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleTimerComplete(int)
{
}

void UIScene_LaunchMoreOptionsMenu::handlePress(int controlId, int childId)
{
    if (m_keyboardActive)
        return;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (controlId == 0x16)
    {
        m_requestKeyboard = true;
        return;
    }
    if (childId == 0x15)
    {
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SuperFlatMenu, m_initData,
                                             EUILayer_Scene, EUIGroup_PlayerDefault);
        return;
    }
    if (childId == 0x19)
    {
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_BiomePreviewMenu, m_initData,
                                             EUILayer_Scene, EUIGroup_PlayerDefault);
        return;
    }
    if (childId == 0x1b)
    {
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_MapSelectMenu, m_initData,
                                             EUILayer_Scene, EUIGroup_PlayerDefault);
        return;
    }
    if (childId != 0x2d)
        return;
    if (m_initData->mJoinMiniGame)
    {
        MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true);
        miniGame->SetDefaultMatchmakingSearchData(true, m_initData);
        m_gameOptionsList.SetSliderValue(0x2e, m_initData->mMatchmakingFilters[0], false);
        m_lootSet.Update(m_gameOptionsList, m_initData->mMatchmakingFilters[0]);
        m_gameOptionsList.SetSliderValue(0x2f, m_initData->mMatchmakingFilters[1], false);
        m_foodBalance.Update(m_gameOptionsList, m_initData->mMatchmakingFilters[1]);
        UpdateDescriptionText();
        return;
    }
    if (m_initData->mHostMiniGame && m_initData->mMatchmaking)
    {
        MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true);
        miniGame->SetDefaultMatchmakingSearchData(false, m_initData);
        m_gameOptionsList.SetSliderValue(0x2e, m_initData->mMatchmakingFilters[0], false);
        m_lootSet.Update(m_gameOptionsList, m_initData->mMatchmakingFilters[0]);
        m_gameOptionsList.SetSliderValue(0x2f, m_initData->mMatchmakingFilters[1], false);
        m_foodBalance.Update(m_gameOptionsList, m_initData->mMatchmakingFilters[1]);
        UpdateDescriptionText();
    }
}

void UIScene_LaunchMoreOptionsMenu::handleSliderElementMove(int, int second, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    switch (second)
    {
    case 0x18:
    {
        static const int biomeLabels[] = { static_cast<int>(IDS_SMALL), static_cast<int>(IDS_MEDIUM),
                                           static_cast<int>(IDS_LARGE) };
        m_initData->mBiomeScale = static_cast<unsigned char>(value);
        m_worldOptionsList.SetItemLabel(0x18, UIString(L10N::GetString(biomeLabels[value])), false);
        m_initData->mSettingsChanged = true;
        return;
    }
    case 0x1c:
        m_initData->mMatchmakingFilters[0] = value;
        m_lootSet.Update(m_gameOptionsList, value);
        m_initData->mSettingsChanged = true;
        return;
    case 0x1d:
    {
        int modeVariant = value - 1;
        if (modeVariant < 0)
            modeVariant = 0;
        m_initData->mMatchmakingFilters[1] = value;
        m_initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, modeVariant);
        m_foodBalance.Update(m_gameOptionsList, value);
        EnableCustomOptions(MiniGameDef::IsCustomGame(m_initData->mMiniGameId,
                                                      m_initData->mPlayerSettings));
        m_initData->mSettingsChanged = true;
        return;
    }
    case 0x1e:
        m_initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_SpectateMode, value);
        m_worldType.Update(m_gameOptionsList, value);
        m_initData->mSettingsChanged = true;
        return;
    case 0x20:
    {
        const int lootSet = m_gameOptionsList.GetSliderValue(0x20);
        m_initData->mPlayerSettings.mLootSetId = static_cast<unsigned int>(lootSet);
        if (m_initData->mMiniGameId == eMiniGameId_Battle)
            m_initData->mPlayerSettings.mRandomLootSet = lootSet == 5;
        const MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(
            m_initData->mMiniGameId, true);
        m_gameOptionsList.SetItemLabel(
            0x20, UIString(std::wstring(L10N::GetString(0x5ccdb447)) + L": " +
                           miniGame->GetLootSetText(m_initData->mPlayerSettings.mLootSetId)), false);
        UpdateDescriptionText();
        break;
    }
    case 0x21:
    {
        m_initData->mPlayerSettings.mFoodBalance = m_gameOptionsList.GetSliderValue(0x21);
        const MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(
            m_initData->mMiniGameId, true);
        m_gameOptionsList.SetItemLabel(
            0x21, UIString(std::wstring(L10N::GetString(0x1cbfab4a)) + L": " +
                           miniGame->GetFoodBalanceIndexText(m_initData->mPlayerSettings.mFoodBalance)), false);
        UpdateDescriptionText();
        break;
    }
    case 0x22:
    {
        const int lives = m_gameOptionsList.GetSliderValue(0x22);
        m_initData->mPlayerSettings.mLivesPerRound = static_cast<unsigned int>(lives);
        const std::wstring label = lives == 0
            ? makeTextLabel(IDS_MG_LIVES_PER_ROUND, IDS_MG_INFINITE)
            : makeValueLabel(IDS_MG_LIVES_PER_ROUND, lives);
        m_gameOptionsList.SetItemLabel(0x22, UIString(label), false);
        UpdateDescriptionText();
        m_initData->mSettingsChanged = true;
        return;
    }
    case 0x23:
        m_initData->mPlayerSettings.mRoundCount = m_gameOptionsList.GetSliderValue(0x23);
        m_gameOptionsList.SetItemLabel(
            0x23, UIString(makeValueLabel(0x9ad3440e, m_initData->mPlayerSettings.mRoundCount)), false);
        UpdateDescriptionText();
        m_initData->mSettingsChanged = true;
        return;
    case 0x24:
    {
        const int mapSize = m_gameOptionsList.GetSliderValue(0x24);
        if (mapSize == 0)
            m_initData->mPlayerSettings.mMapSizeOverride = maybe<int>();
        else
            m_initData->mPlayerSettings.mMapSizeOverride = maybe<int>(mapSize);
        const int overrideValue = m_initData->mPlayerSettings.mMapSizeOverride.isSet()
            ? m_initData->mPlayerSettings.mMapSizeOverride.get() : 0;
        const MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(
            m_initData->mMiniGameId, true);
        m_gameOptionsList.SetItemLabel(
            0x24, UIString(std::wstring(L10N::GetString(0xf8df77ce)) + L": " +
                           miniGame->GetMapSizeOverrideText(overrideValue)), false);
        UpdateDescriptionText();
        break;
    }
    case 0x2b:
    {
        const int arenaSize = m_gameOptionsList.GetSliderValue(0x2b);
        static const float arenaValues[] = { 0.0f, 0.5f, 0.625f, 0.75f, 0.875f, 1.0f };
        if (arenaSize == 0)
        {
            m_initData->mPlayerSettings.mArenaSizeMultiplier = maybe<float>();
            m_gameOptionsList.SetItemLabel(0x2b, UIString(makeTextLabel(0x36df2177, IDS_MG_AUTO)), false);
        }
        else
        {
            m_initData->mPlayerSettings.mArenaSizeMultiplier = maybe<float>(arenaValues[arenaSize]);
            m_gameOptionsList.SetItemLabel(0x2b, UIString(makeArenaSizeLabel(arenaSize)), false);
        }
        UpdateDescriptionText();
        m_initData->mSettingsChanged = true;
        return;
    }
    case 0x2c:
    {
        const int layerCount = m_gameOptionsList.GetSliderValue(0x2c);
        if (layerCount == 0)
        {
            m_initData->mPlayerSettings.mLayerCount = maybe<unsigned int>();
            m_gameOptionsList.SetItemLabel(0x2c, UIString(makeTextLabel(0x50849384, IDS_MG_AUTO)), false);
        }
        else
        {
            m_initData->mPlayerSettings.mLayerCount = maybe<unsigned int>(static_cast<unsigned int>(layerCount));
            m_gameOptionsList.SetItemLabel(0x2c, UIString(makeValueLabel(0x50849384, layerCount)), false);
        }
        UpdateDescriptionText();
        break;
    }
    case 0x2e:
        m_initData->mMatchmakingFilters[0] = value;
        m_lootSet.Update(m_gameOptionsList, value);
        UpdateDescriptionText();
        m_initData->mSettingsChanged = true;
        return;
    case 0x2f:
        m_initData->mMatchmakingFilters[1] = value;
        m_initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, value);
        m_foodBalance.Update(m_gameOptionsList, value);
        UpdateDescriptionText();
        m_initData->mSettingsChanged = true;
        return;
    default:
        break;
    }
    m_initData->mSettingsChanged = true;
}

void UIScene_LaunchMoreOptionsMenu::handleGainFocus(bool)
{
    UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleCheckboxElementToggled(int, int second, bool checked)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    switch (second)
    {
    case 0:
        m_initData->mOnlineGame = checked;
        m_updateOnline = true;
        m_initData->mSettingsChanged = true;
        break;
    case 1:
        m_initData->mSettingsChanged = true;
        m_initData->mInviteOnly = checked;
        break;
    case 2:
        m_initData->mSettingsChanged = true;
        m_initData->mAllowFriendsOfFriends = checked;
        break;
    case 3:
        m_initData->mSettingsChanged = true;
        m_initData->mPvP = checked;
        break;
    case 4:
        m_initData->mSettingsChanged = true;
        m_initData->mTrustPlayers = checked;
        break;
    case 5:
        m_initData->mSettingsChanged = true;
        m_initData->mFireSpreads = checked;
        break;
    case 6:
        m_initData->mSettingsChanged = true;
        m_initData->mTNT = checked;
        break;
    case 7:
        m_initData->mHostPrivileges = checked;
        m_updateCheats = true;
        m_initData->mSettingsChanged = true;
        break;
    case 8:
        m_initData->mSettingsChanged = true;
        m_initData->mResetNether = checked;
        break;
    case 9:
        m_initData->mSettingsChanged = true;
        m_initData->mResetEnd = checked;
        break;
    case 10:
        m_initData->mStructures = m_initData->mFlatWorld || checked;
        m_initData->mSettingsChanged = true;
        break;
    case 11:
        m_initData->mSettingsChanged = true;
        m_initData->mBonusChest = checked;
        break;
    case 12:
        m_initData->mSettingsChanged = true;
        m_initData->mKeepInventory = checked;
        break;
    case 13:
        m_initData->mSettingsChanged = true;
        m_initData->mDoMobSpawning = checked;
        break;
    case 14:
        m_initData->mSettingsChanged = true;
        m_initData->mDoMobLoot = checked;
        break;
    case 15:
        m_initData->mSettingsChanged = true;
        m_initData->mMobGriefing = checked;
        break;
    case 16:
        m_initData->mSettingsChanged = true;
        m_initData->mDoTileDrops = checked;
        break;
    case 17:
        m_initData->mSettingsChanged = true;
        m_initData->mNaturalRegeneration = checked;
        break;
    case 18:
        m_initData->mSettingsChanged = true;
        m_initData->mDoDaylightCycle = checked;
        break;
    case 19:
        m_initData->mSettingsChanged = true;
        m_initData->mDoWeatherCycle = checked;
        break;
    case 20:
        m_initData->mStructures = true;
        m_initData->mFlatWorld = checked;
        if (!m_initData->mWorldTypeLocked)
        {
            m_worldOptionsList.EnableItem(0x15, checked, false);
            m_worldOptionsList.EnableItem(10, !m_initData->mFlatWorld, false);
            if (!m_initData->mFlatWorld)
            {
                m_worldOptionsList.SetCheckboxValue(10, true, false);
                m_initData->mSettingsChanged = true;
                break;
            }
            SuperflatMenu::PresetCache::GetCachedPreset(m_playerId, m_initData->mSuperflatConfig);
        }
        m_initData->mSettingsChanged = true;
        break;
    case 23:
        m_initData->mSettingsChanged = true;
        m_initData->mFindBalancedSeed = checked;
        break;
    case 26:
        m_initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_CentreSpawn, checked);
        m_initData->mSettingsChanged = true;
        break;
    case 0x1f:
        if (MiniGameDef::IsCustomGame(m_initData->mMiniGameId, m_initData->mPlayerSettings))
            m_initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_NoSmallSkins, checked ? 0 : 1);
        m_initData->mSettingsChanged = true;
        break;
    case 0x28:
        if (MiniGameDef::IsCustomGame(m_initData->mMiniGameId, m_initData->mPlayerSettings))
            m_initData->mPlayerSettings.mTakeEverythingEnabled = checked;
        m_initData->mSettingsChanged = true;
        break;
    case 0x29:
        if (MiniGameDef::IsCustomGame(m_initData->mMiniGameId, m_initData->mPlayerSettings))
            m_initData->mPlayerSettings.mChestRefillEnabled = checked;
        m_initData->mSettingsChanged = true;
        break;
    default:
        m_initData->mSettingsChanged = true;
        break;
    }
    UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleTouchInput(unsigned int, int action, int, int, bool repeat,
                                                     bool, bool pressed)
{
    if (pressed && action > 0x33 && action < 0x36)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
        bool world = action == 0x34;
        if (m_menuType != static_cast<int>(world))
        {
            m_menuType = world;
            updateTooltips();
            static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->ChangeTab();
        }
        gConsoleUIController.TouchBoxRebuild(this);
    }
    (void)repeat;
}

UIControl* UIScene_LaunchMoreOptionsMenu::GetMainPanel()
{
    return m_menuType == 1 ? &m_worldBackground : &m_gameBackground;
}

int UIScene_LaunchMoreOptionsMenu::getSceneType()
{
    return EUIScene_LaunchMoreOptionsMenu;
}



