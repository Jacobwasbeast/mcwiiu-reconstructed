#include "game/network/ServerConnectionListener.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/Connection.hpp"
#include "game/network/ClientboundLoginPacket.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientboundPreLoginPacket.hpp"
#include "game/network/ServerboundPreLoginPacket.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/LevelData.hpp"
#include <cstring>
#include <vector>

extern CMinecraftApp* g_pApp;

ServerConnectionListener::ServerConnectionListener(MinecraftServer* server, Socket* socket, const std::wstring& name)
    : PacketListener(), m_connection(new Connection(socket, name, this)), m_server(server), m_name(name), m_tickCount(0), m_disconnected(false),
      m_pendingPacketKind(0), m_pendingPacket()
{
}

ServerConnectionListener::~ServerConnectionListener()
{
    delete m_connection;
    m_connection = 0;
}

void ServerConnectionListener::disconnect(DisconnectPacket::eDisconnectReason reason)
{
    if (m_connection != 0)
    {
        m_connection->send(boost::shared_ptr<Packet>(new DisconnectPacket(reason)));
        m_connection->sendAndQuit();
    }
    m_disconnected = true;
}

void ServerConnectionListener::tick()
{
    if (m_pendingPacketKind != 0)
    {
        boost::shared_ptr<Packet> packet = m_pendingPacket;
        m_pendingPacketKind = 0;
        m_pendingPacket.reset();
        if (packet)
            send(packet);
    }

    unsigned int previousTickCount = m_tickCount;
    ++m_tickCount;
    if (previousTickCount != 600)
    {
        if (m_connection != 0)
            m_connection->tick();
        return;
    }
    disconnect(DisconnectPacket::eDisconnectReason(3));
}

void ServerConnectionListener::sendPreLoginResponse()
{
    if (m_connection == NULL || m_server == NULL || m_server->getPlayers() == NULL)
        return;

    std::vector<boost::shared_ptr<ServerPlayer> >* players = m_server->getPlayers()->getPlayers();
    if (players == NULL)
        return;

    std::vector<PlayerUID> onlinePlayers;
    unsigned char guestFlags = 0;
    unsigned int hostPlayerIndex = 0;
    unsigned int playerIndex = 0;
    const PlayerUID emptyUid(0);

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin();
         it != players->end(); ++it, ++playerIndex)
    {
        boost::shared_ptr<ServerPlayer> serverPlayer = *it;
        if (!serverPlayer || serverPlayer->mConnection == NULL)
            continue;

        ServerGamePacketListenerImpl* listener = serverPlayer->mConnection.get();
        INetworkPlayer* networkPlayer = listener->getNetworkPlayer();
        if (networkPlayer == NULL)
            continue;

        const PlayerUID playerUid = networkPlayer->GetUID();
        if (playerUid == emptyUid)
            continue;

        if (listener->isGuest())
            guestFlags = static_cast<unsigned char>(guestFlags | (1U << (playerIndex & 0x3fU)));

        onlinePlayers.push_back(playerUid);
        if (networkPlayer->IsLocal())
            hostPlayerIndex = playerIndex;
    }

    std::wstring worldName;
    ServerLevel* level = m_server->getLevel(0);
    if (level != NULL && level->getLevelData() != NULL)
        worldName = level->getLevelData()->getLevelName();

    char uniqueMapName[16];
    std::memset(uniqueMapName, 0, sizeof(uniqueMapName));

    PlayerUID* playerUidArray = NULL;
    if (!onlinePlayers.empty())
    {
        playerUidArray = new PlayerUID[onlinePlayers.size()];
        for (size_t index = 0; index < onlinePlayers.size(); ++index)
            playerUidArray[index] = onlinePlayers[index];
    }

    const unsigned int hostOption = g_pApp != NULL
        ? g_pApp->GetGameHostOption(eGameHostOption_32) : 0;
    unsigned int maxPlayers = CGameNetworkManager::sInstance.GetMaxPlayers();
    const MiniGameDef* miniGame = Minecraft::GetInstance() != NULL
        ? Minecraft::GetInstance()->GetMiniGame() : NULL;
    if (miniGame != NULL)
        maxPlayers = miniGame->GetHostOption(static_cast<eCustomGameModeHostOption>(10));

    const bool matchmakingEnabled = CGameNetworkManager::sInstance.IsMatchmakingEnabled();
    const EMiniGameId miniGameId = miniGame != NULL
        ? static_cast<EMiniGameId>(miniGame->GetId()) : eMiniGameId_Any;

    ClientboundPreLoginPacket* packet = new ClientboundPreLoginPacket(
        worldName,
        playerUidArray,
        static_cast<unsigned int>(onlinePlayers.size()),
        guestFlags,
        static_cast<unsigned int>(m_server->mLoginComparisonValue),
        uniqueMapName,
        hostOption,
        maxPlayers,
        static_cast<unsigned char>(hostPlayerIndex),
        static_cast<unsigned int>(m_server->getGameModeId()),
        matchmakingEnabled,
        miniGameId);
    send(boost::shared_ptr<Packet>(packet));
}

