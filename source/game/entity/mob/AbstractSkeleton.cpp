#include "game/entity/mob/AbstractSkeleton.hpp"

#include <cmath>

#include "game/core/AttributeInstance.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

const EntityDataAccessor<bool>* AbstractSkeleton::DATA_ID_ATTACKING = NULL;

AbstractSkeleton::AbstractSkeleton(Level* level)
    : Monster(level), mMeleeGoal(new SkeleteMeleeAttackGoal(this)), mBowGoal(NULL),
      mReassessWeaponGoal(true), mInWater(false)
{
    setSize(0.6f, 1.99f);
    mBlocksBuilding = true;
}

AbstractSkeleton::~AbstractSkeleton()
{
    delete mBowGoal;
    delete mMeleeGoal;
}

void AbstractSkeleton::performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor)
{
    boost::shared_ptr<TippableArrow> arrow = getArrow(distanceFactor);
    if (arrow)
        setAggressive(true);
}

bool AbstractSkeleton::isAggressive()
{
    return getEntityData()->get(DATA_ID_ATTACKING);
}

void AbstractSkeleton::setAggressive(bool aggressive)
{
    getEntityData()->set(DATA_ID_ATTACKING, aggressive, true);
}

void AbstractSkeleton::staticCtor()
{
    DATA_ID_ATTACKING = SynchedEntityData::defineId<bool>(0x4D54044000000000ULL,
                                                           ENTITY_TYPE_MONSTER,
                                                           EntityDataSerializers::BOOLEAN);
}

void AbstractSkeleton::reassessWeaponGoal()
{
    mReassessWeaponGoal = false;
    if (mLevel == NULL || mLevel->mIsClientSide)
    {
        mReassessWeaponGoal = true;
        return;
    }
    if (mMeleeGoal != NULL)
        mGoalSelector.removeGoal(mMeleeGoal);
    if (mMeleeGoal != NULL)
        mGoalSelector.addGoal(4, mMeleeGoal, false);
}

void AbstractSkeleton::tick()
{
    Monster::tick();
    if (mReassessWeaponGoal)
        reassessWeaponGoal();
}

void AbstractSkeleton::registerGoals()
{
    Mob::registerGoals();
    if (mMeleeGoal != NULL)
        mGoalSelector.addGoal(4, mMeleeGoal, false);
}

void AbstractSkeleton::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
}

void AbstractSkeleton::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_ID_ATTACKING, false);
}

void AbstractSkeleton::playStepSound(const BlockPos& pos, Block* block)
{
    Entity::playStepSound(pos, block);
}

EMobType AbstractSkeleton::getMobType()
{
    return MOB_TYPE_UNDEAD;
}

void AbstractSkeleton::aiStep()
{
    if (mLevel != NULL && !mLevel->mIsClientSide && mLevel->isDay() && !isInWater())
    {
        not_null_ptr<ItemInstance> item = getMainHandItem();
        if (item->isEmpty())
            setSecondsOnFire(8);
        else if (item->isDamageableItem())
        {
            item->setAuxValue(item->getDamageValue() + mRandom->nextInt(2));
            if (item->getDamageValue() >= item->getMaxDamage())
                setItemSlot(EquipmentSlot::MAINHAND, not_null_ptr<ItemInstance>(new ItemInstance()));
        }
    }
    bool inWater = isInWater();
    if (inWater != mInWater)
        mReassessWeaponGoal = true;
    mInWater = inWater;
    Monster::aiStep();
}

void AbstractSkeleton::rideTick()
{
    LivingEntity::rideTick();
    if (mVehicle)
        mYRot = mVehicle->mYRot;
}

void AbstractSkeleton::die(const DamageSource* source)
{
    LivingEntity::die(source);
}

void AbstractSkeleton::populateDefaultEquipmentSlots(const DifficultyInstance& difficulty)
{
    Mob::populateDefaultEquipmentSlots(difficulty);
}

MobGroupData* AbstractSkeleton::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                                  const std::wstring& dataString)
{
    data = Mob::finalizeMobSpawn(difficulty, data, dataString);
    populateDefaultEquipmentSlots(difficulty);
    populateDefaultEquipmentEnchantments(difficulty);
    mReassessWeaponGoal = true;
    return data;
}

boost::shared_ptr<TippableArrow> AbstractSkeleton::getArrow(float distanceFactor)
{
    return boost::shared_ptr<TippableArrow>();
}

void AbstractSkeleton::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    mReassessWeaponGoal = true;
}

void AbstractSkeleton::setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> itemInstance)
{
    Mob::setItemSlot(slot, itemInstance);
    if (slot == EquipmentSlot::MAINHAND)
        mReassessWeaponGoal = true;
}

double AbstractSkeleton::getEyeHeight()
{
    return 1.62;
}

double AbstractSkeleton::getRidingHeight()
{
    return -0.3;
}

bool AbstractSkeleton::breathesWater()
{
    return true;
}

unsigned long long AbstractSkeleton::GetType()
{
    return 0x4D54044000000000ULL;
}

AbstractSkeleton::SkeleteMeleeAttackGoal::SkeleteMeleeAttackGoal(AbstractSkeleton* skeleton)
    : MeleeAttackGoal(skeleton, 1.2, true)
{
}

AbstractSkeleton::SkeleteMeleeAttackGoal::~SkeleteMeleeAttackGoal()
{
}

void AbstractSkeleton::SkeleteMeleeAttackGoal::start()
{
    MeleeAttackGoal::start();
    static_cast<AbstractSkeleton*>(mMob)->setAggressive(true);
}

void AbstractSkeleton::SkeleteMeleeAttackGoal::stop()
{
    MeleeAttackGoal::stop();
    static_cast<AbstractSkeleton*>(mMob)->setAggressive(false);
}
