#ifndef GAME_CORE_EGGDISPENSEBEHAVIOR_HPP
#define GAME_CORE_EGGDISPENSEBEHAVIOR_HPP

#include "game/entity/projectile/AbstractProjectileDispenseBehavior.hpp"

class EggDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv
    : public AbstractProjectileDispenseBehavior
{
public:
    virtual ~EggDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv();

protected:
    virtual boost::shared_ptr<Projectile> getProjectile(
        Level* level, Position* position, not_null_ptr<ItemInstance> itemInstance);
};

#endif
