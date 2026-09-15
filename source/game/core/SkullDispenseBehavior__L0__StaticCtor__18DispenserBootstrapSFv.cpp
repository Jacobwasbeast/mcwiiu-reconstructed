#include "game/core/SkullDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/SkullBlock.hpp"
#include "game/world/block/SkullBlockEntity.hpp"
#include "game/world/block/TypedBoxed.hpp"

SkullDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::~SkullDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv()
{
}

not_null_ptr<ItemInstance>
SkullDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::execute(
    BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
{
    Level* level = source->getWorld();
    const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
    BlockPos position = source->getPos().relative(direction);
    SkullBlock* block = static_cast<SkullBlock*>(Blocks::skull);
    if (level->isEmptyBlock(position) && block->canSpawnMob(level, position, item))
    {
        TypedBoxed<const Direction*> facing(direction);
        const BlockState* state = block->defaultBlockState()->setBoxedValue(SkullBlock::FACING, &facing);
        level->setBlock(position, state, 3, false);
        boost::shared_ptr<SkullBlockEntity> skull =
            boost::static_pointer_cast<SkullBlockEntity>(level->getBlockEntity(position));
        skull->setSkullType(item->getAuxValue(), L"");
        skull->setRotation(direction->getOpposite()->get2DDataValue() * 4);
        block->checkMobSpawn(level, position, skull, boost::shared_ptr<Player>());
        item->shrink(1);
        outcome = eOUTCOME_Success;
        return item;
    }

    not_null_ptr<ItemInstance> armorResult = ArmorItem::dispenseArmor(source, item, outcome);
    if (!armorResult->isEmpty())
        return armorResult;
    return DefaultDispenseItemBehavior::execute(source, item, outcome);
}
