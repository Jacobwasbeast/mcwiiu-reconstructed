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
    positions = arrayWithLength<BlockPos>(count, true);
    for (unsigned short i = 0; i < count; ++i)
        positions.data[i] = input->readBlockPos();
}

void ClientboundBlockCollectionDestructionPacket::write(DataOutputStream* output)
{
    output->writeInt(sourceId);
    output->writeByte(static_cast<unsigned char>(progress));
    output->writeShort(static_cast<short>(positions.length));
    if (positions.length != 0)
    {
        unsigned short i = 0;
        do
        {
            output->writeBlockPos(positions.data[i]);
            ++i;
        } while (i < positions.length);
    }
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
