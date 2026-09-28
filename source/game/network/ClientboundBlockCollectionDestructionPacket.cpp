#include "game/network/ClientboundBlockCollectionDestructionPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundBlockCollectionDestructionPacket::ClientboundBlockCollectionDestructionPacket()
    : sourceId(0), progress(0), positions()
{
}

ClientboundBlockCollectionDestructionPacket::~ClientboundBlockCollectionDestructionPacket()
{
    delete[] positions.data;
}

void ClientboundBlockCollectionDestructionPacket::read(DataInputStream* input)
{
    sourceId = input->readInt();
    progress = input->readUnsignedByte();
    const unsigned int count = input->readUnsignedShort();
    if (count == 0)
        return;
    positions = arrayWithLength<BlockPos>(new BlockPos[count], count);
    for (unsigned short i = 0; i < count; ++i)
        positions.data[i] = input->readBlockPos();
}

void ClientboundBlockCollectionDestructionPacket::write(DataOutputStream* output)
{
    output->writeInt(sourceId);
    output->writeByte(static_cast<unsigned char>(progress));
    output->writeShort(static_cast<short>(positions.length));
    for (unsigned short i = 0; i < positions.length; ++i)
        output->writeBlockPos(positions.data[i]);
}

void ClientboundBlockCollectionDestructionPacket::handle(PacketListener* listener)
{
    listener->handleBlockCollectionDestruction(shared_from_this());
}

int ClientboundBlockCollectionDestructionPacket::getEstimatedSize()
{
    return positions.length * 8 + 7;
}

int ClientboundBlockCollectionDestructionPacket::getPacketId()
{
    return 235;
}

boost::shared_ptr<Packet> ClientboundBlockCollectionDestructionPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundBlockCollectionDestructionPacket());
}
