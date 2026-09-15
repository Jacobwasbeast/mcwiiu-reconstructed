#include "game/core/EmptyBucketDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"

#include <boost/shared_ptr.hpp>

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"

EmptyBucketDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::~EmptyBucketDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv()
{
}

not_null_ptr<ItemInstance>
EmptyBucketDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::execute(
    BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
{
    Level* level = source->getWorld();
    const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
    BlockPos position = source->getPos().relative(direction);
    const BlockState* state = level->getBlockState(position);
    bool primaryLiquid = state->getBlock()->isLiquidBlock();
    if (!primaryLiquid)
    {
        state = level->getExtraBlockState(position);
    }
    Item* filled = NULL;

    if (state->getMaterial() == Material::WATER && state->getBlock()->isLiquidBlock()
        && state->getValue(LiquidBlock::LEVEL) == 0)
    {
        filled = Items::waterBucket;
    }
    else if (state->getMaterial() == Material::LAVA && state->getBlock()->isLiquidBlock()
             && state->getValue(LiquidBlock::LEVEL) == 0)
    {
        filled = Items::lavaBucket;
    }
    else
    {
        return DefaultDispenseItemBehavior::execute(source, item, outcome);
    }

    if (primaryLiquid)
    {
        level->removeBlock(position);
    }
    else
    {
        level->setBlock(position, Blocks::AIR->defaultBlockState(), 3, false);
    }
    item->shrink(1);

    not_null_ptr<ItemInstance> result(new ItemInstance(filled));
    if (item->isEmpty())
    {
        return result;
    }

    boost::shared_ptr<DispenserBlockEntity> dispenser =
        boost::static_pointer_cast<DispenserBlockEntity>(source->getEntity());
    if (dispenser->addItem(result) < 0)
    {
        DefaultDispenseItemBehavior::dispense(source, result);
    }
    outcome = eOUTCOME_Success;
    return item;
}
