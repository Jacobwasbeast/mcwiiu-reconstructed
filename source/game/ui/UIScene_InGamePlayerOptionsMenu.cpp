#include "game/ui/UIScene_InGamePlayerOptionsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/command/GameModeCommand.hpp"
#include "game/command/SetSpawnpointCommand.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PlayerInfoPacket.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_InGamePlayerOptions.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/IUIScene_PlayerList.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/GameSettings.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

struct InGamePlayerOptionsData
{
    int playerId;
    unsigned char smallId;
    unsigned int privileges;
};

UIScene_InGamePlayerOptionsMenu::UIScene_InGamePlayerOptionsMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer)
{
    initialiseMovie();
    const InGamePlayerOptionsData* optionsData = static_cast<const InGamePlayerOptionsData*>(data);
    m_privileges = optionsData->privileges;
    m_cheatsEnabled = false;
    m_navigatingBack = false;
    m_smallId = optionsData->smallId;

    INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_smallId);
    if (player != NULL)
        m_gamertag.init(UIString(player->GetDisplayName()));
    unsigned int trustPlayers = g_pApp->GetGameHostOption(eGameHostOption_TrustPlayers);
    unsigned int cheatsEnabled = g_pApp->GetGameHostOption(eGameHostOption_CheatsEnabled);
    bool isHost = false;
    if (localPlayer != NULL && localPlayer == player)
        isHost = true;
    m_isHost = isHost;
    bool checked;

    if (isHost)
    {
        if (cheatsEnabled != 0)
        {
            bool canToggleInvisible = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleInvisible) != 0;
            checked = canToggleInvisible && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Invisible) != 0 && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Invulnerable) != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_INVISIBLE), ePlayerOption_Invisible, checked);
            m_playerOptions.EnableItem(ePlayerOption_Invisible, canToggleInvisible, true);
            if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CreativeMode) == 0)
            {
                bool canToggleFly = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleFly) != 0;
                bool canToggleExhaustion = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleExhaustion) != 0;
                checked = canToggleFly && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanFly) != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_FLY), ePlayerOption_Fly, checked);
                m_playerOptions.EnableItem(ePlayerOption_Fly, canToggleFly, true);
                checked = canToggleExhaustion && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_ExhaustionDisabled) != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_DISABLE_EXHAUSTION), ePlayerOption_DisableExhaustion, checked);
                m_playerOptions.EnableItem(ePlayerOption_DisableExhaustion, canToggleExhaustion, true);
            }
            unsigned int gameType = g_pApp->GetPlayerGameType(m_smallId);
            m_playerOptions.AddNewSlider(L10N::GetString(GameType::byId(gameType)->getSliderLabel()),
                                         ePlayerOption_GameMode, 0, GameType::ADVENTURE->getId(), gameType, false,
                                         7, 3);
            m_playerOptions.AddNewButton(L10N::GetString(IDS_SET_PLAYER_SPAWN), ePlayerOption_SetSpawn, true);
        }
    }
    else
    {
        if (trustPlayers == 0 && player != NULL && !player->IsHost())
        {
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotMine) == 0 && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotBuild) == 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_BUILD_AND_MINE), ePlayerOption_BuildAndMine, checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches) != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_USE_DOORS_AND_SWITCHES), ePlayerOption_UseDoorsAndSwitches, checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanUseContainers) != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_OPEN_CONTAINERS), ePlayerOption_OpenContainers, checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotAttackPlayers) == 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_ATTACK_PLAYERS), ePlayerOption_AttackPlayers, checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotAttackAnimals) == 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_ATTACK_ANIMALS), ePlayerOption_AttackAnimals, checked);
        }

        if (localPlayer->IsHost())
        {
            m_cheatsEnabled = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Op) != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_MODERATOR), ePlayerOption_Moderator, Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Op) != 0);
            if (cheatsEnabled != 0)
            {
                checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleInvisible) != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_INVISIBLE), ePlayerOption_Invisible, checked);
                m_playerOptions.EnableItem(ePlayerOption_Invisible, true, true);
                if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CreativeMode) == 0)
                {
                    checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleFly) != 0;
                    m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_FLY), ePlayerOption_Fly, checked);
                    m_playerOptions.EnableItem(ePlayerOption_Fly, true, true);
                    checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleExhaustion) != 0;
                    m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_DISABLE_EXHAUSTION), ePlayerOption_DisableExhaustion, checked);
                    m_playerOptions.EnableItem(ePlayerOption_DisableExhaustion, true, true);
                }
                checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanTeleport) != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_ENABLE_TELEPORT), ePlayerOption_Teleport, checked);
                unsigned int gameType = g_pApp->GetPlayerGameType(m_smallId);
                m_playerOptions.AddNewSlider(L10N::GetString(GameType::byId(gameType)->getSliderLabel()),
                                             ePlayerOption_GameMode, 0, GameType::ADVENTURE->getId(), gameType,
                                             false, 7, 3);
                m_playerOptions.AddNewButton(L10N::GetString(IDS_SET_PLAYER_SPAWN), ePlayerOption_SetSpawn, true);
            }
            resetCheatCheckboxes();
        }

        if (player != NULL && !player->IsLocal() &&
            !player->IsSameSystem(CGameNetworkManager::sInstance.GetHostPlayer()))
            m_playerOptions.AddNewButton(L10N::GetString(IDS_KICK_PLAYER), ePlayerOption_Kick, true);
    }

    static_cast<FJ_InGamePlayerOptions*>(m_movie->getRootNode()->getFuiNode())
        ->SetPlayerIcon(g_pApp->GetPlayerColour(m_smallId));
    resetCheatCheckboxes();
    addTimer(0, 100);
    CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(m_playerId, OnPlayerChanged, this);
    static_cast<FJ_InGamePlayerOptions*>(m_movie->getRootNode()->getFuiNode())->InitMenu();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_InGamePlayerOptionsMenu::~UIScene_InGamePlayerOptionsMenu()
{
}

