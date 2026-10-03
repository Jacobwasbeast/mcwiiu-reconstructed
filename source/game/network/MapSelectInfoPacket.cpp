#include "game/network/MapSelectInfoPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

MapSelectInfoPacket::MapSelectInfoPacket()
    : mOwnsData(false), mSize(0), mData(NULL), mLength(0)
{
}

MapSelectInfoPacket::MapSelectInfoPacket(arrayWithLength<unsigned char> data, bool ownsData)
    : mData(NULL), mLength(0)
{
    mOwnsData = ownsData;
    mData = data.data;
    mLength = data.length;
    if (mData == NULL)
    {
        mSize = 0;
    }
    else
    {
        mSize = data.length;
    }
}

MapSelectInfoPacket::~MapSelectInfoPacket()
{
    if (mOwnsData)
        delete[] mData;
}

void MapSelectInfoPacket::read(DataInputStream* input)
{
    mSize = static_cast<unsigned int>(input->readShort());
    if (mSize - 1 < 0x7ffe)
    {
        if (mData != NULL)
            delete[] mData;
        arrayWithLength<unsigned char> data(mSize, true);
        mData = data.data;
        mLength = data.length;
        input->readBytes(data);
    }
}

void MapSelectInfoPacket::write(DataOutputStream* output)
{
    output->writeShort(static_cast<short>(mSize));
    if (mData != NULL)
    {
        arrayWithLength<unsigned char> data;
        data.data = mData;
        data.length = mLength;
        output->writeBytes(data);
    }
}

void MapSelectInfoPacket::handle(PacketListener* listener)
{
    listener->handleMapSelectInfo(shared_from_this());
}

int MapSelectInfoPacket::getEstimatedSize()
{
    return static_cast<int>(mSize) + 2;
}

int MapSelectInfoPacket::getPacketId()
{
    return 0xa8;
}

boost::shared_ptr<Packet> MapSelectInfoPacket::create()
{
    return boost::shared_ptr<Packet>(new MapSelectInfoPacket());
}
