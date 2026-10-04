#include "game/ui/UIScene_InGameHostOptionsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/command/DefaultGameModeCommand.hpp"
#include "game/command/GameDifficultyCommand.hpp"
#include "game/command/SetDefaultSpawnpointCommand.hpp"
#include "game/command/TimeCommand.hpp"
#include "game/command/WeatherCommand.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
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
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_FIRE_SPREADS)), 0, gApp.GetGameHostOption(eGameHostOption_FireSpreads) != 0);
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_TNT_EXPLODES)), 1, gApp.GetGameHostOption(eGameHostOption_TNT) != 0);
    if (gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0)
    {
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_DAYLIGHT_CYCLE)), 2, gApp.GetGameHostOption(eGameHostOption_DoDaylightCycle) != 0);
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_KEEP_INVENTORY)), 3, gApp.GetGameHostOption(eGameHostOption_KeepInventory) != 0);
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_SPAWNING)), 4, gApp.GetGameHostOption(eGameHostOption_DoMobSpawning) != 0);
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_GRIEFING)), 5, gApp.GetGameHostOption(eGameHostOption_MobGriefing) != 0);
        m_hostOptions.AddNewButton(std::wstring(L10N::GetString(0xd554d957)), 0xc, true);
        m_hostOptions.AddNewButton(std::wstring(L10N::GetString(0xc0d12a27)), 0xd, true);

        unsigned int difficultyValue = gApp.GetGameHostOption(eGameHostOption_Difficulty);
        wchar_t difficultyLabel[256];
        swprintf(difficultyLabel, 256, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY),
                 L10N::GetString(Difficulty::byId(difficultyValue)->getKey()));
        m_hostOptions.AddNewSlider(std::wstring(difficultyLabel), 0xf, 0, 3, difficultyValue, false, 7, 3);

        unsigned int gameTypeValue = gApp.GetGameHostOption(eGameHostOption_GameType);
        const GameType* gameType = GameType::byId(gameTypeValue);
        m_hostOptions.AddNewSlider(std::wstring(L10N::GetString(gameType->getSliderLabel())), 0x10, 0,
                                   GameType::ADVENTURE->getId(), gameTypeValue, false, 7, 3);
        m_hostOptions.AddNewButton(std::wstring(L10N::GetString(IDS_SET_WORLD_SPAWN)), 0xe, true);

        Level* level = Minecraft::GetInstance()->getLevel(0);
        if (level != 0)
        {
            m_hostOptions.AddNewLabel(std::wstring(L" "), 0x12);
            m_hostOptions.AddNewLabel(std::wstring(L10N::GetString(IDS_WEATHER)), 0x12);
            int weatherState = Minecraft::GetInstance()->getLevel(0)->getWeatherState();
            m_hostOptions.AddNewSlider(std::wstring(L10N::GetString(getWeatherId(weatherState))), 0x11, 0, 2,
                                       weatherState, false, 7, 3);
        }
        m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_WEATHER_CYCLE)), 9, gApp.GetGameHostOption(eGameHostOption_DoWeatherCycle) != 0);
    }
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_LOOT)), 6, gApp.GetGameHostOption(eGameHostOption_DoMobLoot) != 0);
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(IDS_TILE_DROPS)), 7, gApp.GetGameHostOption(eGameHostOption_DoTileDrops) != 0);
    m_hostOptions.AddNewCheckbox(std::wstring(L10N::GetString(0x6f7d8591)), 8, gApp.GetGameHostOption(eGameHostOption_NaturalRegeneration) != 0);

    INetworkPlayer* player = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    if (player != 0)
    {
        unsigned int privileges = gApp.GetPlayerPrivileges(player->GetSmallId());
        if (gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0 &&
            Player::getPlayerGamePrivilege(privileges, Player::ePlayerGamePrivilege_CanTeleport) != 0 &&
            CGameNetworkManager::sInstance.GetPlayerCount() > 1)
        {
            m_hostOptions.AddNewButton(std::wstring(L10N::GetString(IDS_TELEPORT_TO_PLAYER)), 0xa, true);
            m_hostOptions.AddNewButton(std::wstring(L10N::GetString(IDS_TELEPORT_TO_ME)), 0xb, true);
        }
    }
    fuiRenderNode* root = m_movie->getRootNode();
    static_cast<FJ_InGameHostOptions*>(root->getFuiNode())->InitMenu();
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
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"InGameHostOptionsSplit") :
                                            std::wstring(L"InGameHostOptions");
}

