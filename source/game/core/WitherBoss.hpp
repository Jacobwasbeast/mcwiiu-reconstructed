#ifndef GAME_CORE_WITHERBOSS_HPP
#define GAME_CORE_WITHERBOSS_HPP

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/RangedAttackMob.hpp"
#include "game/entity/mob/BossMob.hpp"
#include "game/entity/ai/Goal.hpp"

class AABB;
class Block;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class Entity;
class FlyingPathNavigation;
class Level;
class LivingEntity;
class MobEffectInstance;
class MobGroupData;
class ServerBossEvent;
class ServerPlayer;
class SoundEvent;
class Vec3;
template <class T> class EntityDataAccessor;

class WitherBoss : public Monster, public RangedAttackMob, public BossMob
{
public:
    static const EntityDataAccessor<int>* DATA_TARGET_A;
    static const EntityDataAccessor<int>* DATA_TARGET_B;
    static const EntityDataAccessor<int>* DATA_TARGET_C;
    static const EntityDataAccessor<int>* DATA_TARGETS[3];
    static const EntityDataAccessor<int>* DATA_INVULNERABLE_TICKS;
    static const EntityDataAccessor<bool>* DATA_AERIAL_ATTACK;
    static const EntityDataAccessor<bool>* DATA_CHARGE_ATTACKING;

    class WitherDoNothingGoal : public Goal
    {
    public:
        explicit WitherDoNothingGoal(WitherBoss* wither);
        virtual ~WitherDoNothingGoal();
        virtual bool canUse();
        virtual std::wstring getName();

    private:
        WitherBoss* mWither;
    };

    explicit WitherBoss(Level* level);
    virtual ~WitherBoss();
    static void create(Level* level);
    virtual std::wstring getDisplayName();
    virtual void performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor);
    virtual bool isAggressive();
    virtual void setAggressive(bool aggressive);
    static bool canDestroy(Block* block);
    static void addDataWalkers(DataFixerUpper* fixer);
    static void staticCtor();
    virtual void defineSynchedData();
    virtual void registerAttributes();
    void setAerialAttack(bool aerialAttack);
    virtual void setCustomName(const std::wstring& name, const std::wstring& displayName, bool visible);
    bool getAerialAttack() const;
    virtual void addAdditonalSaveData(CompoundTag* tag);
    int getInvulnerableTicks();
    void setInvulnerableTicks(int ticks);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void readAdditionContructionTag(CompoundTag* tag);
    virtual CompoundTag* getAdditionContructionTag();
    int getAlternativeTarget(int head);
    bool isPowered();
    void generateParticles();
    bool IsChargeAttacking();
    bool canShoot();
    virtual void aiStep();
    virtual void makeInvulnerable();
    virtual void makeStuckInWeb();
    virtual void startSeenByPlayer(boost::shared_ptr<ServerPlayer> player);
    virtual void stopSeenByPlayer(boost::shared_ptr<ServerPlayer> player);
    std::map<boost::shared_ptr<ServerPlayer>, int>& getPlayerParty();
    virtual int getArmorValue();
    void changePhase();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual int getLightColor();
    virtual bool isPickable();
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual void addEffect(MobEffectInstance* effectInstance, const boost::shared_ptr<Entity>& source);
    void setAlternativeTarget(int head, int target);
    double getHeadYRot(int head);
    double getHeadXRot(int head);
    virtual EMobType getMobType();
    virtual void die(const DamageSource* source);
    virtual void handleEntityEvent(unsigned char event, int data);
    double getOverlayAlpha();
    double getSwellAmount(float partialTicks) const;
    bool wantsToMove();
    void setWantsToMove(bool wantsToMove);
    void setIsPathing(bool pathing);
    void setShotDelay(int delay);
    virtual void heal(float amount);
    virtual bool canFly();
    virtual boost::shared_ptr<LivingEntity> getTarget();
    bool IsSpawning() const;
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& dataString);
    virtual FlyingPathNavigation* createNavigation(Level* level);
    virtual void registerGoals();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    void destroyBlocks(Level* level, const AABB* box, int expansion);
    virtual void customServerAiStep();
    virtual void dropDeathLoot(bool hitByPlayer, int lootingLevel);
    virtual void checkDespawn();
    virtual bool canRide(boost::shared_ptr<Entity> entity);
    virtual bool canChangeDimensions();
    virtual double getHeadX(int head);
    virtual double getHeadY(int head);
    virtual double getHeadZ(int head);
    virtual float rotlerp(float from, float to, float maxDelta);
    virtual void performRangedAttack(int head, boost::shared_ptr<LivingEntity> target);
    virtual void performRangedAttack(int head, double x, double y, double z, bool charged);
    virtual float getMaxHealth();
    virtual float getHealth();
    virtual int getDimension();
    virtual unsigned long long GetType();

private:
    int mMaxShieldHealth;
    float mXRotHeads[2];
    float mYRotHeads[2];
    float mXRotOHeads[2];
    float mYRotOHeads[2];
    DamageSource* mLastDamageSource;
    int mPhase;
    int mShotDelay;
    int mStunTicks;
    int mDyingFrames;
    int mDeathFlapInterval;
    float mOverlayAlpha;
    float mSwellAmount;
    float mOldSwellAmount;
    int mChargeWindupTicks;
    bool mCharging;
    int mAttackCooldown;
    int mFirerate;
    int mAttackDelay;
    float mAttackRange;
    int mSpawningFrames;
    bool mDiving;
    int mReloadTicks;
    Vec3* mChargeDirection;
    bool mWantsToMove;
    int mChargeTicks;
    int mTargetTrackingTicks;
    int mShieldHealth;
    int mShotCount;
    int mReloadDuration;
    bool mSkipReload;
    int mMaxHealth;
    int mLastHealthInterval;
    float mSpawnSpin;
    int mNextHeadUpdate[2];
    int mIdleHeadUpdates[2];
    int mDestroyBlocksTick;
    int mHalfHealth;
    int mHealthInterval;
    bool mIsPathing;
    ServerBossEvent* mBossEvent;
};

#endif
