#include "game/network/ClientboundChunkBlocksUpdatePacket.hpp"

#include <cstdlib>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/LevelChunk.hpp"

ClientboundChunkBlocksUpdatePacket::ClientboundChunkBlocksUpdatePacket()
{
    shouldDelay = true;
    x = 0;
    z = 0;
    count = 0;
    dimension = 0;
}

ClientboundChunkBlocksUpdatePacket::ClientboundChunkBlocksUpdatePacket(
    int x, int z, arrayWithLength<short> changes, unsigned char count, Level* level)
{
    shouldDelay = true;
    this->x = x;
    this->z = z;
    this->count = count;
    positions = arrayWithLength<short>(count, true);
    elements = arrayWithLength<unsigned int>(count, true);

    for (unsigned int i = 0; i < count; ++i)
    {
        positions.data[i] = changes.data[i];
    }

    qsort(positions.data, count, sizeof(short), PositionSortFunction);

    LevelChunk* chunk = level->getChunk(x, z);
    for (unsigned int i = 0; i < count; ++i)
    {
        unsigned short packedPosition = static_cast<unsigned short>(positions.data[i]);
        int localX = (packedPosition >> 12) & 15;
        int localZ = (packedPosition >> 8) & 15;
        int y = packedPosition & 255;
        elements.data[i] = chunk->getElement(localX, y, localZ)
            | (static_cast<unsigned int>(chunk->getExtraElement(localX, y, localZ)) << 16);
    }

    const DimensionType* type = level->mDimension->getType();
    if (type == DimensionType::OVERWORLD)
    {
        dimension = 0;
    }
    else if (type == DimensionType::NETHER)
    {
        dimension = 1;
    }
    else
    {
        dimension = 2;
    }
}

ClientboundChunkBlocksUpdatePacket::~ClientboundChunkBlocksUpdatePacket()
{
    delete[] elements.data;
    delete[] positions.data;
}

int ClientboundChunkBlocksUpdatePacket::PositionSortFunction(const void* first, const void* second)
{
    const unsigned char* firstPosition = static_cast<const unsigned char*>(first);
    const unsigned char* secondPosition = static_cast<const unsigned char*>(second);
    return firstPosition[1] - secondPosition[1];
}

void ClientboundChunkBlocksUpdatePacket::read(DataInputStream* input)
{
    unsigned int header = input->readInt();
    x = static_cast<int>(header << 12) >> 22;
    z = static_cast<int>(header << 22) >> 22;
    count = (header >> 20) & 255;
    dimension = static_cast<int>(header) >> 28;

    positions = arrayWithLength<short>(count, true);
    elements = arrayWithLength<unsigned int>(count, true);

    unsigned int previousElement = 0;
    unsigned short previousPosition = 0;

    for (unsigned int i = 0; i < count; ++i)
    {
        unsigned int value = input->readUnsignedByte();
        bool newY = (value & 128) != 0;

        if ((value & 64) != 0)
        {
            unsigned int shift = 6;
            previousElement = value & 63;
            unsigned int part;
            do
            {
                part = input->readUnsignedByte();
                previousElement |= (part & 127) << shift;
                shift += 7;
            }
            while ((part & 128) != 0);
        }
        else if ((value & 32) != 0)
        {
            previousElement = value & 15;
        }
        else
        {
            previousElement = (previousElement & 0xfffffff0) | (value & 15);
        }

        elements.data[i] = previousElement;

        unsigned int xz = input->readUnsignedByte();
        if (newY)
        {
            previousPosition = static_cast<unsigned short>((xz << 8) | input->readUnsignedByte());
        }
        else
        {
            previousPosition = static_cast<unsigned short>((xz << 8) | (previousPosition & 255));
        }
        positions.data[i] = static_cast<short>(previousPosition);
    }
}

void ClientboundChunkBlocksUpdatePacket::write(DataOutputStream* output)
{
    unsigned int header = (static_cast<unsigned int>(dimension) << 28)
        | (count << 20)
        | ((static_cast<unsigned int>(x) & 1023) << 10)
        | (static_cast<unsigned int>(z) & 1023);
    output->writeInt(header);

    unsigned int previousY = static_cast<unsigned int>(-1);
    unsigned int previousElement = static_cast<unsigned int>(-1);

    for (unsigned int i = 0; i < count; ++i)
    {
        unsigned short packedPosition = static_cast<unsigned short>(positions.data[i]);
        unsigned int y = packedPosition & 255;
        unsigned int element = elements.data[i];
        unsigned int value = y == previousY ? 0 : 128;

        if (element == 0)
        {
            value |= 32;
            output->writeByte(static_cast<unsigned char>(value | (element & 15)));
            output->writeByte(static_cast<unsigned char>(packedPosition >> 8));
        }
        else if (((element ^ previousElement) & 0xfffffff0) == 0)
        {
            output->writeByte(static_cast<unsigned char>(value | (element & 15)));
            output->writeByte(static_cast<unsigned char>(packedPosition >> 8));
        }
        else
        {
            output->writeByte(static_cast<unsigned char>(value | 64 | (element & 63)));
            unsigned int remaining = element >> 6;
            do
            {
                unsigned int part = remaining & 127;
                remaining >>= 7;
                if (remaining != 0)
                {
                    part |= 128;
                }
                output->writeByte(static_cast<unsigned char>(part));
            }
            while (remaining != 0);
            output->writeByte(static_cast<unsigned char>(packedPosition >> 8));
        }

        if (y != previousY)
        {
            output->writeByte(static_cast<unsigned char>(y));
        }

        previousY = y;
        previousElement = element;
    }
}

void ClientboundChunkBlocksUpdatePacket::handle(PacketListener* listener)
{
    listener->handleChunkBlocksUpdate(shared_from_this());
}

int ClientboundChunkBlocksUpdatePacket::getEstimatedSize()
{
    return count * 3 + 4;
}

int ClientboundChunkBlocksUpdatePacket::getPacketId()
{
    return 52;
}

boost::shared_ptr<Packet> ClientboundChunkBlocksUpdatePacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundChunkBlocksUpdatePacket());
}