void UIScene_InGameHostOptionsMenu::handleInput(int playerId, int action, bool repeat, bool pressed,
                                                bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, pressed, released);
    if (static_cast<unsigned int>(action) < EControllerActions_MenuUp)
        return;
    if (static_cast<unsigned int>(action) <= EControllerActions_MenuPageDown || action == EControllerActions_MenuOk)
    {
        sendInputToMovie(action, repeat, pressed, released);
    }
    else if (action == EControllerActions_MenuCancel && pressed)
    {
        unsigned int options = gApp.GetGameHostOption(eGameHostOption_All);
        bool settingsChanged = false;
        gApp.SetGameHostOption(options, eGameHostOption_FireSpreads, m_hostOptions.GetCheckboxValue(0));
        gApp.SetGameHostOption(options, eGameHostOption_TNT, m_hostOptions.GetCheckboxValue(1));
        if (gApp.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0)
        {
            gApp.SetGameHostOption(options, eGameHostOption_DoDaylightCycle, m_hostOptions.GetCheckboxValue(2));
            gApp.SetGameHostOption(options, eGameHostOption_KeepInventory, m_hostOptions.GetCheckboxValue(3));
            gApp.SetGameHostOption(options, eGameHostOption_DoMobSpawning, m_hostOptions.GetCheckboxValue(4));
            gApp.SetGameHostOption(options, eGameHostOption_MobGriefing, m_hostOptions.GetCheckboxValue(5));
            gApp.SetGameHostOption(options, eGameHostOption_DoWeatherCycle, m_hostOptions.GetCheckboxValue(9));
            int difficulty = static_cast<int>(m_hostOptions.GetSliderValue(0xf));
            if (gApp.GetGameHostOption(eGameHostOption_Difficulty) != difficulty)
            {
                ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
                if (connection != 0)
                {
                    settingsChanged = true;
                    connection->send(GameDifficultyCommand::preparePacket(difficulty));
                }
            }
            unsigned int gameType = m_hostOptions.GetSliderValue(0x10);
            if (gApp.GetGameHostOption(eGameHostOption_GameType) != gameType)
            {
                ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
                if (connection != 0)
                {
                    settingsChanged = true;
                    connection->send(DefaultGameModeCommand::preparePacket(static_cast<unsigned char>(gameType)));
                }
            }
            if (Minecraft::GetInstance()->getLevel(0) != 0)
            {
                unsigned int weather = m_hostOptions.GetSliderValue(0x11);
                if (static_cast<unsigned int>(Minecraft::GetInstance()->getLevel(0)->getWeatherState()) != weather)
                {
                    ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
                    if (connection != 0)
                        connection->send(WeatherCommand::preparePacket(static_cast<unsigned char>(weather)));
                }
            }
        }
        gApp.SetGameHostOption(options, eGameHostOption_DoMobLoot, m_hostOptions.GetCheckboxValue(6));
        gApp.SetGameHostOption(options, eGameHostOption_DoTileDrops, m_hostOptions.GetCheckboxValue(7));
        gApp.SetGameHostOption(options, eGameHostOption_NaturalRegeneration, m_hostOptions.GetCheckboxValue(8));
        if (settingsChanged || options != gApp.GetGameHostOption(eGameHostOption_All))
        {
            ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
            if (connection != 0)
                connection->send(boost::shared_ptr<Packet>(new ServerSettingsChangedPacket(1, options)));
        }
        navigateBack();
        handled = true;
    }
}

void UIScene_InGameHostOptionsMenu::handlePress(int, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (static_cast<unsigned int>(childId) < 10)
        return;
    if (static_cast<unsigned int>(childId) < 0xc)
    {
        struct SceneData { int player; bool value; char padding[3]; };
        SceneData* data = new SceneData();
        data->player = m_playerId;
        data->value = false;
        if (childId == 10)
            data->value = true;
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_TeleportMenu, data,
                                             EUILayer_Scene, EUIGroup_PlayerDefault);
        return;
    }
    if (childId == 0xc)
    {
        ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
        connection->send(TimeCommand::preparePacket(1000));
    }
    else if (static_cast<unsigned int>(childId) < 0xe)
    {
        ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
        connection->send(TimeCommand::preparePacket(14000));
    }
    else
    {
        if (childId != 0xe)
            return;
        ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
        connection->send(SetDefaultSpawnpointCommand::preparePacket());
    }
}

void UIScene_InGameHostOptionsMenu::handleCheckboxElementToggled(int, int, bool)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void UIScene_InGameHostOptionsMenu::handleSliderElementMove(int, int element, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    if (static_cast<unsigned int>(element) < 0xf)
        return;
    if (element == 0xf)
    {
        wchar_t label[256];
        swprintf(label, 256, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY),
                 L10N::GetString(Difficulty::byId(value)->getKey()));
        m_hostOptions.SetItemLabel(0xf, UIString(label), false);
    }
    else if (static_cast<unsigned int>(element) < 0x11)
    {
        const GameType* gameType = GameType::byId(value);
        m_hostOptions.SetItemLabel(element, UIString(L10N::GetString(gameType->getSliderLabel())), false);
    }
    else
    {
        if (element != 0x11)
            return;
        m_hostOptions.SetItemLabel(element, UIString(L10N::GetString(getWeatherId(value))), false);
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
