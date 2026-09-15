#include "game/core/PumpkinDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"

#include "game/item/ArmorItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/PumpkinBlock.hpp"

PumpkinDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::~PumpkinDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv()
{
}

not_null_ptr<ItemInstance> PumpkinDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::execute(
    BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
{
    Level* level = source->getWorld();
    const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
    BlockPos position = source->getPos().relative(direction);
    PumpkinBlock* block = static_cast<PumpkinBlock*>(Blocks::pumpkin);
    outcome = eOUTCOME_Success;
    if (level->isEmptyBlock(position) && block->canSpawnGolem(level, position))
    {
        if (!level->mIsClientSide)
            level->setBlock(position, block->defaultBlockState(), 3, false);
        item->shrink(1);
        outcome = eOUTCOME_Success;
        return item;
    }
    not_null_ptr<ItemInstance> armorResult = ArmorItem::dispenseArmor(source, item, outcome);
    if (!armorResult->isEmpty())
        return armorResult;
    return DefaultDispenseItemBehavior::execute(source, item, outcome);
}

void PumpkinDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::playSound(
    BlockSource* source, eOUTCOME outcome)
{
    source->getWorld()->levelEvent(outcome == eOUTCOME_Success ? 1000 : 1001,
                                   source->getPos(), 0);
}
