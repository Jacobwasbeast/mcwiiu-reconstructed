#include "game/ui/UIScene_InGameHostOptionsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/command/DefaultGameModeCommand.hpp"
#include "game/command/GameDifficultyCommand.hpp"
#include "game/command/TimeCommand.hpp"
#include "game/command/WeatherCommand.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/network/ServerSettingsChangedPacket.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_InGameHostOptions.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/core/GameSettings.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

UIScene_InGameHostOptionsMenu::UIScene_InGameHostOptionsMenu(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer), m_hostOptions()
{
    initialiseMovie();
    Init();
}

unsigned int UIScene_InGameHostOptionsMenu::getWeatherId(int weatherState)
{
    if (weatherState == 1)
        return IDS_WEATHER_RAIN;
    if (weatherState == 2)
        return IDS_WEATHER_THUNDER;
    return IDS_WEATHER_CLEAR;
}

void UIScene_InGameHostOptionsMenu::Init()
{
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_FIRE_SPREADS)), 0, g_pApp->GetGameHostOption(eGameHostOption_FireSpreads) != 0);
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_TNT_EXPLODES)), 1, g_pApp->GetGameHostOption(eGameHostOption_TNT) != 0);
    if (g_pApp->GetGameHostOption(eGameHostOption_CheatsEnabled) != 0)
    {
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_DAYLIGHT_CYCLE)), 2, g_pApp->GetGameHostOption(eGameHostOption_DoDaylightCycle) != 0);
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_KEEP_INVENTORY)), 3, g_pApp->GetGameHostOption(eGameHostOption_KeepInventory) != 0);
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_SPAWNING)), 4, g_pApp->GetGameHostOption(eGameHostOption_DoMobSpawning) != 0);
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_GRIEFING)), 5, g_pApp->GetGameHostOption(eGameHostOption_MobGriefing) != 0);
        m_hostOptions.AddNewButton(std::wstring(L10N::GetString(0xd554d957)), 0xc, true);
        m_hostOptions.AddNewButton(std::wstring(L10N::GetString(0xc0d12a27)), 0xd, true);

        const Difficulty* difficulty = Difficulty::byId(g_pApp->GetGameHostOption(eGameHostOption_Difficulty));
        wchar_t difficultyLabel[256];
        swprintf(difficultyLabel, 256, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY),
                 L10N::GetString(difficulty->getKey()));
        m_hostOptions.AddNewSlider(std::wstring(difficultyLabel), 0xf, 0, 3,
                                   g_pApp->GetGameHostOption(eGameHostOption_Difficulty), false, 7, 3);

        const GameType* gameType = GameType::byId(g_pApp->GetGameHostOption(eGameHostOption_GameType));
        m_hostOptions.AddNewSlider(std::wstring(L10N::GetString(gameType->getSliderLabel())), 0x10, 0,
                                   GameType::ADVENTURE->getId(), g_pApp->GetGameHostOption(eGameHostOption_GameType),
                                   false, 7, 3);
        m_hostOptions.AddNewButton(std::wstring(L10N::GetString(IDS_SET_WORLD_SPAWN)), 0xe, true);

        Level* level = Minecraft::GetInstance()->getLevel(0);
        if (level != 0)
        {
            m_hostOptions.AddNewLabel(std::wstring(L" "), 0x12);
            m_hostOptions.AddNewLabel(std::wstring(L10N::GetString(IDS_WEATHER)), 0x12);
            m_hostOptions.AddNewSlider(std::wstring(L10N::GetString(getWeatherId(level->getWeatherState()))),
                                       0x11, 0, 2, level->getWeatherState(), false, 7, 3);
        }
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_WEATHER_CYCLE)), 9, g_pApp->GetGameHostOption(eGameHostOption_DoWeatherCycle) != 0);
    }
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_LOOT)), 6, g_pApp->GetGameHostOption(eGameHostOption_DoMobLoot) != 0);
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_TILE_DROPS)), 7, g_pApp->GetGameHostOption(eGameHostOption_DoTileDrops) != 0);
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(0x6f7d8591)), 8, g_pApp->GetGameHostOption(eGameHostOption_NaturalRegeneration) != 0);

    Level* level = Minecraft::GetInstance()->getLevel(0);
    bool hasMultiplePlayers = false;
    if (level != 0)
    {
        std::vector<boost::shared_ptr<Player> >* players = level->getPlayers(0, NULL);
        hasMultiplePlayers = players->size() > 1;
        delete players;
    }
    if (level != 0 && g_pApp->GetGameHostOption(eGameHostOption_CheatsEnabled) != 0 &&
        Player::getPlayerGamePrivilege(g_pApp->GetPlayerPrivileges(static_cast<unsigned char>(m_playerId)),
                                       Player::ePlayerGamePrivilege_CanTeleport) != 0 &&
        hasMultiplePlayers)
    {
        m_hostOptions.AddNewButton(std::wstring(L10N::GetString(IDS_TELEPORT_TO_PLAYER)), 0xa, true);
        m_hostOptions.AddNewButton(std::wstring(L10N::GetString(IDS_TELEPORT_TO_ME)), 0xb, true);
    }
    if (m_movie != 0)
    {
        fuiRenderNode* root = m_movie->getRootNode();
        if (root != 0 && root->getFuiNode() != 0)
            static_cast<FJ_InGameHostOptions*>(root->getFuiNode())->InitMenu();
    }
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_InGameHostOptionsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_InGameHostOptionsMenu::handleReload()
{
    Init();
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_InGameHostOptionsMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"InGameHostOptions") :
                                               std::wstring(L"InGameHostOptionsSplit");
}