void ServerConnectionListener::handlePreLogin(boost::shared_ptr<ServerboundPreLoginPacket> packet)
{
    if (packet->protocolVersion != 0x881)
    {
        disconnect(static_cast<DisconnectPacket::eDisconnectReason>(
            packet->protocolVersion < 0x882 ? 0x0F : 0x0E));
        return;
    }

    m_name = packet->name;
    sendPreLoginResponse();
}

void ServerConnectionListener::handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet)
{
    if (packet->mEntityId != 0x13C)
    {
        disconnect(static_cast<DisconnectPacket::eDisconnectReason>(
            packet->mEntityId < 0x13D ? 0x0F : 0x0E));
        return;
    }

    if (m_server->getPlayers()->isXuidBanned(packet->mPlayerUID2))
    {
        disconnect(static_cast<DisconnectPacket::eDisconnectReason>(0x1A));
        return;
    }

    handleAcceptedLogin(packet);
}

void ServerConnectionListener::handleAcceptedLogin(boost::shared_ptr<ClientboundLoginPacket> packet)
{
    if (packet->mViewDistance != m_server->mLoginComparisonValue)
    {
        sendPreLoginResponse();
        return;
    }

    PlayerUID uid = packet->mPlayerUID;
    if (uid == PlayerUID(0))
        uid = packet->mPlayerUID2;

    boost::shared_ptr<ServerPlayer> player = m_server->getPlayers()->getPlayerForLogin(
        this, packet->mLevelName, uid, packet->mPlayerUID2);
    if (!player)
        return;

    player->setMainArm(packet->mPlayerFlags == 0 ? HumanoidArm::RIGHT : HumanoidArm::LEFT);
    m_server->getPlayers()->placeNewPlayer(m_connection, player, packet);
}

void ServerConnectionListener::onDisconnect(DisconnectPacket::eDisconnectReason, void*)
{
    m_disconnected = true;
}

void ServerConnectionListener::handleGetInfo(boost::shared_ptr<GetInfoPacket>)
{
    boost::shared_ptr<Packet> packet(new DisconnectPacket(
        static_cast<DisconnectPacket::eDisconnectReason>(0x0D)));
    if (m_connection != 0)
        m_connection->send(packet);
    if (m_connection != 0)
        m_connection->sendAndQuit();
    m_disconnected = true;
}

void ServerConnectionListener::handleKeepAlive(boost::shared_ptr<ServerboundKeepAlivePacket>)
{
}

void ServerConnectionListener::onUnhandledPacket(boost::shared_ptr<Packet>)
{
    disconnect(DisconnectPacket::eDisconnectReason(0x10));
}

void ServerConnectionListener::send(boost::shared_ptr<Packet> packet)
{
    if (m_connection != 0 && packet)
        m_connection->send(packet);
}

bool ServerConnectionListener::isServerPacketListener()
{
    return true;
}

bool ServerConnectionListener::isDisconnected()
{
    return m_disconnected;
}

std::wstring ServerConnectionListener::getName()
{
    return m_name;
}
