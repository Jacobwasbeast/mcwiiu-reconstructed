#include "game/network/BlockRegionUpdatePacket.hpp"

#include <cstring>

#include "game/core/Compression.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/Level.hpp"

void MemSect(int section);

BlockRegionUpdatePacket::BlockRegionUpdatePacket()
    : mX(0), mY(0), mZ(0), mWidth(0), mHeight(0), mDepth(0), mData(NULL),
      mDataSize(0), mCompressionType(0), mFullChunk(false),
      mHasBlockChanges(false), mChanges(new std::vector<LevelChunk::BlockChange>()),
      mEstimatedSize(0)
{
}

BlockRegionUpdatePacket::BlockRegionUpdatePacket(int x, int y, int z, int width, int height, int depth,
                                                 Level* level, bool hasBlockChanges,
                                                 const std::vector<short>& blockChanges)
    : mX(x), mY(y), mZ(z), mWidth(width), mHeight(height), mDepth(depth), mData(NULL),
      mDataSize(0), mCompressionType(0), mFullChunk(false),
      mHasBlockChanges(hasBlockChanges), mChanges(new std::vector<LevelChunk::BlockChange>()),
      mEstimatedSize(0)
{
    mFullChunk = width == 16 && height == 256 && depth == 16 &&
                 (x & 15) == 0 && y == 0 && (z & 15) == 0;

    if (!hasBlockChanges)
    {
        arrayWithLength<unsigned char> rawData;
        if (mFullChunk)
        {
            rawData = level->getChunkAt(BlockPos(x, 0, z))->getRawData();
        }
        else
        {
            MemSect(0x32);
            rawData = level->getBlocksAndData(x, y, z, width, height, depth, false);
            MemSect(0);
        }

        if (rawData.data != NULL)
        {
            unsigned int compressedSize = 0x30000;
            unsigned char* compressed = new unsigned char[compressedSize];
            unsigned int rleSize = 0;
            Compression::getCompression()->CompressLZXRLE(compressed, &compressedSize,
                                                            rawData.data, rawData.length, &rleSize);
            mData = new unsigned char[compressedSize];
            std::memcpy(mData, compressed, compressedSize);
            delete[] compressed;
            delete[] rawData.data;
            mDataSize = compressedSize;
            mEstimatedSize = compressedSize;
        }
    }
    else
    {
        for (std::vector<short>::const_iterator it = blockChanges.begin(); it != blockChanges.end(); ++it)
        {
            BlockPos pos = LevelChunk::UnpackBlockPos(*it);
            LevelChunk* chunk = level->getChunkAt(x, z);
            unsigned int element = chunk->getElement(pos.x, pos.y, pos.z);
            unsigned int extraElement = chunk->getExtraElement(pos.x, pos.y, pos.z);
            LevelChunk::BlockChange change;
            change = LevelChunk::BlockChange();
            mChanges->push_back(change);
        }
    }
}

BlockRegionUpdatePacket::~BlockRegionUpdatePacket()
{
    delete[] mData;
    delete mChanges;
}

void BlockRegionUpdatePacket::read(DataInputStream* input)
{
    unsigned int flags = input->readUnsignedByte();
    mX = input->readInt();
    mY = input->readShort();
    mZ = input->readInt();
    mWidth = input->readUnsignedByte() + 1;
    mHeight = input->readUnsignedByte() + 1;
    mDepth = input->readUnsignedByte() + 1;
    mFullChunk = (flags & 1) != 0;
    mHasBlockChanges = (flags & 4) != 0;
    if ((flags & 2) != 0)
        mDepth = 0;
    unsigned int packedSize = input->readInt();
    mCompressionType = packedSize >> 30;
    mDataSize = packedSize & 0x3fffffff;

    unsigned int changeCount;
    if (mDataSize == 0)
    {
        mData = NULL;
        mEstimatedSize = 0;
        changeCount = input->readInt();
        mChanges->reserve(changeCount);
    }
    else
    {
        arrayWithLength<unsigned char> compressed(mDataSize, false);
        unsigned int rawSize = mWidth * mHeight * mDepth * 5;
        if (mFullChunk)
            rawSize = LevelChunk::getRawDataMaxBytes();
        mData = new unsigned char[rawSize];
        mEstimatedSize = rawSize;
        unsigned int outputSize = rawSize;
        if (input->read(compressed) != 0)
            Compression::getCompression()->DecompressLZXRLE(mData, &outputSize,
                                                             compressed.data, mDataSize);
        delete[] compressed.data;
        changeCount = input->readInt();
        mChanges->reserve(changeCount);
    }

    for (unsigned int i = 0; i < changeCount; ++i)
    {
        LevelChunk::BlockChange change;
        change = LevelChunk::BlockChange();
        mChanges->push_back(change);
    }
}

void BlockRegionUpdatePacket::write(DataOutputStream* output)
{
    unsigned int flags = mFullChunk ? 1 : 0;
    if (mHasBlockChanges)
        flags |= 4;
    if (mDepth == 0)
        flags |= 2;
    output->writeByte(static_cast<unsigned char>(flags));
    output->writeInt(mX);
    output->writeShort(static_cast<short>(mY));
    output->writeInt(mZ);
    output->writeByte(static_cast<unsigned char>((mWidth - 1) & 0xff));
    output->writeByte(static_cast<unsigned char>((mHeight - 1) & 0xff));
    output->writeByte(static_cast<unsigned char>((mDepth - 1) & 0xff));
    output->writeInt(static_cast<int>(mDataSize | (mCompressionType << 30)));
    arrayWithLength<unsigned char> data;
    data.data = mData;
    data.length = mDataSize;
    output->write(data, 0, mDataSize);
    short count = mChanges == NULL ? 0 : static_cast<short>(mChanges->size());
    output->writeShort(count);
    if (mChanges != NULL)
    {
        for (std::vector<LevelChunk::BlockChange>::const_iterator it = mChanges->begin();
             it != mChanges->end(); ++it)
        {
            output->writeShort(static_cast<short>(it->GetPackedPos()));
            output->writeInt(static_cast<int>(it->GetElement()));
        }
    }
}

void BlockRegionUpdatePacket::handle(PacketListener* listener)
{
    listener->handleBlockRegionUpdate(shared_from_this());
}

unsigned int BlockRegionUpdatePacket::getEstimatedSize()
{
    return mEstimatedSize + 0x11;
}

void BlockRegionUpdatePacket::SetChanges(std::vector<LevelChunk::BlockChange>* changes)
{
    mChanges = changes;
}

std::vector<LevelChunk::BlockChange>* BlockRegionUpdatePacket::GetChanges() const
{
    return mChanges;
}

unsigned int BlockRegionUpdatePacket::getPacketId()
{
    return 0x33;
}

boost::shared_ptr<Packet> BlockRegionUpdatePacket::create()
{
    return boost::shared_ptr<Packet>(new BlockRegionUpdatePacket());
}
