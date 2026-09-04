#include "game/network/ChunkVisibilityPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ChunkVisibilityPacket::ChunkVisibilityPacket()
{
    visible = false;
    x = 0;
    loaded = false;
    z = 0;
}

ChunkVisibilityPacket::ChunkVisibilityPacket(int x, int z, bool visible, bool loaded)
{
    this->visible = visible;
    this->x = x;
    this->loaded = loaded;
    this->z = z;
}

ChunkVisibilityPacket::~ChunkVisibilityPacket()
{
}

void ChunkVisibilityPacket::read(DataInputStream* input)
{
    int packed = input->readInt();
    x = (packed << 2) >> 17;
    z = (packed << 17) >> 17;
    visible = ((packed >> 30) & 1) != 0;
    loaded = ((packed >> 31) & 1) != 0;
}

void ChunkVisibilityPacket::write(DataOutputStream* output)
{
    unsigned int packed = (static_cast<unsigned int>(x) & 0x7fff) << 15;
    packed |= static_cast<unsigned int>(z) & 0x7fff;
    if (visible)
        packed |= 0x40000000u;
    if (loaded)
        packed |= 0x80000000u;
    output->writeInt(static_cast<int>(packed));
}

void ChunkVisibilityPacket::handle(PacketListener* listener)
{
    listener->handleChunkVisibility(shared_from_this());
}

int ChunkVisibilityPacket::getEstimatedSize()
{
    return 4;
}

int ChunkVisibilityPacket::getPacketId()
{
    return 50;
}

boost::shared_ptr<Packet> ChunkVisibilityPacket::create()
{
    return boost::shared_ptr<Packet>(new ChunkVisibilityPacket());
}
