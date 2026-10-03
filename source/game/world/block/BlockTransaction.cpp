#include "game/world/block/BlockTransaction.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/ChunkPos.hpp"

BlockTransaction::Element g_emptyBlockElement;

BlockTransaction::BlockTransaction(const BlockPos& min, const BlockPos& max, const Vec3i& size,
                                   Element* elements, unsigned int elementCount)
    : mSize(size), mElementCount(elementCount), mMin(min), mMax(0, 0, 0), mElements(elements)
{
}

BlockTransaction::~BlockTransaction()
{
    delete[] mElements;
}

BlockTransaction* BlockTransaction::CreateTransaction(const BlockPos& min, const BlockPos& max)
{
    Vec3i size(max.x - min.x, max.y - min.y, max.z - min.z);
    unsigned int elementCount = size.x * size.y * size.z;
    Element* elements = new Element[elementCount]();
    return new BlockTransaction(min, max, size, elements, elementCount);
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
    if (index >= mElementCount)
        return &g_emptyBlockElement;
    return mElements + index;
}

bool BlockTransaction::inBounds(const Vec3i& position) const
{
    return position.x >= 0 && position.x < mSize.x &&
           position.y >= 0 && position.y < mSize.y &&
           position.z >= 0 && position.z < mSize.z;
}

bool BlockTransaction::shouldWrite(const Vec3i& position)
{
    return at(position)->value != 0;
}

std::deque<ChunkPos>* BlockTransaction::getAffectedChunks(Level*, const Vec3i& position)
{
    Vec3i end = position.offset(mSize);
    int minChunkX = position.x >> 4;
    int minChunkZ = position.z >> 4;
    int maxChunkX = (end.x - 1) >> 4;
    int maxChunkZ = (end.z - 1) >> 4;
    std::deque<ChunkPos>* chunks = new std::deque<ChunkPos>();
    for (int chunkX = minChunkX; chunkX <= maxChunkX; ++chunkX)
    {
        for (int chunkZ = minChunkZ; chunkZ <= maxChunkZ; ++chunkZ)
        {
            chunks->push_back(ChunkPos(chunkX, chunkZ));
        }
    }
    return chunks;
}

void BlockTransaction::setBlock(const Vec3i& position, int block, int data)
{
    at(position)->value = (unsigned short)((block << 4) | data);
    if (block != 0) updateMinMax(position);
}

void BlockTransaction::setElement(const Vec3i& position, unsigned short element)
{
    at(position)->value = element;
    if (element != 0) updateMinMax(position);
}

void BlockTransaction::setBlock(const Vec3i& position, const BlockState* state)
{
    Block* block = state->getBlock();
    setBlock(position, Block::getId(block), block->convertBlockStateToLegacyData(state) & 0xf);
}

int BlockTransaction::getBlock(const Vec3i& position) const
{
    return (int)(at(position)->value >> 4);
}

unsigned short BlockTransaction::getData(const Vec3i& position) const
{
    return at(position)->value & 0xf;
}

bool BlockTransaction::isEmpty(const Vec3i& position) const
{
    return getBlock(position) == 0;
}

void BlockTransaction::reduceMinMax()
{
    Vec3i min = mMin;
    Vec3i max = mMax;
    mMin = mMax;
    for (int x = min.x; x < max.x; ++x)
    {
        for (int y = min.y; y < max.y; ++y)
        {
            for (int z = min.z; z < max.z; ++z)
            {
                Vec3i position(x, y, z);
                if (!isEmpty(position)) updateMinMax(position);
            }
        }
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
