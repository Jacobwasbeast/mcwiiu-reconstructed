#include "game/client/render/LevelRenderer.hpp"

LevelRenderer::DestroyedBlockManager::RecentBlock::RecentBlock(const BlockPos& blockPos, Level* sourceLevel)
    : pos(blockPos), level(sourceLevel), ticks(20), remove(false)
{
}

LevelRenderer::DestroyedBlockManager::RecentBlock::~RecentBlock()
{
    for (std::vector<AABB*>::iterator box = boxes.begin(); box != boxes.end(); ++box)
        delete *box;
}

LevelRenderer::DestroyedBlockManager::DestroyedBlockManager()
{
    InitializeCriticalSection(&lock);
}

void LevelRenderer::DestroyedBlockManager::tick()
{
    EnterCriticalSection(&lock);
    std::vector<RecentBlock*>::iterator block = blocks.begin();
    while (block != blocks.end())
    {
        --(*block)->ticks;
        if ((*block)->ticks == 0)
        {
            delete *block;
            *block = blocks.back();
            blocks.pop_back();
        }
        else
        {
            ++block;
        }
    }
    LeaveCriticalSection(&lock);
}

void LevelRenderer::DestroyedBlockManager::updatedChunkAt(Level* sourceLevel, const BlockPos& chunkPos, int radius)
{
    EnterCriticalSection(&lock);
    std::vector<RecentBlock*>::iterator block = blocks.begin();
    while (block != blocks.end())
    {
        RecentBlock* recent = *block;
        if (recent->level == sourceLevel &&
            recent->pos.x >= chunkPos.x && recent->pos.x < chunkPos.x + 16 &&
            recent->pos.y >= chunkPos.y && recent->pos.y < chunkPos.y + 16 &&
            recent->pos.z >= chunkPos.z && recent->pos.z < chunkPos.z + 16)
            recent->remove = true;
        ++block;
    }

    if (radius < 2)
    {
        block = blocks.begin();
        while (block != blocks.end())
        {
            if ((*block)->remove)
            {
                delete *block;
                *block = blocks.back();
                blocks.pop_back();
            }
            else
            {
                ++block;
            }
        }
    }
    LeaveCriticalSection(&lock);
}

bool LevelRenderer::sortEntDisFunc(std::pair<int, double> first, std::pair<int, double> second)
{
    return first.second < second.second;
}

int LevelRenderer::getDimensionIndexFromId(int id)
{
    return (3 - id) % 3;
}

bool LevelRenderer::isInCloud(double, double, double, float)
{
    return false;
}

void LevelRenderer::playSound(boost::shared_ptr<Player>, const SoundEvent*, SoundSource::ESoundSource,
                              double, double, double, float, float, float)
{
}

void LevelRenderer::playerRemoved(boost::shared_ptr<Entity>)
{
}
