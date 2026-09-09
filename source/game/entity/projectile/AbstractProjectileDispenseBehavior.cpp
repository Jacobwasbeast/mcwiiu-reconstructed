#include "game/entity/projectile/AbstractProjectileDispenseBehavior.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/Position.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/projectile/Projectile.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DispenserBlock.hpp"

AbstractProjectileDispenseBehavior::~AbstractProjectileDispenseBehavior()
{
}

not_null_ptr<ItemInstance> AbstractProjectileDispenseBehavior::execute(
    BlockSource* blockSource, not_null_ptr<ItemInstance> itemInstance,
    DefaultDispenseItemBehavior::eOUTCOME& outcome)
{
    Level* level = blockSource->getWorld();

    if (level->countInstanceOf(0ULL, false, NULL, NULL) < 300)
    {
        Position* position = DispenserBlock::getDispensePosition(blockSource);
        const Direction* direction = blockSource->getBlockState()->getValue(DispenserBlock::FACING);
        boost::shared_ptr<Projectile> projectile = getProjectile(level, position, itemInstance);
        const Vec3i normal = direction->getNormal();

        projectile->shoot((double)normal.x, (double)normal.y + 0.1, (double)normal.z,
                          (float)getPower(), (float)getUncertainty());
        boost::shared_ptr<Entity> entity = boost::dynamic_pointer_cast<Entity>(projectile);
        level->addEntity(entity);

        delete position;
        itemInstance->split(1);
    }
    else
    {
        return DefaultDispenseItemBehavior::execute(blockSource, itemInstance, outcome);
    }

    return itemInstance;
}

void AbstractProjectileDispenseBehavior::playSound(
    BlockSource* blockSource, DefaultDispenseItemBehavior::eOUTCOME outcome)
{
    if (outcome != DefaultDispenseItemBehavior::eOUTCOME_Failure)
    {
        blockSource->getWorld()->levelEvent(1002, blockSource->getPos(), 0);
    }
}

double AbstractProjectileDispenseBehavior::getUncertainty()
{
    return 6.0;
}

double AbstractProjectileDispenseBehavior::getPower()
{
    return 1.1;
}
