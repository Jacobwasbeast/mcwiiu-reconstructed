#ifndef GAME_ENTITY_MOB_MOB_HPP
#define GAME_ENTITY_MOB_MOB_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockPathTypes;
class BodyControl;
class CensoredString;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class EntityTooltipDataHolder;
class EquipmentSlot;
class Level;
class MobGroupData;
class PathNavigation;
class Sensing;
class SoundEvent;
class HumanoidArm;
class ItemEntity;
class Item;
class JumpControl;
class LookControl;
class MoveControl;
class ResourceLocation;

template <class T> class EntityDataAccessor;

class Mob : public LivingEntity
{
    friend class FollowFlockLeaderGoal;
public:
    enum SpawnPlacementType
    {
        SPAWN_PLACEMENT_TYPE_DEFAULT = 0,
        SPAWN_PLACEMENT_TYPE_1 = 1,
        SPAWN_PLACEMENT_TYPE_2 = 2,
        SPAWN_PLACEMENT_TYPE_3 = 3,
        SPAWN_PLACEMENT_TYPE_4 = 4
    };

    Mob(Level* level);
    virtual ~Mob();
    virtual unsigned long long GetType();
    virtual void fjDerivedCtorCalls();
    virtual void defineSynchedData();
    virtual void tick();
    virtual void baseTick();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual bool interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual bool startRiding(boost::shared_ptr<Entity> entity, bool force);
    virtual void dropEquipment(bool hitByPlayer, int lootingLevel);
    virtual void dropAllDeathLoot(bool hitByPlayer, int lootingLevel, const DamageSource* source);
    virtual void dropDeathLoot(bool hitByPlayer, int lootingLevel);
    virtual void setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item);
    virtual not_null_ptr<ItemInstance> getItemSlot(const EquipmentSlot* slot);
    virtual void setSpeed(float speed);
    virtual float tickHeadTurn(float yBodyRot, float headTurnDelta);
    virtual void aiStep();
    virtual void serverAiStep();
    virtual bool isEffectiveAi();
    virtual const HumanoidArm* getMainArm();
    virtual void setNoAi(bool noAi);
    virtual void setLeftHanded(bool leftHanded);
    virtual bool isNoAi();
    virtual void underwaterEjectRider();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);

    static void staticCtor();
    void _init();
    void registerAttributes();
    void registerGoals();
    void createNavigation(Level* level);
    float getPathfindingMalus(const BlockPathTypes* type);
    void setPathfindingMalus(const BlockPathTypes* type, float malus);
    void createBodyControl();
    virtual LookControl* getLookControl();
    virtual MoveControl* getMoveControl();
    void* getJumpControl();
    PathNavigation* getNavigation();
    Sensing* getSensing();
    boost::shared_ptr<LivingEntity> getTarget();
    void setTarget(boost::shared_ptr<LivingEntity> target);
    virtual void setLevel(Level* level);
    bool canAttackType(unsigned long long type);
    void ate();
    unsigned int getAmbientSoundInterval();
    void playAmbientSound();
    void playHurtSound(const DamageSource* source);
    void resetAmbientSoundTime();
    int getExperienceReward(boost::shared_ptr<Player> player);
    void spawnAnim();
    void handleEntityEvent(unsigned char event, int data);
    void setLeader(bool leader);
    const SoundEvent* getAmbientSound();
    Item* getDeathLoot();
    static void addDataWalkers(DataFixerUpper* fixer);
    static void addDataWalkers(DataFixerUpper* fixer, unsigned long long entityType);
    bool isLeftHanded();
    bool canPickUpLoot();
    void setCanPickUpLoot(bool canPickUp);
    void setPersistenceRequired(bool required);
    bool isPersistenceRequired();
    const void* getDefaultLootTable();
    const void* getDefaultGearTable();
    void setZza(float zza);
    void setYya(float yya);
    void setXxa(float xxa);
    static const EquipmentSlot* getEquipmentSlotForItem(not_null_ptr<ItemInstance> item);
    void pickUpItem(boost::shared_ptr<ItemEntity> item);
    bool canHoldItem(not_null_ptr<ItemInstance> item);
    bool removeWhenFarAway();
    void checkDespawn();
    virtual void customServerAiStep();
    int getMaxHeadXRot();
    int getMaxHeadYRot();
    float rotlerp(float from, float to, float max);
    void lookAt(boost::shared_ptr<Entity> entity, float maxHeadXRot, float maxHeadYRot);
    bool checkSpawnRules();
    bool checkSpawnObstruction();
    float getSizeScale();
    float getHeadSizeScale();
    int getMaxSpawnClusterSize();
    int getMaxFallDistance();
    std::vector<boost::shared_ptr<ItemInstance> >* getHandSlots();
    std::vector<boost::shared_ptr<ItemInstance> >* getArmorSlots();
    static not_null_ptr<ItemInstance> getEquipmentForSlot(const EquipmentSlot* slot, int level);
    void populateDefaultEquipmentSlots(const DifficultyInstance& difficulty);
    void populateDefaultEquipmentEnchantments(const DifficultyInstance& difficulty);
    MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& dataString);
    void finalizeSpawnEggSpawn(int type);
    bool canBeControlledByRider();
    void setDropChance(const EquipmentSlot* slot, float chance);
    bool canBeLeashed(const boost::shared_ptr<Player>& player);
    unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    bool shouldRender(double distance);
    static bool isValidSlotForItem(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item);
    bool setSlot(int slot, const not_null_ptr<ItemInstance>& item);
    bool isControlledByLocalInstance();
    unsigned int maxFlockSize();
    unsigned int shouldFollow();
    void setFollowsLeader(bool follows);
    bool followsLeader();
    bool isLeader();
    virtual std::wstring getName();

protected:
    int mAmbientSoundTime;
    int mExperienceReward;
    LookControl* mLookControl;
    MoveControl* mMoveControl;
    JumpControl* mJumpControl;
    BodyControl* mBodyControl;
    PathNavigation* mNavigation;
    GoalSelector mGoalSelector;
    GoalSelector mTargetSelector;
    boost::weak_ptr<LivingEntity> mTarget;
    Sensing* mSensing;
    NonNullList<not_null_ptr<ItemInstance> > mHandItems;
    float mHandDropChances[2];
    NonNullList<not_null_ptr<ItemInstance> > mArmorItems;
    float mArmorDropChances[4];
    NonNullList<not_null_ptr<ItemInstance> > mAdditionalItems;
    float* mAdditionalDropChances;
    unsigned int mAdditionalItemCount;
    bool mCanPickUpLoot;
    bool mPersistenceRequired;
    boost::unordered_map<const BlockPathTypes*, float> mPathfindingMalus;
    boost::shared_ptr<CensoredString> mCustomName;
    float mMoveSpeed;
    ResourceLocation* mLootTable;
    long long mLootTableSeed;
    bool mLeader;
    bool mFollowsLeader;
    unsigned int mMaxFlockSize;
    unsigned int mFlockSize;

private:
    static const EntityDataAccessor<unsigned char>* DATA_MOB_FLAGS_ID;
};

#endif
