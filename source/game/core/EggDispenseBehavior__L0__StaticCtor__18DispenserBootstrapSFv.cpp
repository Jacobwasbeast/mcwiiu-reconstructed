#include "game/core/EggDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"

#include "game/core/Position.hpp"
#include "game/core/ThrownEgg.hpp"

EggDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::~EggDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv()
{
}

boost::shared_ptr<Projectile>
EggDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::getProjectile(
    Level* level, Position* position, not_null_ptr<ItemInstance>)
{
    return boost::shared_ptr<Projectile>(
        new ThrownEgg(level, position->getX(), position->getY(), position->getZ()));
}
