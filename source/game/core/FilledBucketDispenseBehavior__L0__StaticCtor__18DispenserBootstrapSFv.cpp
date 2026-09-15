#include "game/core/FilledBucketDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"

#include <boost/shared_ptr.hpp>

#include "game/item/BucketItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DispenserBlock.hpp"

FilledBucketDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::~FilledBucketDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv()
{
}

not_null_ptr<ItemInstance>
FilledBucketDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::execute(
    BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
{
    Level* level = source->getWorld();
    const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
    BlockPos position = source->getPos().relative(direction);
    boost::shared_ptr<Player> player =
        level->getNearestPlayer(source->getX(), source->getY(), source->getZ(), 128.0, false, -1.0);

    if (!static_cast<BucketItem*>(item->getItem())->emptyBucket(player, level, position, item))
    {
        return DefaultDispenseItemBehavior::dispense(source, item);
    }

    outcome = eOUTCOME_Success;
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::bucket));
}
