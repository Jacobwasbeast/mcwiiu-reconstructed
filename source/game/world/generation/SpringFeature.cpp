#include "game/world/generation/SpringFeature.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

SpringFeature::SpringFeature(Block* block)
    : Feature(false), mBlock(block)
{
}

SpringFeature::~SpringFeature()
{
}

bool SpringFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (options != NULL && options->checkIntersects(origin, origin))
    {
        return false;
    }

    if (level->getBlockId(origin.x, origin.y + 1, origin.z) != 1)
    {
        return false;
    }
    if (level->getBlockId(origin.x, origin.y - 1, origin.z) != 1)
    {
        return false;
    }

    int centerBlockId = level->getBlockId(origin.x, origin.y, origin.z);
    if (centerBlockId != 0 &&
        level->getBlockId(origin.x, origin.y, origin.z) != 1)
    {
        return false;
    }

    int solidNeighbourCount = 0;
    int blockId = level->getBlockId(origin.x - 1, origin.y, origin.z);
    if (blockId == 1)
    {
        solidNeighbourCount = 1;
        blockId = level->getBlockId(origin.x + 1, origin.y, origin.z);
    }
    else if (blockId != 0)
    {
        return false;
    }
    else
    {
        blockId = level->getBlockId(origin.x + 1, origin.y, origin.z);
    }

    if (blockId == 1)
    {
        ++solidNeighbourCount;
        blockId = level->getBlockId(origin.x, origin.y, origin.z - 1);
    }
    else if (blockId != 0)
    {
        return false;
    }
    else
    {
        blockId = level->getBlockId(origin.x, origin.y, origin.z - 1);
    }

    if (blockId == 1)
    {
        ++solidNeighbourCount;
        blockId = level->getBlockId(origin.x, origin.y, origin.z + 1);
    }
    else if (blockId != 0)
    {
        return false;
    }
    else
    {
        blockId = level->getBlockId(origin.x, origin.y, origin.z + 1);
    }

    if (blockId == 1)
    {
        if (solidNeighbourCount != 2)
        {
            return true;
        }
    }
    else
    {
        if (blockId != 0)
        {
            return false;
        }
        if (solidNeighbourCount != 3)
        {
            return true;
        }
    }

    const BlockState* state = mBlock->defaultBlockState();
    level->setBlock(origin, state, 2, false);
    level->instaTick(origin, state, random);
    return true;
}
