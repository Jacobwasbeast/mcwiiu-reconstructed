#include "game/ui/UIScene_InGameSettingsListMenu.hpp"

#include <cwchar>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ClientboundMGPlayerSettingsUpdatePacket.hpp"
#include "game/network/Packet.hpp"
#include "game/network/ServerSettingsChangedPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_MultilistMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/PlayerList.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

template <typename T> std::wstring _toString(T value);

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

UIScene_InGameSettingsListMenu::UIScene_InGameSettingsListMenu(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer), m_roundLengthSlider(), m_maxPlayersSlider(),
      m_gameVariantSlider(), m_multiList(), m_description(), m_launchData(),
      m_playerSettings(), m_originalPlayerSettings(), m_mapSelection(0),
      m_miniGameId(eMiniGameId_Lobby), m_customGame(false), m_gameStarted(false),
      m_primaryPlayer(false), m_host(false)
{
    initialiseMovie();
    const MiniGameDef* firstMiniGame = Minecraft::GetInstance()->GetFirstMiniGame();
    m_miniGameId = static_cast<EMiniGameId>(firstMiniGame->GetId());
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    m_customGame = miniGame->GetId() == eMiniGameId_Lobby;
    m_launchData.mValue5C = m_miniGameId;
    m_launchData.mFlag58 = CGameNetworkManager::sInstance.IsInPublicJoinableGame();
    m_launchData.mFlag68 = miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0;
    m_launchData.mValue18 = sceneType;
    if (m_miniGameId == eMiniGameId_Battle)
        m_mapSelection = GameSettings::GetBattleMiniGameMapSelection(sceneType);
    else if (m_miniGameId == eMiniGameId_Glide)
        m_mapSelection = GameSettings::GetGlideMiniGameMapSelection(sceneType);
    Init();
}

UIScene_InGameSettingsListMenu::~UIScene_InGameSettingsListMenu()
{
}

void UIScene_InGameSettingsListMenu::EnableCustomOptions(bool enabled)
{
    if (m_miniGameId != eMiniGameId_Battle && m_miniGameId != eMiniGameId_Tumble)
        return;

    m_multiList.EnableItem(2, enabled, true);
    m_multiList.EnableItem(8, enabled, true);
    m_multiList.EnableItem(4, enabled, true);
    if (m_miniGameId == eMiniGameId_Battle)
    {
        unsigned int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        int mapSize = miniGame->GetMapSizeOverride();
        bool largeSession = (mapSize == 1 || mapSize == 2) && playerCount > 8;
        m_multiList.EnableItem(0, enabled && !largeSession, true);
        m_multiList.EnableItem(5, enabled, true);
        m_multiList.EnableItem(6, enabled, true);
        m_multiList.EnableItem(7, enabled, true);
        m_multiList.EnableItem(9, enabled, true);
        m_multiList.EnableItem(10, enabled, true);
        m_multiList.EnableItem(11, enabled, true);
        m_multiList.EnableItem(15, enabled, true);
        m_multiList.EnableItem(13, enabled, true);
        m_multiList.EnableItem(14, enabled, true);
        if (!largeSession)
            m_multiList.SetCheckboxValue(0, m_playerSettings.Get(eCustomGameModeHostOption_CentreSpawn) != 0, true);
        m_multiList.SetCheckboxValue(5, m_playerSettings.Get(eCustomGameModeHostOption_NoSmallSkins) == 0, true);
        m_multiList.SetCheckboxValue(11, m_playerSettings.mNaturalRegeneration, true);
        m_multiList.SetCheckboxValue(13, m_playerSettings.mSmallInventoryEnabled, true);
        m_multiList.SetCheckboxValue(14, m_playerSettings.mTakeEverythingEnabled, true);
        m_multiList.SetCheckboxValue(15, m_playerSettings.mChestRefillEnabled, true);
    }
    else
    {
        m_multiList.EnableItem(6, enabled, true);
        m_multiList.EnableItem(16, enabled, true);
        m_multiList.EnableItem(17, enabled, true);
        m_multiList.EnableItem(18, enabled, true);
        m_multiList.SetCheckboxValue(18, m_playerSettings.mSpectatorParticipation, true);
    }
}

void UIScene_InGameSettingsListMenu::doVerticalResizeCheck()
{
    FJ_MultilistMenu* menu = static_cast<FJ_MultilistMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->DoVerticalResizeCheck();
}

