#include "game/world/generation/OreFeature.hpp"

#include <algorithm>
#include <cmath>
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

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

OreFeature::OreFeature(const BlockState* state, int numberOfBlocks)
    : Feature(false), mBlockId(0), mBlockData(0), mNumberOfBlocks(0), mPredicate(NULL)
{
    static NaturalStonePredicate naturalStonePredicate;
    _init(state, numberOfBlocks, &naturalStonePredicate);
}

OreFeature::OreFeature(const BlockState* state, int numberOfBlocks,
                       const Predicate<const Biome::BlockPair&>* predicate)
    : Feature(false), mBlockId(0), mBlockData(0), mNumberOfBlocks(0), mPredicate(NULL)
{
    _init(state, numberOfBlocks, predicate);
}

OreFeature::~OreFeature()
{
}

void OreFeature::_init(const BlockState* state, int numberOfBlocks,
                       const Predicate<const Biome::BlockPair&>* predicate)
{
    Block* block = state->getBlock();
    mBlockId = Block::getId(block);
    mBlockData = block->getSpawnResourcesAuxValue(state);
    mNumberOfBlocks = numberOfBlocks;
    mPredicate = predicate;
}

bool OreFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    unsigned char mask[2048];
    PIXBeginNamedEvent(0.0f, "Place Ore Feature");
    PIXBeginNamedEvent(0.0f, "Prep");

    const float angle = random.nextFloat() * 3.1415927f;
    const double x0 = origin.x + 8.0 + Mth::sin(angle) * mNumberOfBlocks * 0.125;
    const double x1 = origin.x + 8.0 - Mth::sin(angle) * mNumberOfBlocks * 0.125;
    const double z0 = origin.z + 8.0 + Mth::cos(angle) * mNumberOfBlocks * 0.125;
    const double z1 = origin.z + 8.0 - Mth::cos(angle) * mNumberOfBlocks * 0.125;
    const double y0 = origin.y + random.nextInt(3) - 2;
    const double y1 = origin.y + random.nextInt(3) - 2;
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Level generation options");
    const double radius = (mNumberOfBlocks * 0.0625 + 1.0) * 0.5;
    const double minYValue = std::min(y0, y1) - radius;
    const double maxYValue = std::max(y0, y1) + radius;
    const double minXValue = std::min(x0, x1) - radius;
    const double maxXValue = std::max(x0, x1) + radius;
    const double minZValue = std::min(z0, z1) - radius;
    const double maxZValue = std::max(z0, z1) + radius;
    const int minX = static_cast<int>(std::floor(minXValue));
    const int maxX = static_cast<int>(std::floor(maxXValue));
    const int minY = static_cast<int>(std::floor(minYValue));
    const int maxY = static_cast<int>(std::floor(maxYValue));
    const int minZ = static_cast<int>(std::floor(minZValue));
    const int maxZ = static_cast<int>(std::floor(maxZValue));
    const int xSize = maxX - minX + 1;
    const int ySize = maxY - minY + 1;
    const int zSize = maxZ - minZ + 1;
    const int bitCount = xSize * ySize * zSize;
    std::memset(mask, 0, (bitCount + 7) / 8);

    bool intersects = false;
    if (g_pApp->getLevelGenerationOptions() != NULL)
    {
        intersects = g_pApp->getLevelGenerationOptions()->checkIntersects(
            BlockPos(minX, minY, minZ), BlockPos(maxX, maxY, maxZ));
    }
    PIXEndNamedEvent();

    const int seaLevel = level->getSeaLevel();
    PIXBeginNamedEvent(0.0f, "Attempting %d placements", mNumberOfBlocks);
    for (int placement = 0; placement <= mNumberOfBlocks; ++placement)
    {
        const float progress = static_cast<float>(placement) / static_cast<float>(mNumberOfBlocks);
        const double x = (x1 - x0) * progress + x0;
        const double y = (y1 - y0) * progress + y0;
        const double z = (z1 - z0) * progress + z0;
        const double width = random.nextDouble() * mNumberOfBlocks * 0.0625;
        const int lowerX = static_cast<int>(std::floor(x - width));
        const int upperX = static_cast<int>(std::floor(x + width));
        const int lowerY = static_cast<int>(std::floor(y - width));
        const int upperY = static_cast<int>(std::floor(y + width));
        const int lowerZ = static_cast<int>(std::floor(z - width));
        const int upperZ = static_cast<int>(std::floor(z + width));

        if (intersects)
        {
            if (g_pApp->getLevelGenerationOptions()->checkIntersects(
                    BlockPos(lowerX, lowerY, lowerZ), BlockPos(upperX, upperY, upperZ)))
            {
                continue;
            }
        }
        else if (seaLevel < lowerY &&
                 level->getHeightmap(lowerX, lowerZ) < lowerY &&
                 level->getHeightmap(upperX, lowerZ) < lowerY &&
                 level->getHeightmap(lowerX, upperZ) < lowerY &&
                 level->getHeightmap(upperX, upperZ) < lowerY)
        {
            continue;
        }

        const int localXSize = upperX - lowerX + 1;
        const int localYSize = upperY - lowerY + 1;
        const int localZSize = upperZ - lowerZ + 1;
        for (int localX = 0; localX < localXSize; ++localX)
        {
            const double dx = ((lowerX + localX) + 0.5 - x) / width;
            for (int localZ = 0; localZ < localZSize; ++localZ)
            {
                const double dz = ((lowerZ + localZ) + 0.5 - z) / width;
                for (int localY = 0; localY < localYSize; ++localY)
                {
                    const double dy = ((lowerY + localY) + 0.5 - y) / width;
                    if (dx * dx + dy * dy + dz * dz >= 1.0)
                    {
                        continue;
                    }
                    const int index = ((localX + lowerX - minX) * ySize +
                                       (localZ + lowerZ - minZ)) * zSize +
                                      localY + lowerY - minY;
                    const unsigned char bit = static_cast<unsigned char>(1 << (index & 7));
                    if ((mask[index >> 3] & bit) != 0)
                    {
                        continue;
                    }
                    mask[index >> 3] = static_cast<unsigned char>(mask[index >> 3] | bit);
                    const int blockX = lowerX + localX;
                    const int blockY = lowerY + localY;
                    const int blockZ = lowerZ + localZ;
                    const unsigned short element = level->getElement(blockX, blockY, blockZ);
                    Biome::BlockPair pair(static_cast<int>(element >> 4), static_cast<int>(element & 15));
                    if (mPredicate->apply(pair))
                    {
                        level->setBlockAndData(blockX, blockY, blockZ, mBlockId, mBlockData, 0x74);
                    }
                }
            }
        }
    }
    PIXEndNamedEvent();
    PIXEndNamedEvent();
    return true;
}

bool OreFeature::NaturalStonePredicate::apply(const Biome::BlockPair& pair) const
{
    return pair.blockId == Block::getId(Blocks::stone) &&
           StoneBlock::Variant::byData(pair.blockData)->isNatural();
}

OreFeature::NaturalStonePredicate::~NaturalStonePredicate()
{
}
