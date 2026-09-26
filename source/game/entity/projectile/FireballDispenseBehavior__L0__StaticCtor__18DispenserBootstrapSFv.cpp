#include "game/entity/projectile/FireballDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"
#include "game/world/LevelEvent.hpp"

#include "game/core/Direction.hpp"
#include "game/core/Position.hpp"
#include "game/entity/projectile/SmallFireball.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"

FireballDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::~FireballDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv()
{
}

not_null_ptr<ItemInstance> FireballDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::execute(
    BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
{
    Level* level = source->getWorld();
    if (level->countInstanceOf(0ULL, false, 0, 0) >= 200)
        return DefaultDispenseItemBehavior::execute(source, item, outcome);

    Position* position = DispenserBlock::getDispensePosition(source);
    const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
    const double x = position->getX();
    const double y = position->getY();
    const double z = position->getZ();
    const double dx = direction->getStepX();
    const double dy = direction->getStepY();
    const double dz = direction->getStepZ();
    level->mRandom->nextGaussian();
    const double vx = level->mRandom->nextGaussian() * 0.05 + dx;
    const double vy = level->mRandom->nextGaussian() * 0.05 + dy;
    const double vz = level->mRandom->nextGaussian() * 0.05 + dz;
    boost::shared_ptr<SmallFireball> fireball(new SmallFireball(level, x + dx * 0.3,
                                                                  y + dy * 0.3,
                                                                  z + dz * 0.3,
                                                                  vx, vy, vz));
    level->addEntity(fireball);
    delete position;
    item->shrink(1);
    outcome = eOUTCOME_Success;
    return item;
}

void FireballDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::playSound(
    BlockSource* source, eOUTCOME outcome)
{
    if (outcome == eOUTCOME_Success)
        source->getWorld()->levelEvent(LevelEvent::SOUND_BLAZE_FIREBALL, source->getPos(), 0);
    else
        DefaultDispenseItemBehavior::playSound(source, outcome);
}
