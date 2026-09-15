#include "game/core/ExpBottleDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"

#include "game/core/Position.hpp"
#include "game/core/ThrownExpBottle.hpp"

ExpBottleDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::~ExpBottleDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv()
{
}

double ExpBottleDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::getUncertainty()
{
    return AbstractProjectileDispenseBehavior::getUncertainty() * 0.5;
}

double ExpBottleDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::getPower()
{
    return AbstractProjectileDispenseBehavior::getPower() * 1.25;
}

boost::shared_ptr<Projectile>
ExpBottleDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::getProjectile(
    Level* level, Position* position, not_null_ptr<ItemInstance>)
{
    return boost::shared_ptr<Projectile>(new ThrownExpBottle(level, position->getX(),
                                                              position->getY(), position->getZ()));
}
