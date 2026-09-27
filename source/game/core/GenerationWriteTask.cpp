#include "game/core/GenerationWriteTask.hpp"

#include <deque>

#include "game/core/System.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

GenerationWriteTask::GenerationWriteTask(Level* level, BlockTransaction* transaction,
                                         const BlockPos& origin, int* changedBlocks)
    : mLevel(level), mTransaction(transaction),
      mChunks(transaction->getAffectedChunks(level, origin)), mOrigin(origin),
      mChangedBlocks(changedBlocks)
{
}

GenerationWriteTask::~GenerationWriteTask()
{
    delete mTransaction;
    delete mChunks;
}

void GenerationWriteTask::doWork(long long endTime)
{
    if (mLevel->getChunkSource() != NULL)
    {
        while (!mChunks->empty() && System::processTimeInMilliSecs() < endTime)
        {
            ChunkPos chunk = mChunks->front();
            mChunks->pop_front();

            if (mLevel->getChunk(chunk.x, chunk.z) == NULL)
            {
                Sleep(1);
                mChunks->push_back(chunk);
                continue;
            }

            if (mLevel->mIsClientSide)
            {
                if (!mLevel->reallyHasChunk(chunk.x, chunk.z))
                {
                    Sleep(1);
                    mChunks->push_back(chunk);
                    continue;
                }
            }

            if (!mLevel->mIsClientSide)
            {
                if (mLevel->getChunkSource()->getChunk(chunk.x, chunk.z) == NULL)
                {
                    Sleep(1);
                    mChunks->push_back(chunk);
                    continue;
                }
            }

            Vec3i min = mOrigin.offset(mTransaction->getMin());
            Vec3i max = mOrigin.offset(mTransaction->getMax());
            BlockPos from(chunk.x << 4 < min.x ? min.x : chunk.x << 4,
                          min.y,
                          chunk.z << 4 < min.z ? min.z : chunk.z << 4);
            BlockPos to(chunk.x * 16 + 16 < max.x ? chunk.x * 16 + 16 : max.x,
                        max.y,
                        chunk.z * 16 + 16 < max.z ? chunk.z * 16 + 16 : max.z);

            for (int x = from.x; x < to.x; ++x)
            {
                for (int y = from.y; y < to.y; ++y)
                {
                    for (int z = from.z; z < to.z; ++z)
                    {
                        Vec3i position = Vec3i(x, y, z).subtract(mOrigin);
                        if (mTransaction->inBounds(position) && mTransaction->shouldWrite(position))
                        {
                            int block = mTransaction->getBlock(position);
                            unsigned short data = mTransaction->getData(position);
                            if (block == 217) block = 0;

                            bool countsAsChanged = block != 0 && block != 106 && block != 46 &&
                                                   block != 70 && block != 51 && block != 31;
                            if (mChangedBlocks != NULL && countsAsChanged) ++*mChangedBlocks;
                            mLevel->setBlockAndData(x, y, z, block, data, Block::UPDATE_CLIENTS);
                        }
                    }
                }
            }

            mLevel->setBlocksDirty(from, to);
        }
    }
}

bool GenerationWriteTask::hasFinished() const
{
    return mChunks->empty();
}
