#ifndef GAME_NETWORK_KICKPLAYERPACKET_HPP
#define GAME_NETWORK_KICKPLAYERPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class ServerGamePacketListenerImpl;

class KickPlayerPacket : public Packet, public boost::enable_shared_from_this<KickPlayerPacket>
{
public:
    KickPlayerPacket();
    KickPlayerPacket(unsigned char reason);
    virtual ~KickPlayerPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    static boost::shared_ptr<Packet> create();

private:
    friend class ServerGamePacketListenerImpl;

    unsigned char reason;
};

#endif