void UIScene_InGamePlayerOptionsMenu::resetCheatCheckboxes()
{
    if (g_pApp->GetGameHostOption(eGameHostOption_CheatsEnabled) == 0 || m_isHost)
        return;
    bool cheats = m_playerOptions.IsCheckbox(ePlayerOption_Moderator) && m_playerOptions.GetCheckboxValue(ePlayerOption_Moderator) != 0;
    if (m_playerOptions.IsCheckbox(ePlayerOption_Invisible))
    {
        m_playerOptions.EnableItem(ePlayerOption_Invisible, cheats, true);
        m_playerOptions.SetCheckboxValue(ePlayerOption_Invisible, cheats && Player::getPlayerGamePrivilege(m_privileges,
            Player::ePlayerGamePrivilege_CanToggleInvisible) != 0, true);
    }
    if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CreativeMode) == 0)
    {
        m_playerOptions.EnableItem(ePlayerOption_Fly, cheats, true);
        m_playerOptions.SetCheckboxValue(ePlayerOption_Fly, cheats && Player::getPlayerGamePrivilege(m_privileges,
            Player::ePlayerGamePrivilege_CanToggleFly) != 0, true);
        m_playerOptions.EnableItem(ePlayerOption_DisableExhaustion, cheats, true);
        m_playerOptions.SetCheckboxValue(ePlayerOption_DisableExhaustion, cheats && Player::getPlayerGamePrivilege(m_privileges,
            Player::ePlayerGamePrivilege_CanToggleExhaustion) != 0, true);
    }
    m_playerOptions.EnableItem(ePlayerOption_Teleport, cheats, true);
    m_playerOptions.SetCheckboxValue(ePlayerOption_Teleport, cheats && Player::getPlayerGamePrivilege(m_privileges,
        Player::ePlayerGamePrivilege_CanTeleport) != 0, true);
}

void UIScene_InGamePlayerOptionsMenu::OnPlayerChanged(void* context, INetworkPlayer* player, bool added)
{
    UIScene_InGamePlayerOptionsMenu* scene = static_cast<UIScene_InGamePlayerOptionsMenu*>(context);
    UIScene* back = scene->getBackScene();
    if (back != 0)
        IUIScene_PlayerList::OnPlayerChanged(back, player, added);
    if (added && player != 0 && player->GetSmallId() == scene->m_smallId)
        scene->m_navigatingBack = true;
}

