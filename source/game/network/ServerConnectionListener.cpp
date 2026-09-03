#include "game/network/ServerConnectionListener.hpp"
#include "game/network/Connection.hpp"

ServerConnectionListener::ServerConnectionListener(MinecraftServer* server, Socket* socket, const std::wstring& name)
    : PacketListener(), m_connection(new Connection(socket, name, this)), m_server(server), m_name(name), m_tickCount(0), m_disconnected(false)
{
}

ServerConnectionListener::~ServerConnectionListener()
{
    delete m_connection;
    m_connection = 0;
}

void ServerConnectionListener::disconnect(DisconnectPacket::eDisconnectReason)
{
    if (m_connection != 0)
        m_connection->sendAndQuit();
    m_disconnected = true;
}

void ServerConnectionListener::tick()
{
    if (m_connection != 0)
        m_connection->tick();
    ++m_tickCount;
    if (m_tickCount == 600)
        disconnect(DisconnectPacket::eDisconnectReason(3));
}

void ServerConnectionListener::sendPreLoginResponse()
{
}

void ServerConnectionListener::handlePreLogin(boost::shared_ptr<ServerboundPreLoginPacket>)
{
    sendPreLoginResponse();
}

void ServerConnectionListener::handleLogin(boost::shared_ptr<ClientboundLoginPacket>)
{
}

void ServerConnectionListener::handleAcceptedLogin(boost::shared_ptr<ClientboundLoginPacket>)
{
    sendPreLoginResponse();
}

void ServerConnectionListener::onDisconnect(DisconnectPacket::eDisconnectReason, void*)
{
    m_disconnected = true;
}

void ServerConnectionListener::handleGetInfo(boost::shared_ptr<GetInfoPacket>)
{
}

void ServerConnectionListener::handleKeepAlive(boost::shared_ptr<ServerboundKeepAlivePacket>)
{
}

void ServerConnectionListener::onUnhandledPacket(boost::shared_ptr<Packet>)
{
    disconnect(DisconnectPacket::eDisconnectReason(3));
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
