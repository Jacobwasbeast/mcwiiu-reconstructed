#include "game/world/generation/SavannaTreeFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/NewLeafBlock.hpp"
#include "game/world/block/NewLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include <cstdlib>

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

static const BlockState* savannaTrunkState = NULL;
static const BlockState* savannaLeafState = NULL;

SavannaTreeFeature::SavannaTreeFeature(bool doBlockNotify)
    : AbstractTreeFeature(doBlockNotify, 5,
                         savannaTrunkState, savannaLeafState, false)
{
}

SavannaTreeFeature::~SavannaTreeFeature()
{
}

void SavannaTreeFeature::staticCtor()
{
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(4));
    savannaTrunkState = Blocks::log2->defaultBlockState()->setBoxedValue(
        NewLogBlock::VARIANT, &variant);
    savannaLeafState = Blocks::leaves2->defaultBlockState()->setBoxedValue(
        NewLeafBlock::VARIANT, &variant);
    TypedBoxed<bool> decayable(false);
    savannaLeafState = savannaLeafState->setBoxedValue(
        NewLeafBlock::DECAYABLE, &decayable);
}

void SavannaTreeFeature::placeLogAt(Level* level, const BlockPos& pos)
{
    setBlock(level, pos, mTrunkState, 0);
}

void SavannaTreeFeature::placeLeafAt(Level* level, const BlockPos& pos)
{
    Block* block = level->getBlockState(pos)->getBlock();
    if (block == Blocks::leaves || block == Blocks::leaves2)
    {
        setBlock(level, pos, mLeafState, 0);
    }
}

bool SavannaTreeFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    PIXBeginNamedEvent(0.0f, "SavannaTreeFeature::place");
    int height = random.nextInt(3) + random.nextInt(3) + mTreeType;
    if (origin.y <= 0 || origin.y + height + 1 >= 256)
    {
        PIXEndNamedEvent();
        return false;
    }

    for (int y = origin.y; y <= origin.y + height + 1; ++y)
    {
        int radius = y == origin.y ? 0 : (y < origin.y + height - 1 ? 1 : 2);
        for (int x = origin.x - radius; x <= origin.x + radius; ++x)
        {
            for (int z = origin.z - radius; z <= origin.z + radius; ++z)
            {
                if (!isFree(level->getBlockState(BlockPos(x, y, z))->getBlock()))
                {
                    PIXEndNamedEvent();
                    return false;
                }
            }
        }
    }

    BlockPos below = origin.below();
    Block* belowBlock = level->getBlockState(below)->getBlock();
    if (belowBlock != Blocks::grass && belowBlock != Blocks::dirt)
    {
        PIXEndNamedEvent();
        return false;
    }
    setDirtAt(level, below);

    const Direction* firstDirection = Direction::Plane::HORIZONTAL->getRandomDirection(random);
    const Direction* secondDirection = Direction::Plane::HORIZONTAL->getRandomDirection(random);
    Vec3i firstNormal = firstDirection->getNormal();
    int bendStart = height - random.nextInt(4) - 1;
    int bendLength = 3 - random.nextInt(3);
    int x = origin.x;
    int z = origin.z;
    int lastY = origin.y;
    for (int y = 0; y < height; ++y)
    {
        if (y >= bendStart && bendLength > 0)
        {
            x += firstNormal.x;
            z += firstNormal.z;
            --bendLength;
        }
        BlockPos trunk(x, origin.y + y, z);
        if (isFree(level->getBlockState(trunk)->getBlock()))
        {
            placeLogAt(level, trunk);
            lastY = origin.y + y;
        }
    }

    BlockPos canopy(x, lastY, z);
    for (int dx = -3; dx <= 3; ++dx)
    {
        for (int dz = -3; dz <= 3; ++dz)
        {
            if (std::abs(dx) != 3 || std::abs(dz) != 3)
            {
                placeLeafAt(level, canopy.offset(dx, 0, dz));
            }
        }
    }
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dz = -1; dz <= 1; ++dz)
        {
            placeLeafAt(level, canopy.above().offset(dx, 0, dz));
        }
    }
    for (int y = -1; y <= 1; ++y)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dz = -1; dz <= 1; ++dz)
            {
                placeLeafAt(level, canopy.offset(dx, y, dz));
            }
        }
    }

    Vec3i secondNormal = secondDirection->getNormal();
    if (secondDirection != firstDirection)
    {
        int branchStart = bendStart - random.nextInt(2) - 1;
        int branchY = origin.y + branchStart;
        int branchLength = random.nextInt(3) + 1;
        int branchX = origin.x;
        int branchZ = origin.z;
        int branchTop = 0;
        for (int i = 0; i < branchLength && branchY + i < origin.y + height; ++i)
        {
            if (i > 0)
            {
                branchX += secondNormal.x;
                branchZ += secondNormal.z;
            }
            BlockPos branch(branchX, branchY + i, branchZ);
            if (isFree(level->getBlockState(branch)->getBlock()))
            {
                placeLogAt(level, branch);
                branchTop = branchY + i;
            }
        }
        if (branchTop > 0)
        {
            BlockPos branchCanopy(branchX, branchTop, branchZ);
            for (int dx = -2; dx <= 2; ++dx)
            {
                for (int dz = -2; dz <= 2; ++dz)
                {
                    if (std::abs(dx) != 2 || std::abs(dz) != 2)
                    {
                        placeLeafAt(level, branchCanopy.offset(dx, 0, dz));
                    }
                }
            }
            for (int dx = -1; dx <= 1; ++dx)
            {
                for (int dz = -1; dz <= 1; ++dz)
                {
                    placeLeafAt(level, branchCanopy.above().offset(dx, 0, dz));
                }
            }
        }
    }
    PIXEndNamedEvent();
    return true;
}
