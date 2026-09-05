#ifndef GAME_ENTITY_PROJECTILE_ABSTRACTROTATEDITEMPROJECTILE_HPP
#define GAME_ENTITY_PROJECTILE_ABSTRACTROTATEDITEMPROJECTILE_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/MoverType.hpp"

class CompoundTag;
class HitResult;
class Level;
class LivingEntity;
class Player;
class Random;
class Vec3;

class AbstractRotatedItemProjectile : public Entity
{
public:
    AbstractRotatedItemProjectile(Level* level);
    AbstractRotatedItemProjectile(Level* level, double x, double y, double z);
    AbstractRotatedItemProjectile(Level* level, boost::shared_ptr<LivingEntity> owner);
    virtual ~AbstractRotatedItemProjectile();

    virtual unsigned long long GetType() = 0;
    void shoot(double x, double y, double z, float velocity, float inaccuracy);
    void shootFromRotation(boost::shared_ptr<Entity> entity, float xRot, float yRot, float offset, float velocity, float inaccuracy);
    void setFlag(int flag, bool value);
    void setCritArrow(bool critical);
    bool onHit(const HitResult* result);
    static void staticCtor();
    void _init();
    virtual void defineSynchedData();
    unsigned int getOnHitSound() const;
    virtual void doPostHurtEffects(const boost::shared_ptr<LivingEntity>& entity);
    boost::shared_ptr<Entity> findHitEntity(Vec3* from, Vec3* to);
    virtual void checkDespawn();
    virtual bool makeStepSound();
    virtual double getWaterInertia();
    virtual bool canChangeDimensions();
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    bool isNoPhysics();
    bool isCritArrow();
    virtual void tick();
    virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport);
    virtual void lerpMotion(double xd, double yd, double zd);
    virtual void move(MoverType type, double x, double y, double z, bool pushedByPiston);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void playerTouch(boost::shared_ptr<Player> player);
    virtual bool isAttackable();
    virtual float getEyeHeight();
    void setNoPhysics(bool noPhysics);
    virtual bool shouldTick();

protected:
    int mXTile;
    int mYTile;
    int mZTile;
    int mInTile;
    short mLife;
    unsigned char mInData;
    unsigned char mShakeTime;
    bool mInGround;
    unsigned char mPickup;
    bool mCrit;
    int mOwnerId;
    bool mNoPhysicsFlag;
    int mSoundDelay;
    int mOwner;
};

#endif
