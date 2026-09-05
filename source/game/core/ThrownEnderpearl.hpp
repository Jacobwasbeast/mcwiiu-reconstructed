#ifndef GAME_CORE_THROWNENDERPEARL_HPP
#define GAME_CORE_THROWNENDERPEARL_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/ThrowableProjectile.hpp"

class DataFixerUpper;
class HitResult;
class Level;

class ThrownEnderpearl : public ThrowableProjectile
{
public:
    ThrownEnderpearl(Level* level);
    ThrownEnderpearl(Level* level, boost::shared_ptr<LivingEntity> owner);
    ThrownEnderpearl(Level* level, double x, double y, double z);
    virtual ~ThrownEnderpearl();

    static void create(Level* level);
    void PlayTeleportSound();
    virtual bool onHit(const HitResult* result);
    static void addDataWalkers(DataFixerUpper* fixer);
    void _init();
    virtual void tick();
    virtual unsigned long long GetType();

private:
    boost::shared_ptr<Entity> mUnknown;
};

#endif
