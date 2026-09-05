#ifndef GAME_NETWORK_CLIENTBOUNDBLOCKUPDATEPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDBLOCKUPDATEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class Block;
class DataInputStream;
class DataOutputStream;
class Level;
class PacketListener;

class ClientboundBlockUpdatePacket : public Packet, public boost::enable_shared_from_this<ClientboundBlockUpdatePacket>
{
public:
    ClientboundBlockUpdatePacket();
    ClientboundBlockUpdatePacket(Level* level, const BlockPos& pos);
    virtual ~ClientboundBlockUpdatePacket();

    virtual unsigned int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual unsigned int getEstimatedSize();

    BlockPos getPos();
    Block* getBlock();
    Block* getExtraBlock();
    unsigned int getLevelIdx();

    static boost::shared_ptr<Packet> create();

private:
    BlockPos pos;
    Block* block;
    Block* extraBlock;
    unsigned int levelIdx;
};

#endif
