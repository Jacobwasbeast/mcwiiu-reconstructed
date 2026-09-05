#include "game/core/Pig.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/LightningBolt.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowParentGoal.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/core/System.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

const EntityDataAccessor<bool>* Pig::DATA_SADDLE = NULL;
const EntityDataAccessor<int>* Pig::DATA_BOOST_TIME = NULL;

static const unsigned long long ENTITY_TYPE_PIG = 0x000000002D680003ULL;

Pig::Pig(Level* level)
    : Animal(level), mBoosting(false), mBoostTime(0), mBoostTotalTime(0), mBoostStartTime(0)
{
    fjDerivedCtorCalls();
    setSize(0.9f, 0.9f);
}

Pig::~Pig()
{
}

void Pig::create(Level* level)
{
    new Pig(level);
}

void Pig::staticCtor()
{
    DATA_SADDLE = SynchedEntityData::defineId<bool>(ENTITY_TYPE_PIG, ENTITY_TYPE_PIG,
                                                     EntityDataSerializers::BOOLEAN);
    DATA_BOOST_TIME = SynchedEntityData::defineId<int>(ENTITY_TYPE_PIG, ENTITY_TYPE_PIG,
                                                        EntityDataSerializers::INT);
}

void Pig::registerGoals()
{
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new PanicGoal(this, 1.25), true);
    mGoalSelector.addGoal(3, new BreedGoal(this, 1.0), true);
    mGoalSelector.addGoal(5, new FollowParentGoal(this, 1.1), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
}

boost::shared_ptr<Entity> Pig::getControllingPassenger()
{
    boost::shared_ptr<Entity> passenger = Entity::getControllingPassenger();
    if (passenger && Entity::instanceof(passenger, ENTITY_TYPE_PLAYER))
        return passenger;
    return boost::shared_ptr<Entity>();
}

void Pig::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(10.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
}

bool Pig::canBeControlledByRider()
{
    boost::shared_ptr<Entity> passenger = getControllingPassenger();
    return passenger != NULL && Entity::instanceof(passenger, ENTITY_TYPE_PLAYER);
}

void Pig::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    AgableMob::onSyncedDataUpdated(accessor);
    if (DATA_BOOST_TIME != NULL && accessor == DATA_BOOST_TIME && getEntityData() != NULL)
    {
        mBoostTotalTime = getEntityData()->get(DATA_BOOST_TIME);
        mBoostTime = 0;
        mBoosting = mBoostTotalTime > 0;
        mBoostStartTime = System::processTimeInMilliSecs();
    }
}

void Pig::defineSynchedData()
{
    Animal::defineSynchedData();
    if (DATA_SADDLE != NULL) getEntityData()->createDateItem(DATA_SADDLE, false);
    if (DATA_BOOST_TIME != NULL) getEntityData()->createDateItem(DATA_BOOST_TIME, 0);
}

void Pig::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer);
}

bool Pig::hasSaddle()
{
    return DATA_SADDLE != NULL && getEntityData()->get(DATA_SADDLE);
}

void Pig::addAdditonalSaveData(CompoundTag* tag)
{
    Animal::addAdditonalSaveData(tag);
    tag->putBoolean(L"Saddle", hasSaddle());
}

void Pig::setSaddle(bool saddled)
{
    if (DATA_SADDLE != NULL) getEntityData()->set(DATA_SADDLE, saddled, true);
}

void Pig::readAdditionalSaveData(CompoundTag* tag)
{
    Animal::readAdditionalSaveData(tag);
    setSaddle(tag->getBoolean(L"Saddle"));
}

const SoundEvent* Pig::getAmbientSound() { return SoundEvents::PIG_AMBIENT; }
const SoundEvent* Pig::getHurtSound(const DamageSource*) { return SoundEvents::PIG_HURT; }
const SoundEvent* Pig::getDeathSound() { return SoundEvents::PIG_DEATH; }

void Pig::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::PIG_STEP, 0.15f, 1.0f);
}

bool Pig::mobInteract(const boost::shared_ptr<Player>& player,
                      InteractionHand::EInteractionHand hand)
{
    if (hasSaddle() && !isBaby())
        return true;
    return Animal::mobInteract(player, hand);
}

void Pig::die(const DamageSource* source)
{
    Animal::die(source);
    if (hasSaddle()) spawnAtLocation(Items::saddle, 1);
}

unsigned int Pig::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    return Animal::GetInteractTooltip(holder);
}

const void* Pig::getDefaultLootTable() { return &BuiltInLootTables::PIG; }

void Pig::thunderHit(const LightningBolt* lightning)
{
    Animal::thunderHit(lightning);
}

void Pig::causeFallDamage(float fallDistance, float damageMultiplier)
{
    LivingEntity::causeFallDamage(fallDistance, damageMultiplier);
    if (fallDistance > 5.0f) GenericStats::flyPig();
}

void Pig::travel(float xxa, float yya, float zza)
{
    if (isVehicle() && canBeControlledByRider())
    {
        boost();
        boost::shared_ptr<Entity> passenger = getControllingPassenger();
        if (passenger)
        {
            setRot(passenger->mYRot, mXRot);
            xxa = xxa;
            zza = zza;
        }
    }
    LivingEntity::travel(xxa, yya, zza);
}

bool Pig::boost()
{
    if (!mBoosting && mLevel != NULL && !mLevel->mIsClientSide)
    {
        mBoosting = true;
        mBoostTime = 0;
        mBoostTotalTime = mRandom->nextInt(0x231) + 0x8c;
        mBoostStartTime = System::processTimeInMilliSecs();
        if (DATA_BOOST_TIME != NULL) getEntityData()->set(DATA_BOOST_TIME, mBoostTotalTime, true);
        return true;
    }
    return false;
}

bool Pig::testboost() { return !mBoosting; }

boost::shared_ptr<AgableMob> Pig::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<AgableMob>(new Pig(mLevel));
}

bool Pig::isFood(const not_null_ptr<ItemInstance>& itemInstance)
{
    Item* item = itemInstance->getItem();
    return item == Items::carrot || item == Items::potato || item == Items::beetroot;
}

bool Pig::removeWhenFarAway()
{
    return Animal::removeWhenFarAway() && !hasSaddle();
}

unsigned long long Pig::GetType() { return ENTITY_TYPE_PIG; }
