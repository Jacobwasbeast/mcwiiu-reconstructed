#include "game/world/generation/SeaAnemoneFeature.hpp"

#include "game/core/CoralFan.hpp"
#include "game/core/Coral.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

class Property;


SeaAnemoneFeature::~SeaAnemoneFeature()
{
}

bool SeaAnemoneFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (level->mIsClientSide)
    {
        return true;
    }

    BlockPos pos = origin;
    while (!level->isSolidBlockingBlock(pos.below()) && pos.y >= 49)
    {
        pos = pos.below();
    }

    if (level->isEmptyWaterBlock(pos)
        && level->isSolidBlockingBlock(pos.below())
        && pos.y > 48
        && level->isUnderWater(pos, 259, 0)
        && level->reallyHasChunkAt(pos))
    {
        if (!random.rollPercentageChance(75))
        {
            const BlockState* state = Blocks::coral_fan->defaultBlockState();
            TypedBoxed<int> variant(random.nextInt(CoralFan::VARIANT->getValueCount()));
            state = state->setBoxedValue(CoralFan::VARIANT, &variant);
            TypedBoxed<int> direction(random.nextInt(CoralFan::DIRECTION->getValueCount()));
            state = state->setBoxedValue(CoralFan::DIRECTION, &direction);
            setBlock(level, pos, state, Block::UPDATE_NONE);
            return true;
        }

        const BlockState* state = Blocks::coral->defaultBlockState();
        TypedBoxed<int> variant(random.nextInt(Coral::VARIANT->getValueCount()));
        state = state->setBoxedValue(Coral::VARIANT, &variant);
        setBlock(level, pos, state, Block::UPDATE_NONE);
        return true;
    }

    return false;
}
