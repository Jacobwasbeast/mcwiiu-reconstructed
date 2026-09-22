#ifndef GAME_CORE_WOLF_HPP
#define GAME_CORE_WOLF_HPP
#include <boost/shared_ptr.hpp>
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"
class Block; class BlockPos; class CompoundTag; class DamageSource; class DyeColor; class DataFixerUpper;
class Entity; class ItemInstance; class Level; class SoundEvent; class ResourceLocation;
class AvoidEntityGoal;
class EntityTooltipDataHolder;
class Llama;
extern const unsigned long long ENTITY_TYPE_WOLF;
template <class T> class EntityDataAccessor;
class Wolf : public TamableAnimal
{
public:
    enum TooltipId
    {
        TOOLTIP_UNLEASH = 0x551269B5U,
        TOOLTIP_DYE_COLLAR = 0x58AE5A1AU,
        TOOLTIP_DYE_OWNER = 0x95CFA728U,
        TOOLTIP_SPAWN_WOLF = 0x4C36CD96U,
        TOOLTIP_LEASH = 0xA1B34A96U,
        TOOLTIP_RENAME = 0x5592C692U,
        TOOLTIP_FEED_BABY = 0x82F8D90FU,
        TOOLTIP_HEAL = 0x56B3F98DU,
        TOOLTIP_FEED = 0xB47A2AB6U
    };
    class WolfAvoidEntityGoal : public AvoidEntityGoal
    {
    public:
        WolfAvoidEntityGoal(Wolf* wolf, const std::type_info& avoidType, float maxDistance, double walkSpeed, double sprintSpeed);
        virtual ~WolfAvoidEntityGoal();
        bool avoidLlama(const boost::shared_ptr<Llama>& llama);
        virtual bool canUse();
        virtual void start();
        virtual void tick();
    private:
        Wolf* mWolf;
    };
    explicit Wolf(Level* level); virtual ~Wolf();
    static void create(Level* level); static void staticCtor(); static void addDataWalkers(DataFixerUpper* fixer);
    virtual void registerGoals(); virtual void registerAttributes(); virtual void setAngry(bool angry);
    virtual void setTarget(boost::shared_ptr<LivingEntity> target); virtual void customServerAiStep();
    virtual void defineSynchedData(); virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual bool isAngry(); virtual const DyeColor* getCollarColor();
    virtual void addAdditonalSaveData(CompoundTag* tag); virtual void setCollarColor(const DyeColor* color);
    virtual void readAdditionalSaveData(CompoundTag* tag); virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source); virtual const SoundEvent* getDeathSound();
    virtual float getSoundVolume(); virtual ResourceLocation* getDefaultLootTable(); virtual void aiStep();
    virtual bool isInterested(); virtual void tick(); virtual double getBodyRollAngle(float a, float b);
    virtual double getHeadRollAngle(float partialTick); virtual double getEyeHeight(); virtual int getMaxHeadXRot();
    virtual bool hurt(const DamageSource* source, float damage); virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual void setTame(bool tame); virtual void tame(boost::shared_ptr<Player> player, bool success, bool broadcast);
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual void handleEntityEvent(unsigned char event, int data); virtual double getTailAngle();
    virtual bool isFood(const not_null_ptr<ItemInstance>& item); virtual int GetSynchedHealth();
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual bool setIsInterested(bool interested); virtual bool canMate(const boost::shared_ptr<Animal>& other);
    virtual bool wantsToAttack(boost::shared_ptr<LivingEntity> owner, boost::shared_ptr<LivingEntity> target);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player); virtual unsigned long long GetType();
    bool isWet(); double getWetShade(float partialTick); virtual int getMaxSpawnClusterSize();
private:
    static const EntityDataAccessor<float>* DATA_HEALTH_ID;
    static const EntityDataAccessor<bool>* DATA_INTERESTED;
    static const EntityDataAccessor<int>* DATA_COLLAR_COLOR;
    float mInterestedAngle;
    float mInterestedAngleO;
    bool mIsWet;
    bool mIsShaking;
    float mShakeAnim;
    float mWetShade;
};
#endif
