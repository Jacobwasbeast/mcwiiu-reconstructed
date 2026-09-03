#ifndef GAME_NETWORK_CLIENTBOUNDGAMEEVENTPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDGAMEEVENTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundGameEventPacket : public Packet, public boost::enable_shared_from_this<ClientboundGameEventPacket>
{
public:
    ClientboundGameEventPacket();
    ClientboundGameEventPacket(int event, float param);
    ClientboundGameEventPacket(int event, float param, int playerIndex);
    virtual ~ClientboundGameEventPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getEvent();
    float getParam();
    int getPlayerIndex();

    static boost::shared_ptr<Packet> create();

private:
    int mEvent;
    float mParam;
    int mPlayerIndex;
};

#endif
