#include "game/world/generation/DesertWellFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStatePredicate.hpp"
#include "game/world/block/Blocks.hpp"

static Predicate<const BlockState*>* gDesertWellSandPredicate = NULL;

DesertWellFeature::DesertWellFeature()
    : Feature(false),
      mSlabState(Blocks::stone_slab->defaultBlockState()),
      mSandstoneState(Blocks::sandstone->defaultBlockState()),
      mWaterState(Blocks::WATER->defaultBlockState())
{
}

DesertWellFeature::~DesertWellFeature()
{
}

void DesertWellFeature::staticCtor()
{
    gDesertWellSandPredicate = BlockStatePredicate::forBlock(Blocks::sand);
}

bool DesertWellFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    (void)random;

    BlockPos position(origin);
    while (level->isEmptyBlock(position) && position.y > 2)
    {
        position = position.below();
    }

    if (!gDesertWellSandPredicate->apply(level->getBlockState(position)))
    {
        return false;
    }

    for (int z = -2; z <= 2; ++z)
    {
        for (int x = -2; x <= 2; ++x)
        {
            if (!level->isEmptyBlock(position.offset(x, -1, z))
                && !level->isEmptyBlock(position.offset(-2, -1, z)))
            {
                return false;
            }
        }
    }

    for (int y = -1; y <= 0; ++y)
    {
        for (int x = -2; x <= 2; ++x)
        {
            for (int z = -2; z <= 2; ++z)
            {
                setBlock(level, position.offset(x, y, z), mSandstoneState, 0);
            }
        }
    }

    setBlock(level, position, mWaterState, 0);

    arrayWithLength<const Direction*> faces = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < faces.length; ++i)
    {
        setBlock(level, position.relative(faces.data[i]), mWaterState, 0);
    }

    for (int x = -2; x <= 2; ++x)
    {
        for (int z = -2; z <= 2; ++z)
        {
            if (x == -2 || x == 2 || z == -2 || z == 2)
            {
                setBlock(level, position.offset(x, 1, z), mSandstoneState, 0);
            }
        }
    }

    setBlock(level, position.offset(2, 1, 0), mSlabState, 0);
    setBlock(level, position.offset(-2, 1, 0), mSlabState, 0);
    setBlock(level, position.offset(0, 1, 2), mSlabState, 0);
    setBlock(level, position.offset(0, 1, -2), mSlabState, 0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int z = -1; z <= 1; ++z)
        {
            if (x == -1 || x == 1 || z == -1 || z == 1)
            {
                setBlock(level, position.offset(x, 4, z), mSandstoneState, 0);
            }
        }
    }

    setBlock(level, position.offset(0, 4, 0), mWaterState, 0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int z = -1; z <= 1; ++z)
        {
            if ((x == -1 || x == 1) && (z == -1 || z == 1))
            {
                setBlock(level, position.offset(x, 2, z), mSandstoneState, 0);
                setBlock(level, position.offset(x, 3, z), mSandstoneState, 0);
            }
        }
    }

    return true;
}
