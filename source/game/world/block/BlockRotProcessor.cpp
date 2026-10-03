#include "game/world/block/BlockRotProcessor.hpp"

#include "game/util/Random.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"

BlockRotProcessor::~BlockRotProcessor()
{
    if (mOwnsRandom)
        delete mRandom;
}

StructureTemplate::StructureBlockInfo* BlockRotProcessor::processBlock(Level* level,
    const BlockPos& position, StructureTemplate::StructureBlockInfo* blockInfo)
{
    if (mIntegrity < 1.0f && mIntegrity < mRandom->nextFloat())
        return NULL;
    return blockInfo;
}
