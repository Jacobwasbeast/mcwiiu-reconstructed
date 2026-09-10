#include "game/entity/mob/Zombie.hpp"

#include <boost/shared_ptr.hpp>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/FindMountGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/SinkInWaterRandomStrollGoal.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/core/Drowned.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MoveThroughVillageGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

static const EntityDataAccessor<bool>* gZombieBabyId = NULL;
static const EntityDataAccessor<bool>* gZombieAggressiveId = NULL;
static const EntityDataAccessor<bool>* gZombieJockeyId = NULL;

Zombie::ZombieGroupData::ZombieGroupData(bool canBreakDoors, bool isBaby, bool isJockey)
    : mCanBreakDoors(canBreakDoors), mIsBaby(isBaby), mIsJockey(isJockey)
{
}

Zombie::ZombieGroupData::~ZombieGroupData()
{
}

Zombie::Zombie(Level* level)
    : Monster(level), mBreakDoorGoal(NULL), mCanBreakDoors(false), mBaseWidth(0.6f),
      mBaseHeight(1.95f), mIsJockey(false), mConversionTime(0), mWaterConversionTime(0)
{
    fjDerivedCtorCalls();
    _init();
}

Zombie::Zombie(Level* level, bool initializeGoals)
    : Monster(level), mBreakDoorGoal(NULL), mCanBreakDoors(false), mBaseWidth(0.6f),
      mBaseHeight(1.95f), mIsJockey(false), mConversionTime(0), mWaterConversionTime(0)
{
    _init();
    if (initializeGoals)
        fjDerivedCtorCalls();
}

Zombie::~Zombie()
{
    delete mBreakDoorGoal;
}

void Zombie::create(Level* level)
{
    new Zombie(level);
}

void Zombie::staticCtor()
{
    gZombieBabyId = SynchedEntityData::defineId<bool>(0, 0, EntityDataSerializers::BOOLEAN);
    gZombieAggressiveId = SynchedEntityData::defineId<bool>(0, 0, EntityDataSerializers::BOOLEAN);
    gZombieJockeyId = SynchedEntityData::defineId<bool>(0, 0, EntityDataSerializers::BOOLEAN);
}

void Zombie::_init()
{
    mCanBreakDoors = false;
    mBaseWidth = 0.6f;
    mBaseHeight = 1.95f;
    mIsJockey = false;
    mConversionTime = 0;
    mWaterConversionTime = 0;
}

void Zombie::registerGoals()
{
    Monster::registerGoals();
    addBehaviourGoals();
}

void Zombie::addBehaviourGoals()
{
    mGoalSelector.addGoal(6, new MoveThroughVillageGoal(this, 1.0, false), true);
    mGoalSelector.addGoal(1, new FindMountGoal(*this, 16.0f, -1, false, false, -1.0f), true);
    arrayWithLength<unsigned long long> entityTypes(1, true);
    entityTypes.data[0] = 0x5d550041ULL;
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true, entityTypes), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Drowned), false, false, false), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(AbstractSkeleton), true, false, false), true);
    mTargetSelector.addGoal(5, new NearestAttackableTargetGoal(this, typeid(LivingEntity), 10, true, false, NULL, false, false), true);
}

void Zombie::registerAttributes()
{
    Monster::registerAttributes();
}

void Zombie::defineSynchedData()
{
    Monster::defineSynchedData();
    if (mEntityData && gZombieBabyId)
        mEntityData->createDateItem(gZombieBabyId, false);
    if (mEntityData && gZombieAggressiveId)
        mEntityData->createDateItem(gZombieAggressiveId, false);
    if (mEntityData && gZombieJockeyId)
        mEntityData->createDateItem(gZombieJockeyId, false);
}

void Zombie::setAggressive(bool aggressive)
{
    if (mEntityData && gZombieAggressiveId)
        mEntityData->set(gZombieAggressiveId, aggressive, true);
}

bool Zombie::isAggressive()
{
    return mEntityData && gZombieAggressiveId ? mEntityData->get(gZombieAggressiveId) : false;
}

bool Zombie::canBreakDoors()
{
    return mCanBreakDoors;
}

void Zombie::setCanBreakDoors(bool canBreakDoors)
{
    mCanBreakDoors = canBreakDoors;
}

bool Zombie::isBaby()
{
    return mEntityData && gZombieBabyId ? mEntityData->get(gZombieBabyId) : false;
}

bool Zombie::isJockey()
{
    return mIsJockey;
}

int Zombie::getExperienceReward(boost::shared_ptr<Player> player)
{
    int reward = Monster::getExperienceReward(player);
    if (isBaby())
        reward = (int)((float)reward * 2.5f);
    return reward;
}

