#include "game/ui/UIScene_InGamePlayerOptionsMenu.hpp"

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

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

struct InGamePlayerOptionsData
{
    int playerId;
    unsigned char smallId;
    unsigned int privileges;
};

static void addPlayerCheckbox(UIControl_MultiList* list, int stringId, int id, bool checked)
{
    list->AddNewCheckbox(std::wstring(L10N::GetString(stringId)), id, checked);
}

static void addPlayerButton(UIControl_MultiList* list, int stringId, int id)
{
    list->AddNewButton(std::wstring(L10N::GetString(stringId)), id, true);
}

static void addPlayerOptions(UIScene_InGamePlayerOptionsMenu* scene, int playerIndex)
{
    INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(playerIndex);
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(scene->m_smallId);
    const unsigned int hostOption = g_pApp->GetGameHostOption(eGameHostOption_12);
    const unsigned int multiplayer = g_pApp->GetGameHostOption(eGameHostOption_15);
    const bool host = localPlayer != 0 && localPlayer == player;
    scene->m_isHost = host;

    if (host)
    {
        if (multiplayer != 0)
        {
            const bool canChange = Player::getPlayerGamePrivilege(scene->m_privileges,
                Player::ePlayerGamePrivilege_13) != 0;
            addPlayerCheckbox(&scene->m_playerOptions, 0x9e0a027d, 9,
                canChange && Player::getPlayerGamePrivilege(scene->m_privileges,
                Player::ePlayerGamePrivilege_7) != 0 &&
                Player::getPlayerGamePrivilege(scene->m_privileges,
                Player::ePlayerGamePrivilege_8) != 0);
            scene->m_playerOptions.EnableItem(9, canChange, true);
            if (Player::getPlayerGamePrivilege(scene->m_privileges,
                Player::ePlayerGamePrivilege_9) == 0)
            {
                addPlayerCheckbox(&scene->m_playerOptions, 0x65c86b6c, 7,
                    Player::getPlayerGamePrivilege(scene->m_privileges,
                    Player::ePlayerGamePrivilege_14) != 0 &&
                    Player::getPlayerGamePrivilege(scene->m_privileges,
                    Player::ePlayerGamePrivilege_5) != 0);
                scene->m_playerOptions.EnableItem(7,
                    Player::getPlayerGamePrivilege(scene->m_privileges,
                    Player::ePlayerGamePrivilege_14) != 0, true);
                addPlayerCheckbox(&scene->m_playerOptions, 0x6d72db6a, 8,
                    Player::getPlayerGamePrivilege(scene->m_privileges,
                    Player::ePlayerGamePrivilege_15) != 0 &&
                    Player::getPlayerGamePrivilege(scene->m_privileges,
                    Player::ePlayerGamePrivilege_6) != 0);
                scene->m_playerOptions.EnableItem(8,
                    Player::getPlayerGamePrivilege(scene->m_privileges,
                    Player::ePlayerGamePrivilege_15) != 0, true);
            }
            const GameType* gameType = GameType::byId(g_pApp->GetPlayerGameType(scene->m_smallId));
            scene->m_playerOptions.AddNewSlider(std::wstring(L10N::GetString(gameType->getSliderLabel())),
                0xb, 0, GameType::ADVENTURE->getId(), g_pApp->GetPlayerGameType(scene->m_smallId), false, 7, 3);
            addPlayerButton(&scene->m_playerOptions, 0x88f67c3e, 0xc);
        }
    }
    else if (hostOption == 0 && player != 0 && !player->IsHost())
    {
        addPlayerCheckbox(&scene->m_playerOptions, 0xc1f0645, 0,
            Player::getPlayerGamePrivilege(scene->m_privileges,
            Player::ePlayerGamePrivilege_0) == 0 &&
            Player::getPlayerGamePrivilege(scene->m_privileges,
            Player::ePlayerGamePrivilege_1) == 0);
        addPlayerCheckbox(&scene->m_playerOptions, 0xf7e7f9a1, 1,
            Player::getPlayerGamePrivilege(scene->m_privileges,
            Player::ePlayerGamePrivilege_11) != 0);
        addPlayerCheckbox(&scene->m_playerOptions, 0xda9c2af0, 2,
            Player::getPlayerGamePrivilege(scene->m_privileges,
            Player::ePlayerGamePrivilege_12) != 0);
        addPlayerCheckbox(&scene->m_playerOptions, 0x2a3b2570, 3,
            Player::getPlayerGamePrivilege(scene->m_privileges,
            Player::ePlayerGamePrivilege_3) == 0);
        addPlayerCheckbox(&scene->m_playerOptions, 0x2e30f7dd, 4,
            Player::getPlayerGamePrivilege(scene->m_privileges,
            Player::ePlayerGamePrivilege_10) == 0);
    }

    if (localPlayer != 0 && localPlayer->IsHost())
    {
        addPlayerCheckbox(&scene->m_playerOptions, 0xee5328fd, 5,
            Player::getPlayerGamePrivilege(scene->m_privileges,
            Player::ePlayerGamePrivilege_4) != 0);
        scene->m_cheatsEnabled = Player::getPlayerGamePrivilege(scene->m_privileges,
            Player::ePlayerGamePrivilege_4) != 0;
        if (multiplayer != 0)
        {
            addPlayerCheckbox(&scene->m_playerOptions, 0xbf2df1b6, 9,
                Player::getPlayerGamePrivilege(scene->m_privileges,
                Player::ePlayerGamePrivilege_13) != 0);
            scene->m_playerOptions.EnableItem(9, true, true);
            if (Player::getPlayerGamePrivilege(scene->m_privileges,
                Player::ePlayerGamePrivilege_9) == 0)
            {
                addPlayerCheckbox(&scene->m_playerOptions, 0x65c86b6c, 7,
                    Player::getPlayerGamePrivilege(scene->m_privileges,
                    Player::ePlayerGamePrivilege_14) != 0);
                scene->m_playerOptions.EnableItem(7, true, true);
                addPlayerCheckbox(&scene->m_playerOptions, 0x6d72db6a, 8,
                    Player::getPlayerGamePrivilege(scene->m_privileges,
                    Player::ePlayerGamePrivilege_15) != 0);
                scene->m_playerOptions.EnableItem(8, true, true);
            }
            addPlayerCheckbox(&scene->m_playerOptions, 0x77569663, 6,
                Player::getPlayerGamePrivilege(scene->m_privileges,
                Player::ePlayerGamePrivilege_16) != 0);
            const GameType* gameType = GameType::byId(g_pApp->GetPlayerGameType(scene->m_smallId));
            scene->m_playerOptions.AddNewSlider(std::wstring(L10N::GetString(gameType->getSliderLabel())),
                0xb, 0, GameType::ADVENTURE->getId(), g_pApp->GetPlayerGameType(scene->m_smallId), false, 7, 3);
            addPlayerButton(&scene->m_playerOptions, 0x88f67c3e, 0xc);
        }
    }

    if (!host && player != 0 && !player->IsLocal() &&
        !player->IsSameSystem(CGameNetworkManager::sInstance.GetHostPlayer()))
    {
        addPlayerButton(&scene->m_playerOptions, 0x4c8183da, 10);
    }

    scene->resetCheatCheckboxes();
}