unsigned int UIScene_InGamePlayerOptionsMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_playerOptions.setupControl(this, root, std::string("PlayerOptionsList"));
    m_controls.push_back(&m_playerOptions);
    m_gamertag.setupControl(this, root, std::string("Gamertag"));
    m_controls.push_back(&m_gamertag);
    return 1;
}

void UIScene_InGamePlayerOptionsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_InGamePlayerOptionsMenu::handleReload()
{
    INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_smallId);
    unsigned int trustPlayers = g_pApp->GetGameHostOption(eGameHostOption_TrustPlayers);
    unsigned int cheatsEnabled = g_pApp->GetGameHostOption(eGameHostOption_CheatsEnabled);
    bool isHost = false;
    if (localPlayer != NULL && localPlayer == player)
        isHost = true;
    m_isHost = isHost;
    bool checked;

    if (isHost)
    {
        if (cheatsEnabled != 0)
        {
            bool canToggleInvisible = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleInvisible) != 0;
            checked = canToggleInvisible && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Invisible) != 0 && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Invulnerable) != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_INVISIBLE), ePlayerOption_Invisible, checked);
            m_playerOptions.EnableItem(ePlayerOption_Invisible, canToggleInvisible, true);
            if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CreativeMode) == 0)
            {
                bool canToggleFly = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleFly) != 0;
                bool canToggleExhaustion = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleExhaustion) != 0;
                checked = canToggleFly && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanFly) != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_FLY), ePlayerOption_Fly, checked);
                m_playerOptions.EnableItem(ePlayerOption_Fly, canToggleFly, true);
                checked = canToggleExhaustion && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_ExhaustionDisabled) != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_DISABLE_EXHAUSTION), ePlayerOption_DisableExhaustion, checked);
                m_playerOptions.EnableItem(ePlayerOption_DisableExhaustion, canToggleExhaustion, true);
            }
            unsigned int gameType = g_pApp->GetPlayerGameType(m_smallId);
            m_playerOptions.AddNewSlider(L10N::GetString(GameType::byId(gameType)->getSliderLabel()),
                                         ePlayerOption_GameMode, 0, GameType::ADVENTURE->getId(), gameType, false,
                                         7, 3);
            m_playerOptions.AddNewButton(L10N::GetString(IDS_SET_PLAYER_SPAWN), ePlayerOption_SetSpawn, true);
        }
    }
    else
    {
        if (trustPlayers == 0 && player != NULL && !player->IsHost())
        {
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotMine) == 0 && Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotBuild) == 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_BUILD_AND_MINE), ePlayerOption_BuildAndMine, checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches) != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_USE_DOORS_AND_SWITCHES), ePlayerOption_UseDoorsAndSwitches, checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanUseContainers) != 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_OPEN_CONTAINERS), ePlayerOption_OpenContainers, checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotAttackPlayers) == 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_ATTACK_PLAYERS), ePlayerOption_AttackPlayers, checked);
            checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotAttackAnimals) == 0;
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_ATTACK_ANIMALS), ePlayerOption_AttackAnimals, checked);
        }

        if (localPlayer->IsHost())
        {
            m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_MODERATOR), ePlayerOption_Moderator, Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Op) != 0);
            if (cheatsEnabled != 0)
            {
                checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleInvisible) != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_INVISIBLE), ePlayerOption_Invisible, checked);
                m_playerOptions.EnableItem(ePlayerOption_Invisible, true, true);
                if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CreativeMode) == 0)
                {
                    checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleFly) != 0;
                    m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_FLY), ePlayerOption_Fly, checked);
                    m_playerOptions.EnableItem(ePlayerOption_Fly, true, true);
                    checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleExhaustion) != 0;
                    m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_CAN_DISABLE_EXHAUSTION), ePlayerOption_DisableExhaustion, checked);
                    m_playerOptions.EnableItem(ePlayerOption_DisableExhaustion, true, true);
                }
                checked = Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanTeleport) != 0;
                m_playerOptions.AddNewCheckbox(L10N::GetString(IDS_ENABLE_TELEPORT), ePlayerOption_Teleport, checked);
                unsigned int gameType = g_pApp->GetPlayerGameType(m_smallId);
                m_playerOptions.AddNewSlider(L10N::GetString(GameType::byId(gameType)->getSliderLabel()),
                                             ePlayerOption_GameMode, 0, GameType::ADVENTURE->getId(), gameType,
                                             false, 7, 3);
                m_playerOptions.AddNewButton(L10N::GetString(IDS_SET_PLAYER_SPAWN), ePlayerOption_SetSpawn, true);
            }
            resetCheatCheckboxes();
        }

        if (player != NULL && !player->IsLocal() &&
            !player->IsSameSystem(CGameNetworkManager::sInstance.GetHostPlayer()))
            m_playerOptions.AddNewButton(L10N::GetString(IDS_KICK_PLAYER), ePlayerOption_Kick, true);
    }

    static_cast<FJ_InGamePlayerOptions*>(m_movie->getRootNode()->getFuiNode())
        ->SetPlayerIcon(g_pApp->GetPlayerColour(m_smallId));
    static_cast<FJ_InGamePlayerOptions*>(m_movie->getRootNode()->getFuiNode())->InitMenu();
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_InGamePlayerOptionsMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"InGamePlayerOptions") :
                                               std::wstring(L"InGamePlayerOptionsSplit");
}

