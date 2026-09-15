#include "game/core/SnowballDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"

#include "game/core/Position.hpp"
#include "game/core/Snowball.hpp"

SnowballDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::
    ~SnowballDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv()
{
}

boost::shared_ptr<Projectile>
SnowballDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::getProjectile(
    Level* level, Position* position, not_null_ptr<ItemInstance>)
{
    return boost::shared_ptr<Projectile>(
        new Snowball(level, position->getX(), position->getY(), position->getZ()));
}