UIScene_InGamePlayerOptionsMenu::UIScene_InGamePlayerOptionsMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), m_gamertag(), m_playerOptions()
{
    initialiseMovie();
    const InGamePlayerOptionsData* optionsData = static_cast<const InGamePlayerOptionsData*>(data);
    m_smallId = optionsData->smallId;
    m_privileges = optionsData->privileges;
    m_cheatsEnabled = false;
    m_navigatingBack = false;
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_smallId);
    if (player != 0)
        m_gamertag.init(UIString(player->GetDisplayName()));
    addPlayerOptions(this, m_playerId);

    FJ_InGamePlayerOptions* document = static_cast<FJ_InGamePlayerOptions*>(m_movie->getRootNode()->getFuiNode());
    document->SetPlayerIcon(g_pApp->GetPlayerColour(m_smallId));
    resetCheatCheckboxes();
    addTimer(0, 100);
    CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(m_playerId, OnPlayerChanged, this);
    document->InitMenu();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_InGamePlayerOptionsMenu::~UIScene_InGamePlayerOptionsMenu()
{
}

void UIScene_InGamePlayerOptionsMenu::resetCheatCheckboxes()
{
    if (g_pApp->GetGameHostOption(eGameHostOption_15) == 0 || m_isHost)
        return;
    bool cheats = m_playerOptions.IsCheckbox(5) && m_playerOptions.GetCheckboxValue(5) != 0;
    if (m_playerOptions.IsCheckbox(9))
    {
        m_playerOptions.EnableItem(9, cheats, true);
        m_playerOptions.SetCheckboxValue(9, cheats && Player::getPlayerGamePrivilege(m_privileges,
            Player::ePlayerGamePrivilege_13) != 0, true);
    }
    if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_9) == 0)
    {
        m_playerOptions.EnableItem(7, cheats, true);
        m_playerOptions.SetCheckboxValue(7, cheats && Player::getPlayerGamePrivilege(m_privileges,
            Player::ePlayerGamePrivilege_14) != 0, true);
        m_playerOptions.EnableItem(8, cheats, true);
        m_playerOptions.SetCheckboxValue(8, cheats && Player::getPlayerGamePrivilege(m_privileges,
            Player::ePlayerGamePrivilege_15) != 0, true);
    }
    m_playerOptions.EnableItem(6, cheats, true);
    m_playerOptions.SetCheckboxValue(6, cheats && Player::getPlayerGamePrivilege(m_privileges,
        Player::ePlayerGamePrivilege_16) != 0, true);
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
    gConsoleUIController.SetTooltips(m_playerId, 0x8d6323ed, 0x4f6bcf5e, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_InGamePlayerOptionsMenu::handleReload()
{
    m_playerOptions.clearList();
    addPlayerOptions(this, m_playerId);
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_InGamePlayerOptionsMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"InGamePlayerOptions") :
                                               std::wstring(L"InGamePlayerOptionsSplit");
}

