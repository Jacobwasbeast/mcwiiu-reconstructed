#ifndef GAME_ENTITY_MOB_ABSTRACTHORSE_HPP
#define GAME_ENTITY_MOB_ABSTRACTHORSE_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/OwnableEntity.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/core/InteractionHand.hpp"

class Block;
class BlockPos;
class CompoundTag;
class Container;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class Entity;
class Goal;
class Level;
class MobGroupData;
class SoundType;
class ItemInstance;
class AgableMob;

class AbstractHorse : public Animal, public OwnableEntity
{
public:
    AbstractHorse(Level* level);
    virtual ~AbstractHorse();
    bool getFlag(int flag);
    bool isSaddled();
    virtual void containerChanged(Container* container);
    void setFlag(int flag, bool value);
    void setEating(bool eating);
    void setStanding(bool standing);
    void stand();
    virtual void onPlayerJump(int jump);
    virtual bool canJump();
    virtual double getJumpRidingScale();
    virtual void setJumpRidingScale(float scale);
    virtual void handleStartJump(int jump);
    virtual void handleStopJump();
    virtual std::wstring getOwnerUUID();
    virtual boost::shared_ptr<LivingEntity> getOwner();
    int getTailCounter() const;
    static void staticCtor();
    virtual void fjDerivedCtorCalls();
    virtual void registerGoals();
    virtual void defineSynchedData();
    bool isTamed();
    void setOwnerUUID(const std::wstring& uuid);
    virtual double getFoalScale() const;
    virtual void updateSize(bool baby);
    bool isJumping() const;
    void setTamed(bool tamed);
    void setIsJumping(bool jumping);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
    bool isEating();
    virtual void onLeashDistance(float distance);
    bool isStanding();
    bool isBred();
    void setBred(bool bred);
    void setSaddled(bool saddled);
    int getTemper() const;
    void setTemper(int temper);
    int modifyTemper(int amount);
    virtual bool hurt(const DamageSource* source, float damage);
    virtual bool isPushable(boost::shared_ptr<Entity> entity);
    void openMouth();
    void eating(int animation);
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual int getInventorySize();
    virtual void createInventory();
    virtual void updateEquipment();
    boost::shared_ptr<Entity> getClosestMommy(const boost::shared_ptr<Entity>& entity, double distance);
    virtual double getCustomJump();
    virtual const SoundEvent* getJumpSound();
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getAngrySound();
    virtual bool canBeSaddled() const;
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual void playGallopSound(const SoundType* sound);
    virtual void registerAttributes();
    virtual int getMaxSpawnClusterSize();
    virtual int getMaxTemper() const;
    virtual float getSoundVolume();
    virtual int getAmbientSoundInterval();
    virtual void openInventory(const boost::shared_ptr<Player>& player);
    int CheckAndSetInLove(boost::shared_ptr<Player> player);
    int handleEating(const boost::shared_ptr<Player>& player, const not_null_ptr<ItemInstance>& item);
    void doPlayerRide(const boost::shared_ptr<Player>& player);
    virtual bool isImmobile();
    virtual bool isFood(const not_null_ptr<ItemInstance>& item);
    void moveTail();
    virtual void die(const DamageSource* source);
    virtual void aiStep();
    boost::shared_ptr<Entity> followMommy();
    bool canEatGrass() const;
    virtual void tick();
    boost::shared_ptr<Entity> makeMad();
    bool tameWithName(const boost::shared_ptr<Player>& player);
    virtual void travel(float x, float y, float z);
    virtual void updateModelAnimation();
    static void addDataWalkers(DataFixerUpper* fixer, unsigned long long type);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool canMate(const boost::shared_ptr<Animal>& other);
    bool canParent();
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    double generateRandomMaxHealth();
    double generateRandomJumpStrength();
    double generateRandomSpeed();
    void setOffspringAttributes(const boost::shared_ptr<AgableMob>& first, const boost::shared_ptr<AbstractHorse>& second);
    virtual bool canBeControlledByRider();
    double getEatAnim(float partialTick) const;
    double getStandAnim(float partialTick) const;
    double getMouthAnim(float partialTick) const;
    virtual void spawnTamingParticles(bool success);
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual void positionRider(const boost::shared_ptr<Entity>& passenger);
    virtual bool onLadder();
    virtual double getEyeHeight();
    virtual bool wearsArmor();
    virtual bool isArmor(const not_null_ptr<ItemInstance>& item);
    virtual bool setSlot(int slot, const not_null_ptr<ItemInstance>& item);
    virtual boost::shared_ptr<Entity> getControllingPassenger();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& name);
    virtual bool removeWhenFarAway();
    virtual unsigned long long GetType();

protected:
    int mFlags;
    bool mJumping;
    bool mStanding;
    bool mEating;
    bool mBred;
    bool mSaddled;
    bool mTamed;
    int mTemper;
    int mTailCounter;
    float mJumpRidingScale;
    std::wstring mOwnerUUID;
    boost::shared_ptr<Container> mInventory;
    Goal* mFloatGoal;
    Goal* mWaterAvoidingRandomStrollGoal;
};

#endif
