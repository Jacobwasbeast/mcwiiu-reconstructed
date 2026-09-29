#include "game/core/Pig.hpp"
#include "game/entity/ai/TemptGoal.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cmath>
#include <set>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/LightningBolt.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowParentGoal.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/PigZombie.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/core/System.hpp"
#include "game/core/Mth.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemId.hpp"

const EntityDataAccessor<bool>* Pig::DATA_SADDLE = NULL;
const EntityDataAccessor<int>* Pig::DATA_BOOST_TIME = NULL;
arrayWithLength<Item*> Pig::FOOD_ITEMS;

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
    static Item* foodItems[] = { Items::carrot, Items::potato, Items::beetroot };
    FOOD_ITEMS = arrayWithLength<Item*>(foodItems, 3);
    DATA_SADDLE = SynchedEntityData::defineId<bool>(EntityType::PIG, EntityType::ANIMAL,
                                                     EntityDataSerializers::BOOLEAN);
    DATA_BOOST_TIME = SynchedEntityData::defineId<int>(EntityType::PIG, EntityType::ANIMAL,
                                                        EntityDataSerializers::INT);
}

void Pig::registerGoals()
{
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new PanicGoal(this, 1.25), true);
    mGoalSelector.addGoal(3, new BreedGoal(this, 1.0), true);
    static Item* steeringItems[] = { Items::carrotOnAStick };
    mGoalSelector.addGoal(4, new TemptGoal(this, 1.2, arrayWithLength<Item*>(steeringItems, 1), false, arrayWithLength<int>()),
                          true);
    mGoalSelector.addGoal(4, new TemptGoal(this, 1.2, FOOD_ITEMS, false, arrayWithLength<int>()), true);
    mGoalSelector.addGoal(5, new FollowParentGoal(this, 1.1), true);
    mGoalSelector.addGoal(6, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
}

boost::shared_ptr<Entity> Pig::getControllingPassenger()
{
    std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();
    if (passengers->empty())
        return boost::shared_ptr<Entity>();
    return passengers->at(0).lock();
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
    if (!Entity::instanceof(passenger, EntityType::PLAYER))
        return false;

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(passenger);
    return player->getMainHandItem()->getItem() == Items::carrotOnAStick
        || player->getOffhandItem()->getItem() == Items::carrotOnAStick;
}

void Pig::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (DATA_BOOST_TIME != NULL && accessor->getId() == DATA_BOOST_TIME->getId()
        && mLevel->mIsClientSide)
    {
        int boostTime = mEntityData->get(DATA_BOOST_TIME);
        if (boostTime < 1 || boostTime == mBoostTotalTime)
        {
            mBoostTotalTime = 0;
            mBoosting = false;
            mBoostStartTime = 0;
            mBoostTime = 0;
        }
        else
        {
            mBoostTotalTime = boostTime;
            mBoosting = true;
            mBoostTime = 0;
            mBoostStartTime = System::processTimeInMilliSecs();
        }
    }
    AgableMob::onSyncedDataUpdated(accessor);
}

void Pig::defineSynchedData()
{
    Animal::defineSynchedData();
    if (DATA_SADDLE != NULL) mEntityData->createDateItem(DATA_SADDLE, false);
    if (DATA_BOOST_TIME != NULL) mEntityData->createDateItem(DATA_BOOST_TIME, 0);
}

void Pig::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::PIG);
}

bool Pig::hasSaddle()
{
    return DATA_SADDLE != NULL && mEntityData->get(DATA_SADDLE);
}

void Pig::addAdditonalSaveData(CompoundTag* tag)
{
    Animal::addAdditonalSaveData(tag);
    tag->putBoolean(L"Saddle", hasSaddle());
}

void Pig::setSaddle(bool saddled)
{
    if (DATA_SADDLE != NULL) mEntityData->set(DATA_SADDLE, saddled, true);
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
    if (Animal::mobInteract(player, hand))
        return true;

    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (item->getItem() == Items::nameTag)
    {
        item->interactEnemy(player,
                            boost::static_pointer_cast<LivingEntity>(shared_from_this()), hand);
        return true;
    }

    if (hasSaddle() && !player->isVehicle() && !isLeashedToPlayer(player))
    {
        if (!mLevel->mIsClientSide)
        {
            if (!hasPassenger(boost::static_pointer_cast<Entity>(player)))
                player->startRiding(shared_from_this());
            else
                player->stopRiding();
        }
        return true;
    }

    if (item->getItem() != Items::saddle)
        return false;

    item->interactEnemy(player,
                        boost::static_pointer_cast<LivingEntity>(shared_from_this()), hand);
    return true;
}

void Pig::die(const DamageSource* source)
{
    LivingEntity::die(source);
    if (!mLevel->mIsClientSide && hasSaddle())
        spawnAtLocation(Items::saddle, 1);
}

