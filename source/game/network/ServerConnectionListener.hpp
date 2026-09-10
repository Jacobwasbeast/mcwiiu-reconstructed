#ifndef GAME_NETWORK_SERVERCONNECTIONLISTENER_HPP
#define GAME_NETWORK_SERVERCONNECTIONLISTENER_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/network/PacketListener.hpp"

class Connection;
class MinecraftServer;
class Packet;
class Socket;

class ServerConnectionListener : public PacketListener
{
public:
    ServerConnectionListener(MinecraftServer* server, Socket* socket, const std::wstring& name);
    virtual ~ServerConnectionListener();

    void disconnect(DisconnectPacket::eDisconnectReason reason);
    void tick();
    void sendPreLoginResponse();
    void handlePreLogin(boost::shared_ptr<ServerboundPreLoginPacket> packet);
    void handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet);
    void handleAcceptedLogin(boost::shared_ptr<ClientboundLoginPacket> packet);
    virtual void onDisconnect(DisconnectPacket::eDisconnectReason reason, void* userData);
    virtual void handleGetInfo(boost::shared_ptr<GetInfoPacket> packet);
    virtual void handleKeepAlive(boost::shared_ptr<ServerboundKeepAlivePacket> packet);
    virtual void onUnhandledPacket(boost::shared_ptr<Packet> packet);
    void send(boost::shared_ptr<Packet> packet);
    virtual bool isServerPacketListener();
    virtual bool isDisconnected();
    std::wstring getName();

private:
    Connection* m_connection;
    MinecraftServer* m_server;
    std::wstring m_name;
    unsigned int m_tickCount;
    bool m_disconnected;
    int m_pendingPacketKind;
    boost::shared_ptr<Packet> m_pendingPacket;
};

#endif
