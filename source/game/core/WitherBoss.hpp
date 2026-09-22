#ifndef GAME_CORE_WITHERBOSS_HPP
#define GAME_CORE_WITHERBOSS_HPP

#include <string>
#include "game/entity/mob/Monster.hpp"
#include "game/entity/ai/Goal.hpp"

class Block;
class AABB;
class LivingEntity;
class CompoundTag;
class ServerPlayer;
class DifficultyInstance;
class MobGroupData;
class PathNavigation;
class ServerBossEvent;
class WitherBoss : public Monster
{
public:
    static const unsigned long long ENTITY_TYPE;
    void readAdditionContructionTag(CompoundTag* tag);
    int getAdditionContructionTag() const;
    class WitherDoNothingGoal : public Goal
    {
    public:
        explicit WitherDoNothingGoal(WitherBoss* boss);
        ~WitherDoNothingGoal() override;
        bool canUse() override;
        std::wstring getName() override;
    private:
        WitherBoss* mBoss;
    };
    explicit WitherBoss(Level* level);
    ~WitherBoss() override;
    static void create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    static void staticCtor();
    bool isAggressive() const;
    void setAggressive(bool aggressive);
    static bool canDestroy(const Block* block);
    void defineSynchedData() override;
    void registerAttributes() override;
    void setCustomName(const std::wstring& name, const std::wstring& displayName, bool visible) override;
    PathNavigation* createNavigation(Level* level) override;
    int getLightColor() override;
    EMobType getMobType() override;
    void startSeenByPlayer(boost::shared_ptr<ServerPlayer> player) override;
    void stopSeenByPlayer(boost::shared_ptr<ServerPlayer> player) override;
    void getPlayerParty();
    float getHeadYRot(int index) const;
    float getHeadXRot(int index) const;
    float rotlerp(float from, float to, float max);
    void setAerialAttack(bool attacking);
    bool getAerialAttack() const;
    void addAdditonalSaveData(CompoundTag* tag) override;
    int getInvulnerableTicks() const;
    void setInvulnerableTicks(int ticks);
    void readAdditionalSaveData(CompoundTag* tag) override;
    bool isPowered() const;
    bool IsChargeAttacking();
    bool canShoot() const;
    void aiStep() override;
    void makeInvulnerable();
    void makeStuckInWeb() override;
    int getArmorValue() const;
    bool hurt(const DamageSource* source, float damage) override;
    bool isPickable() override;
    void causeFallDamage(float fallDistance, float multiplier) override;
    void heal(float amount) override;
    bool canFly() const;
    void registerGoals() override;
    const SoundEvent* getAmbientSound() override;
    const SoundEvent* getHurtSound(const DamageSource* source) override;
    const SoundEvent* getDeathSound() override;
    void checkDespawn() override;
    bool canRide(boost::shared_ptr<Entity> entity) override;
    bool canChangeDimensions() override;
    double getHeadX(int index) const;
    double getHeadY(int index) const;
    double getHeadZ(int index) const;
    float getMaxHealth() const;
    float getHealth();
    unsigned long long GetType() override;
    std::wstring getDisplayName() const;
    void performRangedAttack(boost::shared_ptr<LivingEntity> target, float power);
    void performRangedAttack(int head, boost::shared_ptr<LivingEntity> target);
    int getAlternativeTarget(int head) const;
    void setAlternativeTarget(int head, int target);
    void generateParticles();
    void changePhase();
    bool destroyBlocks(Level* level, AABB* area, int flags);
    void customServerAiStep();
    void dropDeathLoot(bool hitByPlayer, int lootingLevel);
    float getOverlayAlpha() const;
    float getSwellAmount(float partialTick) const;
    int getTarget() const;
    bool IsSpawning() const;
    void setWantsToMove(bool wantsToMove);
    bool wantsToMove() const;
    void setIsPathing(bool pathing);
    void setShotDelay(int delay);
    int getDimension() const;
private:
    int mShieldHealth;
    int mPhase;
    int mSpawningFrames;
    int mMaxHealth;
    int mLastHealthInterval;
    int mDyingFrames;
    float mOverlayAlpha;
    float mSwellAmount;
    float mOldSwellAmount;
    int mAlternativeTargets[3];
    float mHeadYRot[3];
    float mHeadXRot[3];
    bool mAerialAttack;
    bool mAggressive;
    static const EntityDataAccessor<int>* DATA_TARGETS[3];
    static const EntityDataAccessor<int>* DATA_INVULNERABLE_TICKS;
    static const EntityDataAccessor<bool>* DATA_AERIAL_ATTACK;
    static const EntityDataAccessor<bool>* DATA_CHARGE_ATTACK;
    static const EntityDataAccessor<bool>* DATA_AGGRESSIVE;
    ServerBossEvent* mBossEvent;
};

#endif