unsigned int Pig::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    unsigned int tooltip = Animal::GetInteractTooltip(holder);
    if (tooltip == IDS_NONE)
    {
        if (hasSaddle() && getPassengers()->empty())
            return IDS_TOOLTIPS_MOUNT;
        if (Item::getId(holder.m_pItemInstance->getItem()) == ItemId::SADDLE && !hasSaddle())
            return IDS_TOOLTIPS_SADDLE;
    }
    return tooltip;
}

ResourceLocation* Pig::getDefaultLootTable() { return &BuiltInLootTables::PIG; }

void Pig::thunderHit(const LightningBolt* lightning)
{
    if (mLevel->mIsClientSide)
        return;

    boost::shared_ptr<PigZombie> pigZombie(new PigZombie(mLevel));
    pigZombie->setItemSlot(EquipmentSlot::MAINHAND,
                           not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenSword)));
    pigZombie->moveTo(mX, mY, mZ, mYRot, mXRot);
    pigZombie->setNoAi(isNoAi());

    if (hasCustomName())
    {
        pigZombie->setCustomName(getCustomName(), getOwnerUUID(), wasNamedByRestrictedPlayer());
        pigZombie->setCustomNameVisible(isCustomNameVisible());
    }

    mLevel->addEntity(pigZombie);
    remove();
}

void Pig::causeFallDamage(float fallDistance, float damageMultiplier)
{
    LivingEntity::causeFallDamage(fallDistance, damageMultiplier);
    if (fallDistance > 5.0f)
    {
        std::set<boost::shared_ptr<Entity> >* passengers =
            getIndirectPassengers(EntityType::PLAYER);
        for (std::set<boost::shared_ptr<Entity> >::const_iterator it = passengers->begin();
             it != passengers->end(); ++it)
        {
            boost::dynamic_pointer_cast<Player>(*it)->awardStat(
                GenericStats::flyPig(), GenericStats::param_flyPig());
        }
        delete passengers;
    }
}

void Pig::travel(float xxa, float yya, float zza)
{
    boost::shared_ptr<Entity> passenger;
    std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();
    if (!passengers->empty())
        passenger = passengers->at(0).lock();

    if (!isVehicle() || !canBeControlledByRider())
    {
        mMaxUpStep = 0.5f;
        mFlyingSpeed = 0.02f;
        LivingEntity::travel(xxa, yya, zza);
        return;
    }

    mYRot = passenger->mYRot;
    mYRotO = mYRot;
    mXRot = passenger->mXRot * 0.5f;
    setRot(mYRot, mXRot);
    mYBodyRot = mYRot;
    mYHeadRot = mYRot;
    mMaxUpStep = 1.0f;
    mFlyingSpeed = getSpeed() * 0.1f;

    if (mBoosting)
    {
        int elapsed = Mth::clamp(
            static_cast<int>((System::processTimeInMilliSecs() - mBoostStartTime) / 1000 * 20),
            0, mBoostTotalTime);
        if (!mLevel->mIsClientSide && elapsed == mBoostTotalTime)
        {
            mBoostTime = 0;
            mBoosting = false;
            mBoostTotalTime = 0;
            mBoostStartTime = 0;
            mEntityData->set(DATA_BOOST_TIME, 0, true);
        }
        else
        {
            mBoostTime = elapsed;
        }
    }

    if (isControlledByLocalInstance())
    {
        float speed = static_cast<float>(
            getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue()) * 0.225f;
        if (mBoosting)
            speed += (speed + speed)
                * std::sin(static_cast<float>(mBoostTime) / mBoostTotalTime * 3.1415927f);
        setSpeed(speed);
        LivingEntity::travel(0.0f, 0.0f, 1.0f);
    }
    else
    {
        mXd = 0.0;
        mYd = 0.0;
        mZd = 0.0;
    }

    mAnimationSpeedOld = mAnimationSpeed;
    float movement = static_cast<float>(
        std::sqrt((mX - mXo) * (mX - mXo) + (mZ - mZo) * (mZ - mZo))) * 4.0f;
    movement = std::min(movement, 1.0f);
    mAnimationSpeed += (movement - mAnimationSpeed) * 0.4f;
    mAnimationPosition += mAnimationSpeed;
}

bool Pig::boost()
{
    if (!mBoosting && mLevel != NULL && !mLevel->mIsClientSide)
    {
        mBoosting = true;
        mBoostTime = 0;
        mBoostTotalTime = mRandom->nextInt(0x231) + 0x8c;
        mBoostStartTime = System::processTimeInMilliSecs();
        if (DATA_BOOST_TIME != NULL) mEntityData->set(DATA_BOOST_TIME, mBoostTotalTime, true);
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
    return FOOD_ITEMS.data[0] == item || FOOD_ITEMS.data[1] == item || FOOD_ITEMS.data[2] == item;
}

bool Pig::removeWhenFarAway()
{
    return Animal::removeWhenFarAway() && !hasSaddle();
}

unsigned long long Pig::GetType() { return EntityType::PIG; }
