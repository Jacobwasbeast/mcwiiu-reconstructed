#ifndef GAME_CORE_EXPBOTTLEDISPENSEBEHAVIOR_L0_STATICCTOR_HPP
#define GAME_CORE_EXPBOTTLEDISPENSEBEHAVIOR_L0_STATICCTOR_HPP

#include "game/entity/projectile/AbstractProjectileDispenseBehavior.hpp"

class ExpBottleDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv
    : public AbstractProjectileDispenseBehavior
{
public:
    virtual ~ExpBottleDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv();
    virtual double getUncertainty();
    virtual double getPower();

protected:
    virtual boost::shared_ptr<Projectile> getProjectile(Level* level, Position* position,
                                                         not_null_ptr<ItemInstance> itemInstance);
};

#endif
