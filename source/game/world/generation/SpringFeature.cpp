#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/world/generation/SpringFeature.hpp"
#include "game/world/block/BlockId.hpp"

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
    if (gApp.getLevelGenerationOptions() != NULL
        && gApp.getLevelGenerationOptions()->checkIntersects(origin, origin))
    {
        return false;
    }

    if (level->getBlockId(origin.x, origin.y + 1, origin.z) != BlockId::STONE)
    {
        return false;
    }
    if (level->getBlockId(origin.x, origin.y - 1, origin.z) != BlockId::STONE)
    {
        return false;
    }

    if (level->getBlockId(origin.x, origin.y, origin.z) != BlockId::AIR
        && level->getBlockId(origin.x, origin.y, origin.z) != BlockId::STONE)
    {
        return false;
    }

    int rockCount = 0;
    int blockId = level->getBlockId(origin.x - 1, origin.y, origin.z);
    if (blockId == BlockId::STONE)
    {
        ++rockCount;
    }
    else if (blockId != BlockId::AIR)
    {
        return false;
    }

    blockId = level->getBlockId(origin.x + 1, origin.y, origin.z);
    if (blockId == BlockId::STONE)
    {
        ++rockCount;
    }
    else if (blockId != BlockId::AIR)
    {
        return false;
    }

    blockId = level->getBlockId(origin.x, origin.y, origin.z - 1);
    if (blockId == BlockId::STONE)
    {
        ++rockCount;
    }
    else if (blockId != BlockId::AIR)
    {
        return false;
    }

    blockId = level->getBlockId(origin.x, origin.y, origin.z + 1);
    if (blockId == BlockId::STONE)
    {
        ++rockCount;
    }
    else if (blockId != BlockId::AIR)
    {
        return false;
    }

    if (rockCount == 3)
    {
        const BlockState* state = mBlock->defaultBlockState();
        level->setBlock(origin, state, Block::UPDATE_CLIENTS, false);
        level->instaTick(origin, state, random);
    }
    return true;
}
