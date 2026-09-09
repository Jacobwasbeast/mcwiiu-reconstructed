#ifndef GAME_NETWORK_MAPSELECTINFOPACKET_HPP
#define GAME_NETWORK_MAPSELECTINFOPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class MapSelectInfoPacket : public Packet, public boost::enable_shared_from_this<MapSelectInfoPacket>
{
public:
    MapSelectInfoPacket();
    MapSelectInfoPacket(arrayWithLength<unsigned char> data, bool ownsData);
    virtual ~MapSelectInfoPacket();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual int getPacketId();
    arrayWithLength<unsigned char> getData() const
    {
        arrayWithLength<unsigned char> view;
        view.data = mData;
        view.length = mLength;
        return view;
    }
    static boost::shared_ptr<Packet> create();

private:
    friend class ClientPacketListener;
    unsigned char* mData;
    unsigned int mLength;
    bool mOwnsData;
    unsigned int mSize;
};

#endif
