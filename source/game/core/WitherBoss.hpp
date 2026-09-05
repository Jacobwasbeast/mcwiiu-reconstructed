#ifndef GAME_CORE_WITHERBOSS_HPP
#define GAME_CORE_WITHERBOSS_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Monster.hpp"
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

class WitherBoss : public Monster
{
public:
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
    virtual void performRangedAttack(boost::shared_ptr<LivingEntity> target, float damage);
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
    void readAdditionContructionTag(CompoundTag* tag);
    virtual CompoundTag* getAdditionContructionTag();
    int getAlternativeTarget(int head);
    bool isPowered();
    void generateParticles();
    bool IsChargeAttacking();
    bool canShoot();
    virtual void aiStep();
    bool makeInvulnerable();
    virtual void makeStuckInWeb();
    void startSeenByPlayer(boost::shared_ptr<ServerPlayer> player);
    void stopSeenByPlayer(boost::shared_ptr<ServerPlayer> player);
    void getPlayerParty();
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
    virtual double getSwellAmount(float partialTicks) const;
    bool wantsToMove();
    void setWantsToMove(bool wantsToMove);
    void setIsPathing(bool pathing);
    void setShotDelay(int delay);
    virtual void heal(float amount);
    bool canFly();
    virtual boost::shared_ptr<LivingEntity> getTarget();
    bool IsSpawning() const;
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& dataString);
    virtual FlyingPathNavigation* createNavigation(Level* level);
    virtual void registerGoals();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    bool destroyBlocks(Level* level, const AABB* box, int y);
    virtual void customServerAiStep();
    virtual void dropDeathLoot(bool hitByPlayer, int lootingLevel);
    virtual void checkDespawn();
    virtual bool canRide(boost::shared_ptr<Entity> entity);
    virtual bool canChangeDimensions();
    double getHeadX(int head);
    double getHeadY(int head);
    double getHeadZ(int head);
    float rotlerp(float from, float to, float max);
    void performRangedAttack(int head, boost::shared_ptr<LivingEntity> target);
    void performRangedAttack(int head, double x, double y, double z, bool charged);
    virtual float getMaxHealth();
    virtual float getHealth();
    virtual unsigned int getDimension();
    virtual unsigned long long GetType();

private:
    float mHeadRotations[3];
    float mSwellAmount;
    float mOldSwellAmount;
    int mAlternativeTargets[3];
    int mNextHeadUpdate[3];
    int mIdleHeadUpdates[3];
    int mInvulnerableTicks;
    int mDestroyBlocksTick;
    int mShotDelay;
    int mPhase;
    int mShieldHealth;
    int mMaxHealth;
    bool mAerialAttack;
    bool mWantsToMove;
    bool mIsPathing;
    bool mPowered;
    bool mSpawning;
    ServerBossEvent* mBossEvent;
    DamageSource* mLastDamageSource;
};

#endif
