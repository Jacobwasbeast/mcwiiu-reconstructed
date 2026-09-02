#ifndef GAME_NETWORK_SERVERBOUNDCUSTOMPAYLOADPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDCUSTOMPAYLOADPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundCustomPayloadPacket : public Packet, public boost::enable_shared_from_this<ServerboundCustomPayloadPacket>
{
public:
    ServerboundCustomPayloadPacket();
    ServerboundCustomPayloadPacket(const std::wstring& identifier, arrayWithLength<unsigned char> data);
    virtual ~ServerboundCustomPayloadPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    std::wstring getIdentifier();
    arrayWithLength<unsigned char> getData();

    static boost::shared_ptr<Packet> create();

private:
    std::wstring identifier;
    arrayWithLength<unsigned char> data;
};

#endif
