#ifndef GAME_ENTITY_MOB_ABSTRACTSKELETON_HPP
#define GAME_ENTITY_MOB_ABSTRACTSKELETON_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/RangedAttackMob.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/item/not_null_ptr.hpp"

class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DifficultyInstance;
class EquipmentSlot;
class Level;
class MobGroupData;
class TippableArrow;
class RangedBowAttackGoal;
class ItemInstance;
template <class T> class EntityDataAccessor;

class AbstractSkeleton : public Monster, public RangedAttackMob
{
public:
    class SkeleteMeleeAttackGoal : public MeleeAttackGoal
    {
    public:
        SkeleteMeleeAttackGoal(AbstractSkeleton* skeleton);
        virtual ~SkeleteMeleeAttackGoal();
        virtual void start();
        virtual void stop();
    };

    AbstractSkeleton(Level* level);
    virtual ~AbstractSkeleton();

    virtual void performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor);
    virtual bool isAggressive();
    virtual void setAggressive(bool aggressive);
    static void staticCtor();
    void reassessWeaponGoal();
    virtual void tick();
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual void defineSynchedData();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual EMobType getMobType();
    virtual void aiStep();
    virtual void rideTick();
    virtual void die(const DamageSource* source);
    virtual void populateDefaultEquipmentSlots(const DifficultyInstance& difficulty);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                            const std::wstring& dataString);
    boost::shared_ptr<TippableArrow> getArrow(float distanceFactor);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> itemInstance);
    virtual double getEyeHeight();
    virtual double getRidingHeight();
    virtual bool breathesWater();
    virtual unsigned long long GetType();

private:
    static const EntityDataAccessor<bool>* DATA_ID_ATTACKING;
    SkeleteMeleeAttackGoal* mMeleeGoal;
    RangedBowAttackGoal* mBowGoal;
    bool mReassessWeaponGoal;
    bool mInWater;
};

#endif
