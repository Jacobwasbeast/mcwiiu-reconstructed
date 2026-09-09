#ifndef GAME_CORE_WOLF_HPP
#define GAME_CORE_WOLF_HPP

#include <boost/shared_ptr.hpp>
#include <typeinfo>

#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/core/InteractionHand.hpp"

class AgableMob;
class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class DyeColor;
class Entity;
class EntityTooltipDataHolder;
class Level;
class Llama;
class LivingEntity;
class Player;
class SoundEvent;
template <class T> class EntityDataAccessor;

class Wolf : public TamableAnimal
{
public:
    class WolfAvoidEntityGoal : public AvoidEntityGoal
    {
    public:
        WolfAvoidEntityGoal(Wolf* wolf, const std::type_info& avoidType, float maxDistance,
                            double walkSpeedModifier, double sprintSpeedModifier);
        virtual ~WolfAvoidEntityGoal();
        bool avoidLlama(const boost::shared_ptr<Llama>& llama);
        virtual bool canUse();
        virtual void start();
        virtual void tick();

    private:
        Wolf* mWolf;
    };

    Wolf(Level* level);
    virtual ~Wolf();
    static void create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    static void staticCtor();
    virtual void registerGoals();
    virtual void registerAttributes();
    void setAngry(bool angry);
    virtual void setTarget(boost::shared_ptr<LivingEntity> target);
    virtual void customServerAiStep();
    virtual void defineSynchedData();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    bool isAngry();
    const DyeColor* getCollarColor();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void setCollarColor(const DyeColor* color);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual float getSoundVolume();
    virtual const void* getDefaultLootTable();
    virtual void aiStep();
    bool isInterested();
    virtual void tick();
    double getBodyRollAngle(float partialTick, float scale);
    double getHeadRollAngle(float partialTick);
    virtual double getEyeHeight();
    virtual int getMaxHeadXRot();
    virtual bool hurt(const DamageSource* source, float amount);
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    void setTame(bool tame);
    void tame(boost::shared_ptr<Player> player, bool success, bool showParticles);
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual void handleEntityEvent(unsigned char event, int data);
    double getTailAngle();
    virtual bool isFood(const not_null_ptr<ItemInstance>& item);
    virtual int getMaxSpawnClusterSize();
    int GetSynchedHealth();
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    void setIsInterested(bool interested);
    virtual bool canMate(const boost::shared_ptr<Animal>& other);
    virtual bool wantsToAttack(boost::shared_ptr<LivingEntity> owner, boost::shared_ptr<LivingEntity> target);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
    virtual unsigned long long GetType();
    bool isWet();
    float getWetShade(float partialTick);

private:
    static const EntityDataAccessor<float>* DATA_HEALTH;
    static const EntityDataAccessor<bool>* DATA_INTERESTED;
    static const EntityDataAccessor<int>* DATA_COLLAR_COLOR;
    float mBodyRollAngle;
    float mInterestedAngle;
    unsigned char mIsWet;
    unsigned char mIsWetShaking;
    float mHeadRollAngle;
    float mTailRollAngle;
};

#endif