void UIScene_InGamePlayerOptionsMenu::handleInput(int, int action, bool repeat, bool released, bool pressed,
                                                  bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (action <= 3)
        return;
    if (action < 10 || action == 0x14)
    {
        sendInputToMovie(action, repeat, released, pressed);
        return;
    }
    if (action != 0x15 || !pressed)
        return;

    INetworkPlayer* localPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(m_smallId);
    m_privileges = g_pApp->GetPlayerPrivileges(m_smallId);
    bool sentGameModeChange = false;
    const unsigned int hostOption = g_pApp->GetGameHostOption(eGameHostOption_12);
    const unsigned int multiplayer = g_pApp->GetGameHostOption(eGameHostOption_15);
    const bool host = localPlayer != 0 && localPlayer == player;
    m_isHost = host;

    if (host)
    {
        if (multiplayer != 0)
        {
            if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_13) != 0)
            {
                Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_7,
                                               m_playerOptions.GetCheckboxValue(9));
                Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_8,
                                               m_playerOptions.GetCheckboxValue(9));
            }
            if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_9) == 0)
            {
                if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_14) != 0)
                {
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_5,
                                                   m_playerOptions.GetCheckboxValue(7));
                }
                if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_15) != 0)
                {
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_6,
                                                   m_playerOptions.GetCheckboxValue(8));
                }
            }
            const unsigned int gameType = m_playerOptions.GetSliderValue(0xb);
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
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_0,
                                           m_playerOptions.GetCheckboxValue(0) ^ 1);
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_1,
                                           m_playerOptions.GetCheckboxValue(0) ^ 1);
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_11,
                                           m_playerOptions.GetCheckboxValue(1));
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_12,
                                           m_playerOptions.GetCheckboxValue(2));
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_3,
                                           m_playerOptions.GetCheckboxValue(3) ^ 1);
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_10,
                                           m_playerOptions.GetCheckboxValue(4) ^ 1);
        }

        if (localPlayer != 0 && localPlayer->IsHost())
        {
            Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_4,
                                           m_playerOptions.GetCheckboxValue(5));
            if (multiplayer != 0)
            {
                Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_13,
                                               m_playerOptions.GetCheckboxValue(9));
                if (Player::getPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_9) == 0)
                {
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_14,
                                                   m_playerOptions.GetCheckboxValue(7));
                    Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_15,
                                                   m_playerOptions.GetCheckboxValue(8));
                }
                Player::setPlayerGamePrivilege(m_privileges, Player::ePlayerGamePrivilege_16,
                                               m_playerOptions.GetCheckboxValue(6));

                const unsigned int gameType = m_playerOptions.GetSliderValue(0xb);
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
                gameTypeByte = static_cast<unsigned char>(m_playerOptions.GetSliderValue(0xb));
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

void UIScene_InGamePlayerOptionsMenu::handlePress(int, int second)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (second == 10)
    {
        unsigned char* data = new unsigned char( m_smallId );
        int parameters[2] = { (int)0xb9a1bf4d, (int)0x4fcbc7e7 };
        gConsoleUIController.RequestAlertMessage(0x628f8b32,
            Minecraft::InMiniGame(eMiniGameId_Any, true) ? 0x2cf43e54 : 0xb5f4b25,
            parameters, 2, m_playerId,
            (int (*)(void*, int, C4JStorage::EMessageResult))IUIScene_PlayerList::KickPlayerReturned,
            data, 0, true);
    }
    else if (second == 0xc)
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
        const bool cheats = m_playerOptions.IsCheckbox(5) && m_playerOptions.GetCheckboxValue(5) != 0;
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
                                          static_cast<EUILayer>(8));
    }
}

void UIScene_InGamePlayerOptionsMenu::handleCheckboxElementToggled(int, int, bool)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void UIScene_InGamePlayerOptionsMenu::handleSliderElementMove(int, int element, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (element == 0xb)
    {
        const GameType* gameType = GameType::byId(value);
        m_playerOptions.SetItemLabel(element, UIString(L10N::GetString(gameType->getSliderLabel())), false);
    }
}

int UIScene_InGamePlayerOptionsMenu::getSceneType()
{
    return EUIScene_InGamePlayerOptionsMenu;
}
