#include "game/network/MapSelectInfoPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

MapSelectInfoPacket::MapSelectInfoPacket()
    : mData(NULL), mLength(0), mOwnsData(false), mSize(0)
{
}

MapSelectInfoPacket::MapSelectInfoPacket(arrayWithLength<unsigned char> data, bool ownsData)
    : mData(data.data), mLength(data.length), mOwnsData(ownsData), mSize(data.length)
{
}

MapSelectInfoPacket::~MapSelectInfoPacket()
{
    if (mOwnsData)
        delete[] mData;
}

void MapSelectInfoPacket::read(DataInputStream* input)
{
    unsigned int length = static_cast<unsigned int>(input->readUnsignedShort());
    if (length == 0)
    {
        if (mOwnsData)
            delete[] mData;
        mData = NULL;
        mLength = 0;
        mSize = 0;
        mOwnsData = false;
        return;
    }
    if (length > 0 && length < 0x7fff)
    {
        if (mOwnsData)
            delete[] mData;
        mData = new unsigned char[length];
        mLength = length;
        mSize = length;
        mOwnsData = true;
        arrayWithLength<unsigned char> data;
        data.data = mData;
        data.length = mLength;
        input->read(data);
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
        output->write(data);
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
