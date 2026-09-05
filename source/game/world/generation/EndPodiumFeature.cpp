#include "game/world/generation/EndPodiumFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/DirectionProperty.hpp"

namespace EndPortalFrameBlock
{
    extern DirectionProperty* FACING;
}

EndPodiumFeature::EndPodiumFeature(bool active)
    : Feature(false),
      mActive(active)
{
}

EndPodiumFeature::~EndPodiumFeature()
{
}

bool EndPodiumFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    (void)random;

    BlockPos from(origin.x - 4, origin.y - 1, origin.z - 4);
    BlockPos to(origin.x + 4, origin.y + 32, origin.z + 4);
    Iterator<MutableBlockPos*>* positions = BlockPos::mutablesBetween(from, to);

    while (positions->hasNext())
    {
        MutableBlockPos* position = positions->next();
        double distance = position->dist(origin.x, position->y, origin.z);

        if (distance > 3.5)
        {
            continue;
        }

        if (position->y < origin.y)
        {
            setBlock(level, *position,
                     distance <= 2.5 ? Blocks::end_stone->defaultBlockState()
                                     : Blocks::bedrock->defaultBlockState(), 0);
        }
        else if (position->y > origin.y)
        {
            setBlock(level, *position, Blocks::AIR->defaultBlockState(), 0);
        }
        else if (distance > 2.5)
        {
            setBlock(level, *position, Blocks::end_stone->defaultBlockState(), 0);
        }
        else if (mActive)
        {
            setBlock(level, *position, Blocks::end_portal->defaultBlockState(), 0);
        }
        else
        {
            setBlock(level, *position, Blocks::AIR->defaultBlockState(), 0);
        }
    }

    delete positions;

    for (int height = 0; height < 4; ++height)
    {
        setBlock(level, origin.above(height), Blocks::AIR->defaultBlockState(), 0);
    }

    BlockPos framePosition = origin.above(2);
    arrayWithLength<const Direction*> faces = Direction::Plane::HORIZONTAL->getFaces();
    for (unsigned int i = 0; i < faces.length; ++i)
    {
        const Direction* direction = faces.data[i];
        TypedBoxed<const Direction*> facing(direction);
        setBlock(level, framePosition.relative(direction),
                 Blocks::end_portal_frame->defaultBlockState()->setBoxedValue(
                     EndPortalFrameBlock::FACING, &facing), 0);
    }

    return true;
}
