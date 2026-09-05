#include "game/world/generation/MegaJungleTreeFeature.hpp"

#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TypedBoxed.hpp"

static const BooleanProperty* megaJungleWest =
    static_cast<const BooleanProperty*>(BooleanProperty::create(L"west"));
static const BooleanProperty* megaJungleNorth =
    static_cast<const BooleanProperty*>(BooleanProperty::create(L"north"));
static const BooleanProperty* megaJungleSouth =
    static_cast<const BooleanProperty*>(BooleanProperty::create(L"south"));
static const BooleanProperty* megaJungleEast =
    static_cast<const BooleanProperty*>(BooleanProperty::create(L"east"));

MegaJungleTreeFeature::MegaJungleTreeFeature(bool doBlockNotify, int treeType, int height,
                                             const BlockState* trunkState,
                                             const BlockState* leafState)
    : MegaTreeFeature(doBlockNotify, treeType, height, trunkState, leafState)
{
}

MegaJungleTreeFeature::~MegaJungleTreeFeature()
{
}

void MegaJungleTreeFeature::createCrown(Level* level, const BlockPos& pos, int radius)
{
    placeDoubleTrunkLeaves(level, pos.above(), radius + 1);
    placeDoubleTrunkLeaves(level, pos.above(-1), radius + 2);
    placeDoubleTrunkLeaves(level, pos.above(-2), radius + 3);
}

void MegaJungleTreeFeature::placeVine(Level* level, Random& random, const BlockPos& pos,
                                      const BooleanProperty* property)
{
    if (random.nextInt(3) > 0 && level->isEmptyBlock(pos))
    {
        TypedBoxed<bool> value(true);
        const BlockState* state = Blocks::vine->defaultBlockState()->setBoxedValue(property, &value);
        setBlock(level, pos, state, 0);
    }
}

bool MegaJungleTreeFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    const int height = calcTreeHeigth(random);
    if (!prepareTree(level, random, origin, height))
    {
        return false;
    }

    createCrown(level, origin.above(height), 2);

    int top = origin.y + height - 2 - random.nextInt(4);
    MutableBlockPos leafPos(0, 0, 0);
    const int halfHeight = height / 2;
    if (origin.y + halfHeight < top)
    {
        do
        {
            const float angle = random.nextFloat() * 6.2831855f;
            const float cosine = Mth::cos(angle);
            const float sine = Mth::sin(angle);
            const int x1 = (int)(cosine * 3.0f + 1.5f) + origin.x;
            const int z1 = (int)(sine * 3.0f + 1.5f) + origin.z;
            leafPos.set(x1, top - 3, z1);
            setBlock(level, leafPos, mLeafState, 0);

            const int x2 = (int)(cosine * 1.75f + 1.5f) + origin.x;
            const int z2 = (int)(sine * 1.75f + 1.5f) + origin.z;
            leafPos.set(x2, top - 3, z2);
            setBlock(level, leafPos, mLeafState, 0);

            const int x3 = (int)(cosine * 2.0f + 1.5f) + origin.x;
            const int z3 = (int)(sine * 2.0f + 1.5f) + origin.z;
            leafPos.set(x3, top - 2, z3);
            setBlock(level, leafPos, mLeafState, 0);

            const int x4 = (int)(cosine * 3.0f + 1.5f) + origin.x;
            const int z4 = (int)(sine * 3.0f + 1.5f) + origin.z;
            leafPos.set(x4, top - 2, z4);
            setBlock(level, leafPos, mLeafState, 0);

            const int x5 = (int)(cosine * 4.0f + 1.5f) + origin.x;
            const int z5 = (int)(sine * 4.0f + 1.5f) + origin.z;
            leafPos.set(x5, top - 1, z5);

            const int first = top - (random.nextInt(2) + 1);
            for (int y = first; y <= top; ++y)
            {
                leafPos.set(x5, y, z5);
                placeSingleTrunkLeaves(level, leafPos, top - y + 1);
            }

            top -= random.nextInt(4) + 2;
        }
        while (origin.y + halfHeight < top);
    }

    for (int y = 0; y < height; ++y)
    {
        const BlockPos trunkPos = origin.above(y);
        if (isFree(level->getBlock(trunkPos)))
        {
            setBlock(level, trunkPos, mTrunkState, 0);
            if (y > 0)
            {
                placeVine(level, random, trunkPos.west(), megaJungleWest);
                placeVine(level, random, trunkPos.north(), megaJungleNorth);
            }
        }

        if (y < height - 1)
        {
            const BlockPos eastPos = trunkPos.east();
            if (isFree(level->getBlock(eastPos)))
            {
                setBlock(level, eastPos, mTrunkState, 0);
                if (y > 0)
                {
                    placeVine(level, random, eastPos.east(), megaJungleEast);
                    placeVine(level, random, eastPos.north(), megaJungleNorth);
                }
            }

            const BlockPos southEastPos = trunkPos.south().east();
            if (isFree(level->getBlock(southEastPos)))
            {
                setBlock(level, southEastPos, mTrunkState, 0);
                if (y > 0)
                {
                    placeVine(level, random, southEastPos.east(), megaJungleEast);
                    placeVine(level, random, southEastPos.south(), megaJungleSouth);
                }
            }

            const BlockPos southPos = trunkPos.south();
            if (isFree(level->getBlock(southPos)))
            {
                setBlock(level, southPos, mTrunkState, 0);
                if (y > 0)
                {
                    placeVine(level, random, southPos.west(), megaJungleWest);
                    placeVine(level, random, southPos.south(), megaJungleSouth);
                }
            }
        }
    }

    return true;
}
