#include "game/core/DyeDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"

#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/item/DyeItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"

DyeDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::~DyeDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv()
{
}

not_null_ptr<ItemInstance> DyeDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::execute(
    BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
{
    outcome = eOUTCOME_Success;
    if (DyeColor::byItemData(item->getAuxValue()) != DyeColor::WHITE)
        return DefaultDispenseItemBehavior::execute(source, item, outcome);

    Level* level = source->getWorld();
    const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
    BlockPos position = source->getPos().relative(direction);
    if (DyeItem::growCrop(item, level, position, false))
    {
        if (!level->mIsClientSide)
            level->levelEvent(2005, position, 0);
    }
    else
    {
        outcome = eOUTCOME_Failure;
    }
    return item;
}
