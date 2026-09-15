#include "game/core/FireworksDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"

#include "game/core/Direction.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"

FireworksDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::~FireworksDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv()
{
}

not_null_ptr<ItemInstance> FireworksDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::execute(
    BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
{
    Level* level = source->getWorld();
    if (level->countInstanceOf(0ULL, false, 0, 0) >= 300)
    {
        outcome = eOUTCOME_Failure;
        return item;
    }

    const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
    const BlockPos blockPos = source->getPos();
    const double x = source->getX() + direction->getStepX();
    const float y = static_cast<float>(blockPos.y + direction->getStepY()) + 0.2f;
    const double z = source->getZ() + direction->getStepZ();
    boost::shared_ptr<FireworksRocketEntity> rocket(new FireworksRocketEntity(
        level, x, y, z, item));
    level->addEntity(rocket);
    outcome = eOUTCOME_Success;
    item->shrink(1);
    return item;
}

void FireworksDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::playSound(
    BlockSource* source, eOUTCOME outcome)
{
    if (outcome == eOUTCOME_Success)
        source->getWorld()->levelEvent(1004, source->getPos(), 0);
    else
        DefaultDispenseItemBehavior::playSound(source, outcome);
}
