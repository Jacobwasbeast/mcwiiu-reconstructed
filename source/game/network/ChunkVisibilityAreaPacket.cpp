#include "game/network/ChunkVisibilityAreaPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ChunkVisibilityAreaPacket::ChunkVisibilityAreaPacket()
    : minX(0), minZ(0), maxX(0), maxZ(0), visible(true), loaded(false), outside(false)
{
}

ChunkVisibilityAreaPacket::ChunkVisibilityAreaPacket(int minX, int minZ, int maxX, int maxZ,
                                                     bool visible, bool loaded, bool outside)
    : minX(minX), minZ(minZ), maxX(maxX), maxZ(maxZ), visible(visible), loaded(loaded), outside(outside)
{
}

ChunkVisibilityAreaPacket::~ChunkVisibilityAreaPacket()
{
}

void ChunkVisibilityAreaPacket::read(DataInputStream* input)
{
    unsigned int first = static_cast<unsigned int>(input->readInt());
    unsigned int second = static_cast<unsigned int>(input->readInt());
    minX = static_cast<int>((first >> 15) & 0x7fff);
    maxX = static_cast<int>(first & 0x7fff);
    visible = (first & 0x40000000u) != 0;
    loaded = (first & 0x80000000u) != 0;
    minZ = static_cast<int>((second >> 15) & 0x7fff);
    maxZ = static_cast<int>(second & 0x7fff);
    outside = (second & 0x40000000u) != 0;
}

void ChunkVisibilityAreaPacket::write(DataOutputStream* output)
{
    unsigned int first = (static_cast<unsigned int>(minX) & 0x7fff) << 15;
    first |= static_cast<unsigned int>(maxX) & 0x7fff;
    if (visible)
        first |= 0x40000000u;
    if (loaded)
        first |= 0x80000000u;
    unsigned int second = (static_cast<unsigned int>(minZ) & 0x7fff) << 15;
    second |= static_cast<unsigned int>(maxZ) & 0x7fff;
    if (outside)
        second |= 0x40000000u;
    output->writeInt(static_cast<int>(first));
    output->writeInt(static_cast<int>(second));
}

void ChunkVisibilityAreaPacket::handle(PacketListener* listener)
{
    listener->handleChunkVisibilityArea(shared_from_this());
}

int ChunkVisibilityAreaPacket::getEstimatedSize()
{
    return 8;
}

int ChunkVisibilityAreaPacket::getPacketId()
{
    return 155;
}

boost::shared_ptr<Packet> ChunkVisibilityAreaPacket::create()
{
    return boost::shared_ptr<Packet>(new ChunkVisibilityAreaPacket());
}
