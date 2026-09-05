#ifndef GAME_CORE_WITHERSKULL_HPP
#define GAME_CORE_WITHERSKULL_HPP

#include <vector>
#include <boost/shared_ptr.hpp>
#include "game/entity/projectile/Fireball.hpp"

class BlockPos;
class DataFixerUpper;
class Explosion;
class HitResult;
class Level;
class LivingEntity;
class MobEffectInstance;

class WitherSkull : public Fireball
{
public:
    WitherSkull(Level* level);
    WitherSkull(Level* level, boost::shared_ptr<LivingEntity> owner, double xd, double yd, double zd);
    WitherSkull(Level* level, double x, double y, double z, double xd, double yd, double zd);
    virtual ~WitherSkull();
    static void create(Level* level);
    static void staticCtor();
    virtual bool isOnFire();
    virtual float getBlockExplosionResistance(Explosion* explosion, Level* level, const BlockPos& pos);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual bool isPickable();
    virtual bool isDangerous();
    void setDangerous(bool dangerous);
    virtual double getInertia();
    virtual void onHit(const HitResult* result);
    virtual void defineSynchedData();
    virtual bool shouldBurn();
    virtual std::vector<unsigned long long>* getEntityTypesToIgnoreCollisionsWith();
    virtual unsigned long long GetType();
};

#endif
