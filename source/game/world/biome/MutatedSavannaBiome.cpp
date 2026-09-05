#include "game/world/biome/MutatedSavannaBiome.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"

MutatedSavannaBiome::MutatedSavannaBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : SavannaBiome(biomeId, properties)
{
    mDecorator->flowersPerChunk = 2;
    mDecorator->treesPerChunk = 2;
    mDecorator->grassPerChunk = 5;
}

MutatedSavannaBiome::~MutatedSavannaBiome()
{
}

void MutatedSavannaBiome::buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer,
                                          int x, int z, double noise)
{
    mTopBlock = BlockPair(2, 0);
    mFillerBlock = BlockPair(3, 0);

    if (noise <= 1.75)
    {
        if (noise > -0.5)
        {
            Block* dirt = Block::byId(3);
            mTopBlock.blockId = 3;
            mTopBlock.blockData = dirt->convertBlockStateToLegacyData(dirt->getBlockState(1));
        }
    }
    else
    {
        mTopBlock = BlockPair(1, 0);
        mFillerBlock = BlockPair(1, 0);
    }

    Biome::buildSurfaceAt(level, random, primer, x, z, noise);
}

void MutatedSavannaBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    Biome::decorate(level, random, pos);
}
