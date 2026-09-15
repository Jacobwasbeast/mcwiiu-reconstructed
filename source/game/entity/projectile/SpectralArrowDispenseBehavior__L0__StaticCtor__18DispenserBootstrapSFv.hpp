#ifndef GAME_ENTITY_PROJECTILE_SPECTRAL_ARROW_DISPENSE_BEHAVIOR_HPP
#define GAME_ENTITY_PROJECTILE_SPECTRAL_ARROW_DISPENSE_BEHAVIOR_HPP

#include <boost/shared_ptr.hpp>
#include "game/entity/projectile/AbstractProjectileDispenseBehavior.hpp"

class Level;
class Position;
class Projectile;
class ItemInstance;

class SpectralArrowDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv
    : public AbstractProjectileDispenseBehavior
{
public:
    virtual ~SpectralArrowDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv();
protected:
    virtual boost::shared_ptr<Projectile> getProjectile(
        Level* level, Position* position, not_null_ptr<ItemInstance> itemInstance);
};

#endif
