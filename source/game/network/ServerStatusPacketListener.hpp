#ifndef GAME_NETWORK_SERVERSTATUSPACKETLISTENER_HPP
#define GAME_NETWORK_SERVERSTATUSPACKETLISTENER_HPP

#include "game/network/PacketListener.hpp"

class ServerStatusPacketListener : public PacketListener
{
public:
    ServerStatusPacketListener();
    virtual ~ServerStatusPacketListener();
};

#endif
