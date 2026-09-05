#include "game/network/ClientboundBlockUpdatePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"

ClientboundBlockUpdatePacket::ClientboundBlockUpdatePacket()
{
    compressed = true;
    extraBlock = NULL;
}

ClientboundBlockUpdatePacket::ClientboundBlockUpdatePacket(Level* level, const BlockPos& pos)
{
    compressed = true;
    this->pos = pos;
    block = level->getBlock(pos);
    extraBlock = level->getExtraBlock(pos);
    levelIdx = 0;
}

ClientboundBlockUpdatePacket::~ClientboundBlockUpdatePacket()
{
}

void ClientboundBlockUpdatePacket::read(DataInputStream* input)
{
    unsigned int packedPos = input->readInt();
    pos = BlockPos(static_cast<int>(packedPos) >> 22,
                   (packedPos >> 14) & 0xFF,
                   static_cast<int>((packedPos << 18) | (packedPos >> 14)) >> 22);
    block = Block::byId(input->readShort() & 0xFFF);
    extraBlock = Block::byId(input->readShort() & 0xFFF);
    levelIdx = packedPos & 0xF;
}

void ClientboundBlockUpdatePacket::write(DataOutputStream* output)
{
    unsigned int packedPos = (levelIdx & 0xF)
        | ((static_cast<unsigned int>(pos.z) & 0x3FF) << 4)
        | ((static_cast<unsigned int>(pos.y) & 0xFF) << 14)
        | (static_cast<unsigned int>(pos.x) << 22);
    output->writeInt(packedPos);
    output->writeShort(static_cast<short>(Block::getId(block)));
    output->writeShort(static_cast<short>(Block::getId(extraBlock)));
}

void ClientboundBlockUpdatePacket::handle(PacketListener* listener)
{
    listener->handleBlockUpdate(shared_from_this());
}

unsigned int ClientboundBlockUpdatePacket::getEstimatedSize()
{
    return 8;
}

BlockPos ClientboundBlockUpdatePacket::getPos()
{
    return pos;
}

Block* ClientboundBlockUpdatePacket::getBlock()
{
    return block;
}

Block* ClientboundBlockUpdatePacket::getExtraBlock()
{
    return extraBlock;
}

unsigned int ClientboundBlockUpdatePacket::getLevelIdx()
{
    return levelIdx;
}

unsigned int ClientboundBlockUpdatePacket::getPacketId()
{
    return 53;
}

boost::shared_ptr<Packet> ClientboundBlockUpdatePacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundBlockUpdatePacket());
}
