#include "game/entity/projectile/SpectralArrowDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"

#include "game/core/Position.hpp"
#include "game/entity/projectile/SpectralArrow.hpp"

SpectralArrowDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::~SpectralArrowDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv()
{
}

boost::shared_ptr<Projectile>
SpectralArrowDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::getProjectile(
    Level* level, Position* position, not_null_ptr<ItemInstance>)
{
    return boost::shared_ptr<Projectile>(new SpectralArrow(
        level, position->getX(), position->getY(), position->getZ()));
}
