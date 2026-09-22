#include "game/core/FlintAndSteelDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TntBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

FlintAndSteelDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::~FlintAndSteelDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv()
{
}

not_null_ptr<ItemInstance>
FlintAndSteelDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::execute(
    BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
{
    Level* level = source->getWorld();
    const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
    BlockPos position = source->getPos().relative(direction);

    outcome = eOUTCOME_Success;
    if (level->isEmptyBlock(position))
    {
        level->setBlockAndUpdate(position, Blocks::FIRE->defaultBlockState());
        if (item->hurt(1, level->mRandom))
            item->setCount(0);
    }
    else if (level->getBlockState(position)->getBlock() == Blocks::tnt)
    {
        TypedBoxed<bool> explode(true);
        const BlockState* state = Blocks::tnt->defaultBlockState()->setBoxedValue(TntBlock::EXPLODE, &explode);
        Blocks::tnt->onPlace(level, position, state);
        level->removeBlock(position);
    }
    else
    {
        outcome = eOUTCOME_Failure;
    }

    return item;
}