void UIScene_InGamePlayerOptionsMenu::handleInput(int, int action, bool repeat, bool pressed, bool released,
                                                  bool& handled)
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

    INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_smallId);
    m_privileges = g_pApp->GetPlayerPrivileges(m_smallId);
    bool sentGameModeChange = false;
    const unsigned int hostOption = g_pApp->GetGameHostOption(eGameHostOption_TrustPlayers);
    const unsigned int multiplayer = g_pApp->GetGameHostOption(eGameHostOption_CheatsEnabled);
    const bool host = localPlayer != 0 && localPlayer == player;
    m_isHost = host;

    if (host)
    {
        if (multiplayer != 0)
        {
            if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleInvisible) != 0)
            {
                Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Invisible,
                                               m_playerOptions.GetCheckboxValue(ePlayerOption_Invisible));
                Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Invulnerable,
                                               m_playerOptions.GetCheckboxValue(ePlayerOption_Invisible));
            }
            if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CreativeMode) == 0)
            {
                if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleFly) != 0)
                {
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanFly,
                                                   m_playerOptions.GetCheckboxValue(ePlayerOption_Fly));
                }
                if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleExhaustion) != 0)
                {
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_ExhaustionDisabled,
                                                   m_playerOptions.GetCheckboxValue(ePlayerOption_DisableExhaustion));
                }
            }
            const unsigned int gameType = m_playerOptions.GetSliderValue(ePlayerOption_GameMode);
            if (g_pApp->GetPlayerGameType(m_smallId) != gameType)
            {
                ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
                if (connection != 0)
                {
                    sentGameModeChange = true;
                    connection->send(GameModeCommand::preparePacket(player->GetUID(),
                                                                     static_cast<unsigned char>(gameType)));
                }
            }
        }
    }
    else
    {
        if (hostOption == 0 && player != 0 && !player->IsHost())
        {
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotMine,
                                           m_playerOptions.GetCheckboxValue(ePlayerOption_BuildAndMine) ^ 1);
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotBuild,
                                           m_playerOptions.GetCheckboxValue(ePlayerOption_BuildAndMine) ^ 1);
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches,
                                           m_playerOptions.GetCheckboxValue(ePlayerOption_UseDoorsAndSwitches));
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanUseContainers,
                                           m_playerOptions.GetCheckboxValue(ePlayerOption_OpenContainers));
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotAttackPlayers,
                                           m_playerOptions.GetCheckboxValue(ePlayerOption_AttackPlayers) ^ 1);
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CannotAttackAnimals,
                                           m_playerOptions.GetCheckboxValue(ePlayerOption_AttackAnimals) ^ 1);
        }

        if (localPlayer != 0 && localPlayer->IsHost())
        {
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_Op,
                                           m_playerOptions.GetCheckboxValue(ePlayerOption_Moderator));
            if (multiplayer != 0)
            {
                Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleInvisible,
                                               m_playerOptions.GetCheckboxValue(ePlayerOption_Invisible));
                if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CreativeMode) == 0)
                {
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleFly,
                                                   m_playerOptions.GetCheckboxValue(ePlayerOption_Fly));
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanToggleExhaustion,
                                                   m_playerOptions.GetCheckboxValue(ePlayerOption_DisableExhaustion));
                }
                Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_CanTeleport,
                                               m_playerOptions.GetCheckboxValue(ePlayerOption_Teleport));

                const unsigned int gameType = m_playerOptions.GetSliderValue(ePlayerOption_GameMode);
                if (g_pApp->GetPlayerGameType(m_smallId) != gameType)
                {
                    ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
                    if (connection != 0)
                    {
                        sentGameModeChange = true;
                        connection->send(GameModeCommand::preparePacket(player->GetUID(),
                                                                         static_cast<unsigned char>(gameType)));
                    }
                }
            }
        }
    }

    if (sentGameModeChange || g_pApp->GetPlayerPrivileges(m_smallId) != m_privileges)
    {
        ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
        if (connection != 0)
        {
            unsigned char gameTypeByte = static_cast<unsigned char>(g_pApp->GetPlayerGameType(m_smallId));
            if (localPlayer != 0 && localPlayer->IsHost() && multiplayer != 0)
            {
                gameTypeByte = static_cast<unsigned char>(m_playerOptions.GetSliderValue(ePlayerOption_GameMode));
            }
            connection->send(boost::shared_ptr<Packet>(
                new PlayerInfoPacket(m_smallId, -1, m_privileges, gameTypeByte)));
        }
    }

    navigateBack();
    handled = true;
}

