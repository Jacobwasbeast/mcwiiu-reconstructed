#include "game/world/generation/SeaAnemoneFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

class Property;

extern AbstractProperty<int>* CoralFan_PROPERTY_1;
extern AbstractProperty<int>* CoralFan_PROPERTY_2;
extern Property* Coral_VARIANT;

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

    if (level->isEmptyBlock(pos)
        && level->isSolidBlockingBlock(pos.below())
        && pos.y > 48
        && level->isUnderWater(pos, 259, 0)
        && level->reallyHasChunkAt(pos))
    {
        if (!random.rollPercentageChance(75))
        {
            const BlockState* state = Blocks::coral_fan->defaultBlockState();
            state = state->setBoxedValue(CoralFan_PROPERTY_1,
                                         new TypedBoxed<int>(random.nextInt(CoralFan_PROPERTY_1->getValueCount())));
            state = state->setBoxedValue(CoralFan_PROPERTY_2,
                                         new TypedBoxed<int>(random.nextInt(CoralFan_PROPERTY_2->getValueCount())));
            setBlock(level, pos, state, 0);
            return true;
        }

        const BlockState* state = Blocks::coral->defaultBlockState();
        state = state->setBoxedValue(Coral_VARIANT,
                                     new TypedBoxed<int>(random.nextInt(Coral_VARIANT->getValueCount())));
        setBlock(level, pos, state, 0);
        return true;
    }

    return false;
}