void UIScene_InGameSettingsListMenu::Init()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    m_gameStarted = minecraft->mLevel != 0;
    m_primaryPlayer = g_pApp->GetPrimaryUserIndex() == m_stageWidth;
    m_host = CGameNetworkManager::sInstance.IsHost();
    bool canEdit = m_host && m_primaryPlayer;

    MiniGameDef::GetPlayerSettings(m_playerSettings);
    MiniGameDef::GetPlayerSettings(m_originalPlayerSettings);
    const MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(m_miniGameId, true);

    if (miniGame->HasMultipleMaps() && canEdit && m_customGame && m_miniGameId == eMiniGameId_Battle)
    {
        m_multiList.AddNewButton(std::wstring(L10N::GetString(0x88c46bf1)), 1, true);
        m_multiList.EnableItem(1, true, true);
    }

    if (miniGame->HasRoundLengthOption())
    {
        m_roundLengthSlider = MultiListSliderLogic(2, 0x908811f4, eMiniGameId_Battle,
                                                   eCustomGameModeHostOption_RoundLength);
        m_roundLengthSlider.AddTo(m_multiList,
                                  m_playerSettings.Get(eCustomGameModeHostOption_RoundLength));
    }

    if (miniGame->HasCentralSpawnOption())
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0xef2a2f6a)), 0,
            m_playerSettings.Get(eCustomGameModeHostOption_CentreSpawn) != 0);

    std::wstring livesLabel = m_playerSettings.mLivesPerRound == 0
        ? makeTextLabel(0x8d5cbb71, 0x1d2ede0f)
        : makeValueLabel(0x8d5cbb71, m_playerSettings.mLivesPerRound);
    m_multiList.AddNewSlider(livesLabel, 8,
        miniGame->HasFeatureSupportForInfiniteLives() ? 0 : 1, 10,
        m_playerSettings.mLivesPerRound, false, 7, 3);

    m_gameVariantSlider = MultiListSliderLogic(4, 0x901c1fe8, m_miniGameId,
                                               eCustomGameModeHostOption_SpectateMode);
    m_gameVariantSlider.AddTo(m_multiList,
                              m_playerSettings.Get(eCustomGameModeHostOption_SpectateMode));

    if (m_miniGameId == eMiniGameId_Battle)
    {
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0xa329d6e9)), 5,
            m_playerSettings.Get(eCustomGameModeHostOption_NoSmallSkins) == 0);

        int lootSet = m_playerSettings.mLootSetId;
        if (m_customGame && m_playerSettings.mRandomLootSet)
            lootSet = 5;
        std::wstring lootLabel = std::wstring(L10N::GetString(0x5ccdb447)) + L": " +
                                 miniGame->GetLootSetText(lootSet);
        m_multiList.AddNewSlider(lootLabel, 6, 0, 5, lootSet, false, 7, 3);

        std::wstring foodLabel = std::wstring(L10N::GetString(0x1cbfab4a)) + L": " +
                                 miniGame->GetFoodBalanceIndexText(m_playerSettings.mFoodBalance);
        m_multiList.AddNewSlider(foodLabel, 7, 0, 7, m_playerSettings.mFoodBalance, false, 7, 3);
        m_multiList.AddNewSlider(makeValueLabel(0x9ad3440e, m_playerSettings.mRoundCount),
                                 9, 1, 10, m_playerSettings.mRoundCount, false, 7, 3);

        int mapSize = m_playerSettings.mMapSizeOverride.isSet()
            ? m_playerSettings.mMapSizeOverride.get() : 0;
        std::wstring mapLabel = std::wstring(L10N::GetString(0xf8df77ce)) + L": " +
                                miniGame->GetMapSizeOverrideText(mapSize);
        m_multiList.AddNewSlider(mapLabel, 10, 0, 2, mapSize, false, 7, 3);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0x1d7e9b6f)), 11,
                                   m_playerSettings.mNaturalRegeneration);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0xddc824da)), 13,
                                   m_playerSettings.mSmallInventoryEnabled);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0x7356945d)), 14,
                                   m_playerSettings.mTakeEverythingEnabled);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0xaa27b127)), 15,
                                   m_playerSettings.mChestRefillEnabled);
    }
    else if (m_miniGameId == eMiniGameId_Tumble)
    {
        std::wstring lootLabel = std::wstring(L10N::GetString(0x5ccdb447)) + L": " +
                                 miniGame->GetLootSetText(m_playerSettings.mLootSetId);
        m_multiList.AddNewSlider(lootLabel, 6, 0, 3, m_playerSettings.mLootSetId, false, 7, 3);

        static const float arenaSizes[] = { 0.0f, 0.5f, 0.625f, 0.75f, 0.875f, 1.0f };
        static const float arenaSizeLabels[] = { 1.0f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f };
        int arenaSize = 0;
        if (m_playerSettings.mArenaSizeMultiplier.isSet())
        {
            for (int index = 1; index <= 5; ++index)
                if (m_playerSettings.mArenaSizeMultiplier.get() == arenaSizes[index])
                    arenaSize = index;
        }
        std::wstring arenaLabel = arenaSize == 0
            ? makeTextLabel(0x36df2177, 0x74ae70e6)
            : std::wstring(L10N::GetString(0x36df2177)) + L" x " +
              _toString<float>(arenaSizeLabels[arenaSize]);
        m_multiList.AddNewSlider(arenaLabel, 16, 0, 5, arenaSize, true, 7, 3);

        int layerCount = m_playerSettings.mLayerCount.isSet() ? m_playerSettings.mLayerCount.get() : 0;
        std::wstring layerLabel = layerCount > 0
            ? makeValueLabel(0x50849384, layerCount)
            : makeTextLabel(0x50849384, 0x74ae70e6);
        m_multiList.AddNewSlider(layerLabel, 17, 0, 8, layerCount, false, 7, 3);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(0x40ea8efc)), 18,
                                   m_playerSettings.mSpectatorParticipation);
    }

    m_description.init(UIString(L""));
    EnableCustomOptions(canEdit && m_customGame);
    doVerticalResizeCheck();
    doHorizontalResizeCheck();
    g_pApp->GetLocalPlayerCount();
    gConsoleUIController.TouchBoxRebuild(this);
}

