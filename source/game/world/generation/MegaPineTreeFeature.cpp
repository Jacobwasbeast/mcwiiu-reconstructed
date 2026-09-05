#include "game/world/generation/MegaPineTreeFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

static const BlockState* megaPineTrunkState = NULL;
static const BlockState* megaPineLeafState = NULL;
static const BlockState* megaPinePodzolState = NULL;

void MegaPineTreeFeature::staticCtor()
{
    megaPineTrunkState = Blocks::log->getBlockState(1);
    megaPineLeafState = Blocks::leaves->getBlockState(1);
    megaPinePodzolState = Blocks::dirt->getBlockState(2);
}

MegaPineTreeFeature::MegaPineTreeFeature(bool doBlockNotify, bool useHeight)
    : MegaTreeFeature(doBlockNotify, 13, 15, megaPineTrunkState, megaPineLeafState),
      mUseHeight(useHeight)
{
}

MegaPineTreeFeature::~MegaPineTreeFeature()
{
}

void MegaPineTreeFeature::createCrown(Level* level, int x, int z, int y,
                                      int radius, Random& random)
{
    int crownHeight = mUseHeight ? mHeight : 3;
    int crownStart = y - (random.nextInt(5) + crownHeight);
    int previousRadius = 0;

    for (int layer = crownStart; layer <= y; ++layer)
    {
        int distance = y - layer;
        int layerRadius = (int)(3.5f * (float)distance /
                                (float)(y - crownStart));
        if (distance > 0 && layerRadius == previousRadius &&
            (layer & 1) == 0)
        {
            ++layerRadius;
        }

        placeDoubleTrunkLeaves(level, BlockPos(x, layer, z),
                               radius + layerRadius);
        previousRadius = layerRadius;
    }
}

bool MegaPineTreeFeature::place(Level* level, Random& random,
                                const BlockPos& origin)
{
    int height = calcTreeHeigth(random);
    if (!prepareTree(level, random, origin, height))
    {
        return false;
    }

    createCrown(level, origin.x, origin.z, origin.y + height, 0, random);

    for (int y = 0; y < height; ++y)
    {
        BlockPos trunkPos = origin.above(y);
        Block* block = level->getBlockState(trunkPos)->getBlock();
        if (block == Blocks::leaves || block == Blocks::leaves2)
        {
            setBlock(level, trunkPos, mTrunkState, 0);
        }

        if (y < height - 1)
        {
            BlockPos eastPos = origin.offset(1, y, 0);
            block = level->getBlockState(eastPos)->getBlock();
            if (block == Blocks::leaves || block == Blocks::leaves2)
            {
                setBlock(level, eastPos, mTrunkState, 0);
            }

            BlockPos southEastPos = origin.offset(1, y, 1);
            block = level->getBlockState(southEastPos)->getBlock();
            if (block == Blocks::leaves || block == Blocks::leaves2)
            {
                setBlock(level, southEastPos, mTrunkState, 0);
            }

            BlockPos southPos = origin.offset(0, y, 1);
            block = level->getBlockState(southPos)->getBlock();
            if (block == Blocks::leaves || block == Blocks::leaves2)
            {
                setBlock(level, southPos, mTrunkState, 0);
            }
        }
    }

    return true;
}

void MegaPineTreeFeature::placePodzolAt(Level* level, const BlockPos& pos)
{
    for (int y = 2; y >= -3; --y)
    {
        BlockPos checkPos = pos.above(y);
        Block* block = level->getBlockState(checkPos)->getBlock();
        if (block == Blocks::grass || block == Blocks::dirt)
        {
            setBlock(level, checkPos, megaPinePodzolState, 0);
            return;
        }

        if (y < 0 && block != Blocks::AIR)
        {
            return;
        }
    }
}

void MegaPineTreeFeature::placePodzolCircle(Level* level, const BlockPos& pos)
{
    for (int x = -2; x <= 2; ++x)
    {
        for (int z = -2; z <= 2; ++z)
        {
            if (x * x + z * z < 8)
            {
                placePodzolAt(level, pos.offset(x, 0, z));
            }
        }
    }
}

void MegaPineTreeFeature::postPlaceTree(Level* level, Random& random,
                                        const BlockPos& pos)
{
    placePodzolCircle(level, pos.west().north());
    placePodzolCircle(level, pos.east(2).north());
    placePodzolCircle(level, pos.west().south(2));
    placePodzolCircle(level, pos.east(2).south(2));

    for (int i = 0; i < 5; ++i)
    {
        int value = random.nextInt(64);
        int x = value & 7;
        int z = value >> 3;
        if (x == 0 || x == 7 || z == 0 || z == 7)
        {
            placePodzolCircle(level, pos.offset(x - 3, 0, z - 3));
        }
    }
}
