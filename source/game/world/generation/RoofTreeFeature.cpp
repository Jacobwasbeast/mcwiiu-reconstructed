#include "game/world/generation/RoofTreeFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MutableBlockPos.hpp"

static const BlockState* roofTreeTrunkState = NULL;
static const BlockState* roofTreeLeafState = NULL;

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

RoofTreeFeature::RoofTreeFeature(bool doBlockNotify)
    : AbstractTreeFeature(doBlockNotify, 6,
                         Blocks::log->defaultBlockState(),
                         Blocks::leaves->defaultBlockState(), false)
{
    roofTreeTrunkState = mTrunkState;
    roofTreeLeafState = mLeafState;
}

RoofTreeFeature::~RoofTreeFeature()
{
}

void RoofTreeFeature::staticCtor()
{
    roofTreeTrunkState = Blocks::log->defaultBlockState();
    roofTreeLeafState = Blocks::leaves->defaultBlockState();
}

bool RoofTreeFeature::canPlaceTreeOfHeight(Level* level, const BlockPos& pos, int height)
{
    for (int y = 0; y <= height + 1; ++y)
    {
        int radius = y == 0 ? 0 : (y >= height - 1 ? 2 : 1);
        for (int x = -radius; x <= radius; ++x)
        {
            for (int z = -radius; z <= radius; ++z)
            {
                if (!isFree(level->getBlock(pos.offset(x, y, z))))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

void RoofTreeFeature::placeLogAt(Level* level, const BlockPos& pos)
{
    if (isFree(level->getBlock(pos)))
    {
        setBlock(level, pos, mTrunkState, 0);
    }
}

void RoofTreeFeature::placeLeafAt(Level* level, int x, int y, int z)
{
    Block* block = level->getBlock(BlockPos(x, y, z));
    if (block == Blocks::AIR || block == Blocks::leaves || block == Blocks::leaves2)
    {
        setBlock(level, BlockPos(x, y, z), mLeafState, 0);
    }
}

bool RoofTreeFeature::place(Level* level, Random& random, const BlockPos& pos)
{
    PIXBeginNamedEvent(0.0f, "RoofTreeFeature::place");
    int height = random.nextInt(3) + random.nextInt(2) + mTreeType;
    if (pos.y <= 0 || pos.y + height + 1 >= 257)
    {
        PIXEndNamedEvent();
        return false;
    }

    Block* ground = level->getBlock(pos.below());
    if ((ground != Blocks::grass && ground != Blocks::dirt) ||
        pos.y >= 255 - height || !canPlaceTreeOfHeight(level, pos, height))
    {
        PIXEndNamedEvent();
        return false;
    }

    setDirtAt(level, pos.below());
    setDirtAt(level, pos.below().east());
    setDirtAt(level, pos.below().south());
    setDirtAt(level, pos.below().south().east());

    const Direction* direction = Direction::Plane::HORIZONTAL->getRandomDirection(random);
    Vec3i normal = direction->getNormal();
    int bendStart = height - random.nextInt(4);
    int bendLength = 2 - random.nextInt(3);
    int x = pos.x;
    int z = pos.z;
    int lastY = pos.y + height - 1;
    MutableBlockPos cursor(x, pos.y, z);

    for (int y = 0; y < height; ++y)
    {
        if (y >= bendStart && bendLength > 0)
        {
            x += normal.x;
            z += normal.z;
            --bendLength;
        }
        cursor.set(x, pos.y + y, z);
        if (level->getBlockState(cursor)->getBlock() == Blocks::AIR ||
            level->getBlockState(cursor)->getBlock() == Blocks::leaves ||
            level->getBlockState(cursor)->getBlock() == Blocks::leaves2)
        {
            placeLogAt(level, cursor);
            lastY = pos.y + y;
        }
        cursor.set(x + 1, pos.y + y, z);
        placeLogAt(level, cursor);
        cursor.set(x, pos.y + y, z + 1);
        placeLogAt(level, cursor);
        cursor.set(x + 1, pos.y + y, z + 1);
        placeLogAt(level, cursor);
    }

    for (int layer = 0; layer < 3; ++layer)
    {
        int left = x - 2 + layer;
        int right = x + 3 - layer;
        int y = lastY - 1;
        for (int dz = -2; dz <= 3; ++dz)
        {
            placeLeafAt(level, left, y, z + dz);
            placeLeafAt(level, right, y, z + dz);
            if (layer < 2)
            {
                placeLeafAt(level, left, lastY + 1, z + dz);
                placeLeafAt(level, right, lastY + 1, z + dz);
            }
        }
    }

    if (random.nextBoolean())
    {
        placeLeafAt(level, x, lastY + 2, z);
        placeLeafAt(level, x + 1, lastY + 2, z);
        placeLeafAt(level, x, lastY + 2, z + 1);
        placeLeafAt(level, x + 1, lastY + 2, z + 1);
    }

    for (int branch = -1; branch <= 2; ++branch)
    {
        int branchX = pos.x + branch;
        for (int branchZOffset = -1; branchZOffset <= 2; ++branchZOffset)
        {
            if (random.nextInt(3) < 1)
            {
                int branchLength = random.nextInt(3) + 2;
                for (int i = 0; i < branchLength; ++i)
                {
                    placeLogAt(level, BlockPos(branchX, lastY - 1 - i,
                                               pos.z + branchZOffset));
                }
                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dz = -1; dz <= 1; ++dz)
                    {
                        placeLeafAt(level, branchX + dx, lastY,
                                    pos.z + branchZOffset + dz);
                    }
                }
                for (int dx = -2; dx <= 2; ++dx)
                {
                    for (int dz = -2; dz <= 2; ++dz)
                    {
                        if ((dx != -2 && dx != 2) || (dz != -2 && dz != 2))
                        {
                            placeLeafAt(level, branchX + dx, lastY - 1,
                                        pos.z + branchZOffset + dz);
                        }
                    }
                }
            }
        }
    }

    PIXEndNamedEvent();
    return true;
}