unsigned int UIScene_InGameSettingsListMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_multiList.setupControl(this, root, std::string("MultiList"));
    m_controls.push_back(&m_multiList);
    m_description.setupControl(this, root, std::string("Description"));
    m_controls.push_back(&m_description);
    return 1;
}

void UIScene_InGameSettingsListMenu::UpdateSettings(bool unavailable)
{
    int activeItem = m_multiList.GetActiveItem();
    m_multiList.clearList();
    Init();
    m_multiList.setCurrentSelection(activeItem, false, true);
    int parameters[1] = { static_cast<int>(0xb9a1bf4d) };
    if (unavailable)
    {
        gConsoleUIController.RequestAlertMessage(0x6abe4799, 0xd5232b99, parameters,
                                                  0, 0, 0, 0, 0, false);
        m_multiList.EnableItem(0, false, true);
        m_multiList.SetCheckboxValue(0, false, true);
    }
    else
    {
        gConsoleUIController.RequestAlertMessage(0x6abe4799, 0xba3da348, parameters,
                                                  1, m_stageWidth, 0, 0, 0, true);
    }
}

void UIScene_InGameSettingsListMenu::tick()
{
    UIScene::tick();
}

void UIScene_InGameSettingsListMenu::updateTooltips()
{
    int accept = m_host && m_primaryPlayer ? 0x8d6323ed : -1;
    gConsoleUIController.SetTooltips(m_stageWidth, accept, 0x4f6bcf5e, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_InGameSettingsListMenu::handleReload()
{
    Init();
}

void UIScene_InGameSettingsListMenu::updateComponents()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLevel == 0)
    {
        m_layer->showComponent(m_stageWidth, static_cast<EUIScene>(0x42), true);
        m_layer->showComponent(m_stageWidth, static_cast<EUIScene>(0x43), true);
    }
    else
    {
        m_layer->showComponent(m_stageWidth, static_cast<EUIScene>(0x42), false);
        m_layer->showComponent(m_stageWidth, static_cast<EUIScene>(0x43),
                               g_pApp->GetLocalPlayerCount() == 1);
    }
}

std::wstring UIScene_InGameSettingsListMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"MultilistMenu")
                                             : std::wstring(L"MultilistMenuSplit");
}

