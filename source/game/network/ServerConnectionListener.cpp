#include "game/network/ServerConnectionListener.hpp"
#include "game/network/Connection.hpp"
#include "game/network/ClientboundLoginPacket.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/HumanoidArm.hpp"

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
}

void ServerConnectionListener::handlePreLogin(boost::shared_ptr<ServerboundPreLoginPacket>)
{
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
