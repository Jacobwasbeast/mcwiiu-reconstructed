#ifndef GAME_CORE_SNOWBALLDISPENSEBEHAVIOR_HPP
#define GAME_CORE_SNOWBALLDISPENSEBEHAVIOR_HPP

#include "game/entity/projectile/AbstractProjectileDispenseBehavior.hpp"

class SnowballDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv
    : public AbstractProjectileDispenseBehavior
{
public:
    virtual ~SnowballDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv();

protected:
    virtual boost::shared_ptr<Projectile> getProjectile(
        Level* level, Position* position, not_null_ptr<ItemInstance> itemInstance);
};

#endif