void Zombie::setBaby(bool baby)
{
    if (mEntityData && gZombieBabyId)
        mEntityData->set(gZombieBabyId, baby, true);
    setSize(0.6f, 1.95f);
}

void Zombie::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (gZombieBabyId && accessor && accessor->getId() == gZombieBabyId->getId())
        setSize(0.6f, 1.95f);
    LivingEntity::onSyncedDataUpdated(accessor);
}

void Zombie::aiStep()
{
    Monster::aiStep();
}

bool Zombie::hurt(const DamageSource* source, float damage)
{
    return Monster::hurt(source, damage);
}

bool Zombie::doHurtTarget(boost::shared_ptr<Entity> target)
{
    return Monster::doHurtTarget(target);
}

bool Zombie::isSunSensitive() const
{
    return true;
}

const SoundEvent* Zombie::getAmbientSound()
{
    return SoundEvents::ZOMBIE_AMBIENT;
}

const SoundEvent* Zombie::getHurtSound(const DamageSource*)
{
    return SoundEvents::ZOMBIE_HURT;
}

const SoundEvent* Zombie::getDeathSound()
{
    return SoundEvents::ZOMBIE_DEATH;
}

const SoundEvent* Zombie::getStepSound()
{
    return SoundEvents::ZOMBIE_STEP;
}

void Zombie::playStepSound(const BlockPos&, Block*)
{
    playSound(getStepSound(), 0.15f, 1.0f);
}

EMobType Zombie::getMobType()
{
    return MOB_TYPE_UNDEAD;
}

ResourceLocation* Zombie::getDefaultLootTable()
{
    return &BuiltInLootTables::ZOMBIE_EQUIPMENT;
}

ResourceLocation* Zombie::getDefaultGearTable()
{
    return &BuiltInLootTables::ZOMBIE_EQUIPMENT;
}

void Zombie::populateDefaultEquipmentSlots(const DifficultyInstance& difficulty)
{
    Mob::populateDefaultEquipmentSlots(difficulty);
}

void Zombie::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x5d550040ULL);
}

void Zombie::addAdditonalSaveData(CompoundTag* tag)
{
    Monster::addAdditonalSaveData(tag);
    tag->putBoolean(std::wstring(L"CanBreakDoors"), mCanBreakDoors);
    tag->putInt(std::wstring(L"ConversionTime"), mConversionTime);
}

void Zombie::readAdditionalSaveData(CompoundTag* tag)
{
    Monster::readAdditionalSaveData(tag);
    mCanBreakDoors = tag->getBoolean(std::wstring(L"CanBreakDoors"));
    mConversionTime = tag->getInt(std::wstring(L"ConversionTime"));
}

void Zombie::killed(boost::shared_ptr<LivingEntity> entity)
{
    Entity::killed(entity);
}

double Zombie::getEyeHeight()
{
    return isBaby() ? 0.93 : 1.74;
}

void Zombie::positionRider(const boost::shared_ptr<Entity>& entity)
{
    Mob::positionRider(entity);
}

bool Zombie::breathesWater()
{
    return true;
}

bool Zombie::canHoldItem(not_null_ptr<ItemInstance> item)
{
    return !item->isEmpty() && !isPassenger();
}

MobGroupData* Zombie::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                       const std::wstring& dataString)
{
    Monster::finalizeMobSpawn(difficulty, data, dataString);
    return data;
}

bool Zombie::isConverting()
{
    return mConversionTime > 0;
}

void Zombie::startConverting(int conversionTime)
{
    mConversionTime = conversionTime;
}

void Zombie::waterCheck()
{
    if (isInWater())
        ++mWaterConversionTime;
    else
        mWaterConversionTime = 0;
    if (!isConverting() && mWaterConversionTime > 599)
        startConverting(300);
}

void Zombie::tick()
{
    waterCheck();
    Monster::tick();
}

void Zombie::setSize(float width, float height)
{
    bool initialized = mBaseWidth > 0.0f && mBaseHeight > 0.0f;
    mBaseWidth = width;
    mBaseHeight = height;
    if (!initialized)
        internalSetSize(1.0f);
}

void Zombie::internalSetSize(float scale)
{
    Entity::setSize(mBaseWidth * scale, mBaseHeight * scale);
}

not_null_ptr<ItemInstance> Zombie::getSkull()
{
    return ItemInstance::EMPTY;
}

double Zombie::getRidingHeight()
{
    return isBaby() ? 0.6 : 1.0;
}

void Zombie::die(const DamageSource* source)
{
    Monster::die(source);
}

bool Zombie::startRiding(boost::shared_ptr<Entity> vehicle, bool force)
{
    return Entity::startRiding(vehicle, force);
}

void Zombie::finishConversion()
{
    mConversionTime = 0;
}

unsigned long long Zombie::GetType()
{
    return 0x5D550040ULL;
}
