#include "game/world/block/BlockTransaction.hpp"

#include <cstring>

#include "game/world/block/BlockState.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/Level.hpp"

BlockTransaction::BlockTransaction(const BlockPos&, const BlockPos&, const Vec3i& size,
                                   Element* elements, unsigned int elementCount)
    : mSize(size), mElementCount(elementCount), mMin(size), mMax(0, 0, 0), mElements(elements)
{
}

BlockTransaction::~BlockTransaction()
{
    delete[] mElements;
}

BlockTransaction* BlockTransaction::CreateTransaction(const BlockPos& min, const BlockPos& max)
{
    Vec3i size(max.x - min.x, max.y - min.y, max.z - min.z);
    unsigned int count = size.x * size.y * size.z;
    Element* elements = new Element[count]();
    return new BlockTransaction(min, max, size, elements, count);
}

void BlockTransaction::readFrom(Level* level, const Vec3i& position)
{
    for (int x = 0; x < mSize.x; ++x)
    {
        for (int y = 0; y < mSize.y; ++y)
        {
            for (int z = 0; z < mSize.z; ++z)
            {
                Vec3i local(x, y, z);
                Vec3i world(position.x + x, position.y + y, position.z + z);
                setBlock(local, level->getBlockId(world.x, world.y, world.z),
                         level->getBlockData(world.x, world.y, world.z));
            }
        }
    }
}

void BlockTransaction::updateMinMax(const Vec3i& position)
{
    if (position.x < mMin.x) mMin.x = position.x;
    if (position.y < mMin.y) mMin.y = position.y;
    if (position.z < mMin.z) mMin.z = position.z;
    if (position.x + 1 > mMax.x) mMax.x = position.x + 1;
    if (position.y + 1 > mMax.y) mMax.y = position.y + 1;
    if (position.z + 1 > mMax.z) mMax.z = position.z + 1;
}

BlockTransaction::Element* BlockTransaction::at(const Vec3i& position) const
{
    unsigned int index = position.y + mSize.y * position.x + mSize.y * mSize.x * position.z;
    static Element empty = 0;
    if (index >= mElementCount) return &empty;
    return mElements + index;
}

bool BlockTransaction::inBounds(const Vec3i& position) const
{
    return position.x >= 0 && position.x < mSize.x && position.y >= 0 && position.y < mSize.y &&
           position.z >= 0 && position.z < mSize.z;
}

bool BlockTransaction::shouldWrite(const Vec3i& position)
{
    return *at(position) != 0;
}

std::deque<ChunkPos>* BlockTransaction::getAffectedChunks(Level*, const Vec3i& position)
{
    Vec3i end = position.offset(mSize);
    int minX = position.x >> 4;
    int minZ = position.z >> 4;
    int maxX = (end.x - 1) >> 4;
    int maxZ = (end.z - 1) >> 4;
    std::deque<ChunkPos>* chunks = new std::deque<ChunkPos>();
    for (int x = minX; x <= maxX; ++x)
        for (int z = minZ; z <= maxZ; ++z)
            chunks->push_back(ChunkPos(x, z));
    return chunks;
}

void BlockTransaction::setBlock(const Vec3i& position, int block, int data)
{
    *at(position) = (unsigned short)((block << 4) | data);
    if (block != 0) updateMinMax(position);
}

void BlockTransaction::setElement(const Vec3i& position, unsigned short element)
{
    *at(position) = element;
    if (element != 0) updateMinMax(position);
}

void BlockTransaction::setBlock(const Vec3i& position, const BlockState* state)
{
    Block* block = state->getBlock();
    setBlock(position, Block::getId(block), block->convertBlockStateToLegacyData(state));
}

int BlockTransaction::getBlock(const Vec3i& position) const
{
    return (int)(*at(position) >> 4);
}

unsigned short BlockTransaction::getData(const Vec3i& position) const
{
    return *at(position) & 0xf;
}

bool BlockTransaction::isEmpty(const Vec3i& position) const
{
    return getBlock(position) == 0;
}

void BlockTransaction::reduceMinMax()
{
    mMin = mMax;
    for (int x = 0; x < mSize.x; ++x)
        for (int y = 0; y < mSize.y; ++y)
            for (int z = 0; z < mSize.z; ++z)
            {
                Vec3i position(x, y, z);
                if (!isEmpty(position)) updateMinMax(position);
            }
}

Vec3i BlockTransaction::getMin() const
{
    return mMin;
}

Vec3i BlockTransaction::getMax() const
{
    return mMax;
}

bool BlockTransaction::isEmpty() const
{
    return mMin.x >= mMax.x || mMin.y >= mMax.y || mMin.z >= mMax.z;
}
