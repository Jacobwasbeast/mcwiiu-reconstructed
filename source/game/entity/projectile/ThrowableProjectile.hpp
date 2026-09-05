#ifndef GAME_ENTITY_PROJECTILE_THROWABLEPROJECTILE_HPP
#define GAME_ENTITY_PROJECTILE_THROWABLEPROJECTILE_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include "game/entity/mob/Entity.hpp"

class CompoundTag;
class DataFixerUpper;
class Level;
class LivingEntity;
class Vec3;

class ThrowableProjectile : public Entity
{
public:
    ThrowableProjectile(Level* level);
    ThrowableProjectile(Level* level, boost::shared_ptr<LivingEntity> owner);
    ThrowableProjectile(Level* level, double x, double y, double z);
    virtual ~ThrowableProjectile();
    virtual void shoot(double x, double y, double z, float velocity, float inaccuracy);
    virtual void shootFromRotation(boost::shared_ptr<Entity> entity, float xRot, float yRot, float offset, float velocity, float inaccuracy);
    void fjSetParticleSpawnPos(double x, double y, double z);
    static void addDataWalkers(DataFixerUpper* fixer, unsigned long long version);
    void _throwableInit();
    virtual void defineSynchedData();
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual void lerpMotion(double xd, double yd, double zd);
    virtual void tick();
    virtual double getGravity();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    boost::shared_ptr<LivingEntity> getOwner();
    Vec3* fjGetParticleSpawnPos() const;
    virtual bool shouldTick();
protected:
    boost::shared_ptr<LivingEntity> mOwner;
    Level* mProjectileLevel;
    int mXTile;
    int mYTile;
    int mZTile;
    int mInTile;
    unsigned char mShake;
    int mInGround;
    std::wstring mOwnerName;
    int mOwnerId;
    int mOwnerData;
    Vec3* mParticleSpawnPos;
};

#endif