void UIScene_InGamePlayerOptionsMenu::handleDestroy()
{
    CGameNetworkManager::sInstance.UnRegisterPlayerChangedCallback(m_playerId, OnPlayerChanged, this);
}

void UIScene_InGamePlayerOptionsMenu::handlePress(int, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (childId == ePlayerOption_Kick)
    {
        unsigned char* data = new unsigned char( m_smallId );
        int parameters[2] = { (int)IDS_CONFIRM_OK, (int)IDS_CONFIRM_CANCEL };
        gConsoleUIController.RequestAlertMessage(IDS_UNLOCK_KICK_PLAYER_TITLE,
            Minecraft::InMiniGame(eMiniGameId_Any, true) ? IDS_UNLOCK_KICK_PLAYER_MINIGAMES : IDS_UNLOCK_KICK_PLAYER,
            parameters, 2, m_playerId,
            IUIScene_PlayerList::KickPlayerReturned,
            data, 0, true);
    }
    else if (childId == ePlayerOption_SetSpawn)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_smallId);
        ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_playerId);
        if (player != 0 && connection != 0)
            connection->send(SetSpawnpointCommand::preparePacket(player->GetUID()));
    }
}

void UIScene_InGamePlayerOptionsMenu::handleCheckboxToggled(int, bool)
{
}

void UIScene_InGamePlayerOptionsMenu::handleTimerComplete(int timerId)
{
    if (timerId == 0 && !m_isHost)
    {
        const bool cheats = m_playerOptions.IsCheckbox(ePlayerOption_Moderator) && m_playerOptions.GetCheckboxValue(ePlayerOption_Moderator) != 0;
        if (m_cheatsEnabled != cheats)
        {
            m_cheatsEnabled = cheats;
            resetCheatCheckboxes();
        }
    }
}

void UIScene_InGamePlayerOptionsMenu::tick()
{
    UIScene::tick();
    if (m_navigatingBack)
    {
        if (!gConsoleUIController.IsSceneInStack(0xff, EUIScene_MessageBox))
            m_navigatingBack = false;
        gConsoleUIController.NavigateBack(m_playerId, false, EUIScene_None,
                                          EUILayer_Count);
    }
}

void UIScene_InGamePlayerOptionsMenu::handleCheckboxElementToggled(int, int, bool)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void UIScene_InGamePlayerOptionsMenu::handleSliderElementMove(int, int element, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    if (element == ePlayerOption_GameMode)
    {
        const GameType* gameType = GameType::byId(value);
        m_playerOptions.SetItemLabel(element, UIString(L10N::GetString(gameType->getSliderLabel())), false);
    }
}

int UIScene_InGamePlayerOptionsMenu::getSceneType()
{
    return EUIScene_InGamePlayerOptionsMenu;
}
