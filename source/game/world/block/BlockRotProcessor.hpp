#ifndef GAME_WORLD_BLOCK_BLOCKROTPROCESSOR_HPP
#define GAME_WORLD_BLOCK_BLOCKROTPROCESSOR_HPP

#include "game/world/generation/StructureProcessor.hpp"

class BlockPos;
class Random;
class StructurePlaceSettings;

class BlockRotProcessor : public StructureProcessor
{
public:
    BlockRotProcessor(StructurePlaceSettings* settings, const BlockPos& position);
    virtual ~BlockRotProcessor();

    virtual StructureTemplate::StructureBlockInfo* processBlock(Level* level,
        const BlockPos& position, StructureTemplate::StructureBlockInfo* blockInfo);

private:
    float mIntegrity;
    Random* mRandom;
    bool mOwnsRandom;
};

#endif
