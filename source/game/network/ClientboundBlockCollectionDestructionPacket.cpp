#include "game/network/ClientboundBlockCollectionDestructionPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundBlockCollectionDestructionPacket::ClientboundBlockCollectionDestructionPacket()
    : sourceId(0), positions(), progress(0)
{
}

ClientboundBlockCollectionDestructionPacket::ClientboundBlockCollectionDestructionPacket(int sourceId, std::vector<BlockPos> positions, int progress)
    : sourceId(sourceId), positions(positions), progress(progress)
{
}

ClientboundBlockCollectionDestructionPacket::~ClientboundBlockCollectionDestructionPacket()
{
}

void ClientboundBlockCollectionDestructionPacket::read(DataInputStream* input)
{
    sourceId = input->readInt();
    progress = input->readByte();
    int count = input->readVarInt();
    positions.clear();
    positions.resize(count);
    for (int i = 0; i < count; ++i)
    {
        positions[i] = input->readBlockPos();
    }
}

void ClientboundBlockCollectionDestructionPacket::write(DataOutputStream* output)
{
    output->writeVarInt(sourceId);
    output->writeByte((unsigned char)progress);
    output->writeVarInt((int)positions.size());
    for (std::vector<BlockPos>::const_iterator it = positions.begin(); it != positions.end(); ++it)
    {
        output->writeBlockPos(*it);
    }
}

void ClientboundBlockCollectionDestructionPacket::handle(PacketListener* listener)
{
    listener->handleBlockCollectionDestruction(shared_from_this());
}

int ClientboundBlockCollectionDestructionPacket::getEstimatedSize()
{
    return (int)positions.size() * 8 + 7;
}

int ClientboundBlockCollectionDestructionPacket::getPacketId()
{
    return 235;
}

boost::shared_ptr<Packet> ClientboundBlockCollectionDestructionPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundBlockCollectionDestructionPacket());
}