void UIScene_InGameSettingsListMenu::setGameSettings()
{
    if (!m_host || !m_primaryPlayer ||
        (m_miniGameId != eMiniGameId_Battle && m_miniGameId != eMiniGameId_Tumble))
        return;

    unsigned int hostOptions = g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x20));
    if (m_miniGameId == eMiniGameId_Battle)
    {
        m_playerSettings.Set(eCustomGameModeHostOption_CentreSpawn,
                             static_cast<unsigned int>(m_multiList.GetCheckboxValue(0)));
        m_playerSettings.Set(eCustomGameModeHostOption_NoSmallSkins,
                             static_cast<unsigned int>(m_multiList.GetCheckboxValue(5)) ^ 1);
        m_playerSettings.mNaturalRegeneration = m_multiList.GetCheckboxValue(11) != 0;
        m_playerSettings.mSmallInventoryEnabled = m_multiList.GetCheckboxValue(13) != 0;
        m_playerSettings.mTakeEverythingEnabled = m_multiList.GetCheckboxValue(14) != 0;
        m_playerSettings.mChestRefillEnabled = m_multiList.GetCheckboxValue(15) != 0;
        g_pApp->SetGameHostOption(hostOptions, static_cast<eGameHostOption>(0x1d),
                                  m_playerSettings.mNaturalRegeneration);
    }
    else
    {
        m_playerSettings.mSpectatorParticipation = m_multiList.GetCheckboxValue(18) != 0;
    }

    MiniGameDef::SetPlayerSettings(m_playerSettings);
    bool playerSettingsChanged = !(m_playerSettings == m_originalPlayerSettings);
    bool mapChanged = false;
    if (m_miniGameId == eMiniGameId_Battle)
        mapChanged = GameSettings::GetBattleMiniGameMapSelection(m_stageWidth) != m_mapSelection;
    else if (m_miniGameId == eMiniGameId_Glide)
        mapChanged = GameSettings::GetGlideMiniGameMapSelection(m_stageWidth) != m_mapSelection;
    else
        mapChanged = m_mapSelection != 0;

    if (mapChanged)
        gMinecraftServer->mPlayers->broadcastPlaylistInfo(&m_launchData.mLevelRulesetGroups);

    if (mapChanged || playerSettingsChanged)
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        gMinecraftServer->mPlayers->broadcastAll(miniGame->GetPlayerSettingsPacket());
    }

    if (hostOptions != g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x20)))
    {
        ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_stageWidth);
        if (connection != 0)
            connection->send(boost::shared_ptr<Packet>(new ServerSettingsChangedPacket(1, hostOptions)));
    }
}

void UIScene_InGameSettingsListMenu::UpdateDescriptionText()
{
    static const int descriptionIds[] = {
        0x4f8bedac, 0x75cf2428, 0x4c17ef92, 0x18a5cddd, static_cast<int>(0xfe516d5e),
        static_cast<int>(0xbe68971f), 0, 0, 0x42d4ae0f, 0x5e1bb988,
        0x2a5c55c8, static_cast<int>(0xb66f0561), static_cast<int>(0xf7c16b2c),
        static_cast<int>(0xe7ba74fc), static_cast<int>(0xe8f6de23),
        static_cast<int>(0xbfd9bf09), 0x0d7dae19, 0x35ef3122,
        static_cast<int>(0xa541f7ca)
    };

    int childFocus = static_cast<int>(m_childFocus);
    if (childFocus == -1)
        return;

    int descriptionId = descriptionIds[childFocus];
    std::wstring description;
    if (descriptionId != -1 && descriptionId != -2)
    {
        std::wstring source(L10N::GetString(descriptionId));
        description = g_pApp->FormatHTMLString(m_stageWidth, source, -1);
    }

    const MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(m_miniGameId, true);
    if (childFocus == 6)
        description = miniGame->GetLootSetDescription(m_multiList.GetSliderValue(6));
    else if (childFocus == 7)
        description = miniGame->GetFoodBalanceIndexDescription(m_multiList.GetSliderValue(7));

    wchar_t colourTag[64];
    std::swprintf(colourTag, 64, L"<font color=\"#%08x\">",
                  g_pApp->GetHTMLColour(static_cast<eMinecraftColour>(0x1b5)));
    description = std::wstring(colourTag) + description;
    m_description.setLabel(UIString(description), false, false);
}

void UIScene_InGameSettingsListMenu::handleInput(int playerId, int action, bool repeat,
                                                  bool released, bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
    if (action < 4 || (action > 9 && action < 14) ||
        (action > 15 && action != 20 && action != 21))
        return;
    if (action == 21 && released)
    {
        setGameSettings();
        navigateBack();
        handled = true;
        return;
    }
    sendInputToMovie(action, repeat, released, pressed);
}

void UIScene_InGameSettingsListMenu::handleFocusChange(int focus, int childFocus)
{
    m_focus = static_cast<double>(focus);
    m_childFocus = static_cast<double>(childFocus);
    UpdateDescriptionText();
}

void UIScene_InGameSettingsListMenu::handlePress(int, int second)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (second == 1)
        gConsoleUIController.NavigateToScene(m_stageWidth, static_cast<EUIScene>(0x3f),
                                              &m_launchData, static_cast<EUILayer>(6),
                                              static_cast<EUIGroup>(7));
}