void UIScene_InGameHostOptionsMenu::handleInput(int playerId, int action, bool repeat, bool pressed,
                                                bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        return;
    case EControllerActions_MenuCancel:
        break;
    default:
        return;
    }
    if (!pressed)
        return;

    unsigned int options = g_pApp->GetGameHostOption(eGameHostOption_All);
    g_pApp->SetGameHostOption(options, eGameHostOption_FireSpreads, m_hostOptions.GetCheckboxValue(0));
    g_pApp->SetGameHostOption(options, eGameHostOption_TNT, m_hostOptions.GetCheckboxValue(1));
    bool settingsChanged = false;
    if (g_pApp->GetGameHostOption(eGameHostOption_CheatsEnabled) != 0)
    {
        g_pApp->SetGameHostOption(options, eGameHostOption_DoDaylightCycle, m_hostOptions.GetCheckboxValue(2));
        g_pApp->SetGameHostOption(options, eGameHostOption_KeepInventory, m_hostOptions.GetCheckboxValue(3));
        g_pApp->SetGameHostOption(options, eGameHostOption_DoMobSpawning, m_hostOptions.GetCheckboxValue(4));
        g_pApp->SetGameHostOption(options, eGameHostOption_MobGriefing, m_hostOptions.GetCheckboxValue(5));
        g_pApp->SetGameHostOption(options, eGameHostOption_DoWeatherCycle, m_hostOptions.GetCheckboxValue(9));
        int difficulty = static_cast<int>(m_hostOptions.GetSliderValue(0xf));
        if (g_pApp->GetGameHostOption(eGameHostOption_Difficulty) != difficulty)
        {
            ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
            if (connection != 0)
            {
                connection->send(GameDifficultyCommand::preparePacket(difficulty));
                settingsChanged = true;
            }
        }
        unsigned int gameType = static_cast<unsigned int>(m_hostOptions.GetSliderValue(0x10));
        if (g_pApp->GetGameHostOption(eGameHostOption_GameType) != gameType)
        {
            ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
            if (connection != 0)
            {
                connection->send(DefaultGameModeCommand::preparePacket(static_cast<unsigned char>(gameType)));
                settingsChanged = true;
            }
        }
        Level* level = Minecraft::GetInstance()->getLevel(0);
        if (level != 0)
        {
            unsigned int weather = m_hostOptions.GetSliderValue(0x11);
            if (level->getWeatherState() != weather)
            {
                ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
                if (connection != 0)
                    connection->send(WeatherCommand::preparePacket(static_cast<unsigned char>(weather)));
            }
        }
    }
    g_pApp->SetGameHostOption(options, eGameHostOption_DoMobLoot, m_hostOptions.GetCheckboxValue(6));
    g_pApp->SetGameHostOption(options, eGameHostOption_DoTileDrops, m_hostOptions.GetCheckboxValue(7));
    g_pApp->SetGameHostOption(options, eGameHostOption_NaturalRegeneration, m_hostOptions.GetCheckboxValue(8));
    if (settingsChanged || options != g_pApp->GetGameHostOption(eGameHostOption_All))
    {
        ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
        if (connection != 0)
            connection->send(boost::shared_ptr<Packet>(new ServerSettingsChangedPacket(1, options)));
    }
    navigateBack();
    handled = true;
}

void UIScene_InGameHostOptionsMenu::handlePress(int, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (childId < 10 || childId > 0xe)
        return;
    if (childId < 0xc)
    {
        struct SceneData { int player; bool value; char padding[3]; };
        SceneData* data = new SceneData;
        data->player = m_playerId;
        data->value = childId == 10;
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_TeleportMenu, data,
                                             EUILayer_Scene, EUIGroup_PlayerDefault);
        return;
    }
    ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
    if (connection == 0)
        return;
    if (childId == 0xc)
        connection->send(TimeCommand::preparePacket(1000));
    else if (childId == 0xd)
        connection->send(TimeCommand::preparePacket(14000));
    else
        connection->send(boost::shared_ptr<Packet>(new GameCommandPacket(
            eGameCommand_SetDefaultSpawnpoint, arrayWithLength<unsigned char>())));
}

void UIScene_InGameHostOptionsMenu::handleCheckboxElementToggled(int, int, bool)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void UIScene_InGameHostOptionsMenu::handleSliderElementMove(int, int element, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    if (element == 0xf)
    {
        const Difficulty* difficulty = Difficulty::byId(value);
        wchar_t label[256];
        swprintf(label, 256, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY), L10N::GetString(difficulty->getKey()));
        m_hostOptions.SetItemLabel(0xf, UIString(label), false);
    }
    else if (element < 0x11 && element > 0xf)
    {
        const GameType* gameType = GameType::byId(value);
        m_hostOptions.SetItemLabel(element, UIString(L10N::GetString(gameType->getSliderLabel())), false);
    }
    else if (element == 0x11)
    {
        m_hostOptions.SetItemLabel(0x11, UIString(L10N::GetString(getWeatherId(value))), false);
    }
}

unsigned int UIScene_InGameHostOptionsMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_hostOptions.setupControl(this, root, std::string("HostOptionsList"));
    m_controls.push_back(&m_hostOptions);
    return 1;
}

int UIScene_InGameHostOptionsMenu::getSceneType()
{
    return EUIScene_InGameHostOptionsMenu;
}
