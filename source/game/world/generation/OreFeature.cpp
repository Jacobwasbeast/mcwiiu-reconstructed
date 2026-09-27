#include "game/world/generation/OreFeature.hpp"

#include <algorithm>
#include <cstring>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/core/PIXNamedEvents.hpp"

OreFeature::OreFeature(const BlockState* state, int numberOfBlocks)
    : Feature(false), mBlockId(0), mBlockData(0)
{
    static NaturalStonePredicate naturalStonePredicate;
    _init(state, numberOfBlocks, &naturalStonePredicate);
}

OreFeature::OreFeature(const BlockState* state, int numberOfBlocks,
                       const Predicate<const Biome::BlockPair&>* predicate)
    : Feature(false), mBlockId(0), mBlockData(0)
{
    _init(state, numberOfBlocks, predicate);
}

OreFeature::~OreFeature()
{
}

void OreFeature::_init(const BlockState* state, int numberOfBlocks,
                       const Predicate<const Biome::BlockPair&>* predicate)
{
    mBlockId = Block::getId(state->getBlock());
    mBlockData = state->getBlock()->getSpawnResourcesAuxValue(state);
    mNumberOfBlocks = numberOfBlocks;
    mPredicate = predicate;
}

bool OreFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    unsigned char mask[2048];
    PIXBeginNamedEvent(0.0f, "Place Ore Feature");
    PIXBeginNamedEvent(0.0f, "Prep");

    const float angle = random.nextFloat() * 3.1415927f;
    const double x0 = (float)(origin.x + 8) + Mth::sin(angle) * (float)mNumberOfBlocks * 0.125f;
    const double x1 = (float)(origin.x + 8) - Mth::sin(angle) * (float)mNumberOfBlocks * 0.125f;
    const double z0 = (float)(origin.z + 8) + Mth::cos(angle) * (float)mNumberOfBlocks * 0.125f;
    const double z1 = (float)(origin.z + 8) - Mth::cos(angle) * (float)mNumberOfBlocks * 0.125f;
    const double y0 = origin.y + random.nextInt(3) - 2;
    const double y1 = origin.y + random.nextInt(3) - 2;
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Level generation options");
    const double radius = (2.0 * (mNumberOfBlocks * 0.0625) + 1.0) * 0.5;
    const int minX = Mth::floor(std::min(x0, x1) - radius);
    const int minY = Mth::floor(std::min(y0, y1) - radius);
    const int minZ = Mth::floor(std::min(z0, z1) - radius);
    const int maxX = Mth::floor(std::max(x0, x1) + radius);
    const int maxY = Mth::floor(std::max(y0, y1) + radius);
    const int maxZ = Mth::floor(std::max(z0, z1) + radius);
    const int xSize = maxX - minX + 1;
    const int ySize = maxY - minY + 1;
    const int zSize = maxZ - minZ + 1;
    std::memset(mask, 0, (xSize * ySize * zSize + 7) / 8);

    LevelGenerationOptions* options = NULL;
    bool intersects = false;
    if (g_pApp->getLevelGenerationOptions() != NULL)
    {
        options = g_pApp->getLevelGenerationOptions();
        intersects = options->checkIntersects(BlockPos(minX, minY, minZ), BlockPos(maxX, maxY, maxZ));
    }
    PIXEndNamedEvent();

    const int seaLevel = level->getSeaLevel();
    PIXBeginNamedEvent(0.0f, "Attempting %d placements", mNumberOfBlocks);
    for (int placement = 0; placement <= mNumberOfBlocks; ++placement)
    {
        const float progress = (float)placement / (float)mNumberOfBlocks;
        const double x = (x1 - x0) * progress + x0;
        const double y = (y1 - y0) * progress + y0;
        const double z = (z1 - z0) * progress + z0;
        const double noise = random.nextDouble();
        const double width = ((Mth::sin(3.1415927f * progress) + 1.0f) * noise * mNumberOfBlocks * 0.0625 + 1.0) * 0.5;
        const int lowerX = Mth::floor(x - width);
        const int lowerY = Mth::floor(y - width);
        const int lowerZ = Mth::floor(z - width);
        const int upperX = Mth::floor(x + width);
        const int upperY = Mth::floor(y + width);
        const int upperZ = Mth::floor(z + width);

        if (intersects && options != NULL)
        {
            PIXBeginNamedEvent(0.0f, "Check intersects");
            bool localIntersection = options->checkIntersects(BlockPos(lowerX, lowerY, lowerZ),
                                                              BlockPos(upperX, upperY, upperZ));
            PIXEndNamedEvent();
            if (localIntersection)
            {
                continue;
            }
        }

        if (seaLevel < y0)
        {
            PIXBeginNamedEvent(0.0f, "Early reject tests");
            bool reject = false;
            if (level->getHeightmap(lowerX, lowerZ) < lowerY && level->getHeightmap(upperX, lowerZ) < lowerY &&
                level->getHeightmap(lowerX, upperZ) < lowerY && level->getHeightmap(upperX, upperZ) < lowerY)
            {
                reject = true;
            }
            PIXEndNamedEvent();
            if (reject)
            {
                continue;
            }
        }

        PIXBeginNamedEvent(0.0f, "Setting blocks");
        const int radiusSquared = (int)(width * width * 65536.0);
        int dx = (int)((lowerX + 0.5 - x) * 256.0);
        for (int blockX = lowerX; blockX <= upperX; ++blockX, dx += 256)
        {
            if (dx * dx >= radiusSquared)
            {
                continue;
            }

            int dy = (int)((lowerY + 0.5 - y) * 256.0);
            for (int blockY = lowerY; blockY <= upperY; ++blockY, dy += 256)
            {
                if (dx * dx + dy * dy >= radiusSquared)
                {
                    continue;
                }

                int dz = (int)((lowerZ + 0.5 - z) * 256.0);
                int index = ((blockX - minX) * ySize + (blockY - minY)) * zSize + (lowerZ - minZ);
                for (int blockZ = lowerZ; blockZ <= upperZ; ++blockZ, dz += 256)
                {
                    if (dx * dx + dy * dy + dz * dz < radiusSquared)
                    {
                        const unsigned char bit = (unsigned char)(1 << (index & 7));
                        if ((mask[index >> 3] & bit) != 0)
                        {
                            continue;
                        }
                        mask[index >> 3] |= bit;

                        PIXBeginNamedEvent(0.0f, "Applying");
                        const int element = level->getElement(blockX, blockY, blockZ);
                        if ((element >> 4) != 0)
                        {
                            Biome::BlockPair pair(element >> 4, element & 15);
                            if (mPredicate->apply(pair))
                            {
                                level->setBlockAndData(blockX, blockY, blockZ, mBlockId, mBlockData, Block::UPDATE_INVISIBLE | Block::UPDATE_NO_OBSERVERS | Block::UPDATE_LIGHT_IF_CHANGED | Block::UPDATE_CHUNK_ONLY);
                            }
                        }
                        PIXEndNamedEvent();
                    }
                    ++index;
                }
            }
        }
        PIXEndNamedEvent();
    }
    PIXEndNamedEvent();
    PIXEndNamedEvent();
    return true;
}

bool OreFeature::NaturalStonePredicate::apply(const Biome::BlockPair& pair) const
{
    static int stoneId = Block::getId(Blocks::stone);
    if (pair.blockId != stoneId)
    {
        return false;
    }
    return StoneBlock::Variant::BY_DATA[pair.blockData]->isNatural();
}

OreFeature::NaturalStonePredicate::~NaturalStonePredicate()
{
}