void UIScene_InGameSettingsListMenu::handleCheckboxElementToggled(int, int, bool)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void UIScene_InGameSettingsListMenu::handleSliderElementMove(int, int element, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    const MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(m_miniGameId, true);
    switch (element)
    {
    case 2:
        m_playerSettings.Set(eCustomGameModeHostOption_RoundLength, value);
        m_roundLengthSlider.Update(m_multiList, value);
        return;
    case 3:
        m_playerSettings.Set(eCustomGameModeHostOption_GameVariant, value);
        m_maxPlayersSlider.Update(m_multiList, value);
        break;
    case 4:
        m_playerSettings.Set(eCustomGameModeHostOption_SpectateMode, value);
        m_gameVariantSlider.Update(m_multiList, value);
        return;
    case 6:
    {
        m_playerSettings.mLootSetId = static_cast<unsigned int>(m_multiList.GetSliderValue(6));
        if (m_miniGameId == eMiniGameId_Battle)
            m_playerSettings.mRandomLootSet = m_playerSettings.mLootSetId == 5;
        std::wstring label = std::wstring(L10N::GetString(0x5ccdb447)) + L": " +
                             miniGame->GetLootSetText(m_playerSettings.mLootSetId);
        m_multiList.SetItemLabel(6, UIString(label), false);
        UpdateDescriptionText();
        break;
    }
    case 7:
    {
        m_playerSettings.mFoodBalance = static_cast<int>(m_multiList.GetSliderValue(7));
        std::wstring label = std::wstring(L10N::GetString(0x1cbfab4a)) + L": " +
                             miniGame->GetFoodBalanceIndexText(m_playerSettings.mFoodBalance);
        m_multiList.SetItemLabel(7, UIString(label), false);
        UpdateDescriptionText();
        break;
    }
    case 8:
        m_playerSettings.mLivesPerRound = m_multiList.GetSliderValue(8);
        m_multiList.SetItemLabel(8, UIString(m_playerSettings.mLivesPerRound == 0
            ? makeTextLabel(0x8d5cbb71, 0x1d2ede0f)
            : makeValueLabel(0x8d5cbb71, m_playerSettings.mLivesPerRound)), false);
        UpdateDescriptionText();
        return;
    case 9:
        m_playerSettings.mRoundCount = static_cast<int>(m_multiList.GetSliderValue(9));
        m_multiList.SetItemLabel(9, UIString(makeValueLabel(0x9ad3440e,
                                                            m_playerSettings.mRoundCount)), false);
        UpdateDescriptionText();
        return;
    case 10:
    {
        int mapSize = static_cast<int>(m_multiList.GetSliderValue(10));
        m_playerSettings.mMapSizeOverride = mapSize == 0 ? maybe<int>() : maybe<int>(mapSize);
        std::wstring label = std::wstring(L10N::GetString(0xf8df77ce)) + L": " +
                             miniGame->GetMapSizeOverrideText(mapSize);
        m_multiList.SetItemLabel(10, UIString(label), false);
        UpdateDescriptionText();
        break;
    }
    case 16:
    {
        static const float arenaSizes[] = { 0.0f, 0.5f, 0.625f, 0.75f, 0.875f, 1.0f };
        static const float arenaSizeLabels[] = { 1.0f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f };
        int sizeIndex = static_cast<int>(m_multiList.GetSliderValue(16));
        m_playerSettings.mArenaSizeMultiplier = sizeIndex == 0
            ? maybe<float>() : maybe<float>(arenaSizes[sizeIndex]);
        std::wstring label = sizeIndex == 0
            ? makeTextLabel(0x36df2177, 0x74ae70e6)
            : std::wstring(L10N::GetString(0x36df2177)) + L" x " +
              _toString<float>(arenaSizeLabels[sizeIndex]);
        m_multiList.SetItemLabel(16, UIString(label), false);
        UpdateDescriptionText();
        return;
    }
    case 17:
    {
        int layerCount = static_cast<int>(m_multiList.GetSliderValue(17));
        m_playerSettings.mLayerCount = layerCount == 0
            ? maybe<unsigned int>() : maybe<unsigned int>(layerCount);
        std::wstring label = layerCount == 0
            ? makeTextLabel(0x50849384, 0x74ae70e6)
            : makeValueLabel(0x50849384, layerCount);
        m_multiList.SetItemLabel(17, UIString(label), false);
        UpdateDescriptionText();
        return;
    }
    default:
        break;
    }
}

int UIScene_InGameSettingsListMenu::getSceneType()
{
    return 0x14;
}
