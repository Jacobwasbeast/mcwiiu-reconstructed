#include "game/entity/mob/AbstractSkeleton.hpp"

#include <cmath>

#include "game/core/AttributeInstance.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/util/AABB.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/ai/RangedBowAttackGoal.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<bool>* AbstractSkeleton::DATA_ID_ATTACKING = NULL;

AbstractSkeleton::AbstractSkeleton(Level* level)
    : Monster(level), mMeleeGoal(new SkeleteMeleeAttackGoal(this)),
      mBowGoal(new RangedBowAttackGoal(this, 1.0, 20, 15.0f)),
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
    Entity* targetEntity = target.get();
    const double dx = targetEntity->mX - mX;
    const double dz = targetEntity->mZ - mZ;
    const double horizontalDistance = std::sqrt(dx * dx + dz * dz);
    const double targetY = targetEntity->mBoundingBox->mMinY
        + static_cast<double>(targetEntity->mBbHeight) / 3.0;
    const double dy = targetY - arrow->mY;
    const float inaccuracy = 14.0f
        - static_cast<float>(mLevel->getDifficulty()->getId()) * 4.0f;

    arrow->shoot(dx, dy + horizontalDistance * 0.2, dz, 1.6f, inaccuracy);
    mLevel->addEntity(boost::static_pointer_cast<Entity>(arrow));

    const float pitch = 1.0f / (mRandom->nextFloat() * 0.4f + 1.2f)
        + distanceFactor * 0.5f;
    playSound(SoundEvents::SKELETON_SHOOT, 1.0f, pitch);
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
    if (mBowGoal != NULL)
        mGoalSelector.removeGoal(mBowGoal);

    not_null_ptr<ItemInstance> item = getMainHandItem();
    if (!item->isEmpty() && item->getItem() == Items::bow && !isInWater())
    {
        int interval = mLevel->getDifficulty() == Difficulty::HARD ? 40 : 20;
        mBowGoal->setMinAttackInterval(interval);
        mGoalSelector.addGoal(4, mBowGoal, false);
    }
    else
    {
        mGoalSelector.addGoal(4, mMeleeGoal, false);
    }
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
    playSound(getStepSound(), 0.15f, 1.0f);
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

    boost::shared_ptr<Entity> directEntity = source->getDirectEntity();
    if (!directEntity)
        return;

    boost::shared_ptr<Entity> entity = source->getEntity();
    if (!entity)
        return;

    if ((directEntity->GetType() & 0x8a020000ULL) != 0x8a020000ULL)
        return;

    if ((entity->GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER)
        return;

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    const double dx = player->mX - mX;
    const double dz = player->mZ - mZ;
    if (dx * dx + dz * dz < 2500.0)
        return;

    player->awardStat(GenericStats::snipeSkeleton(), GenericStats::param_snipeSkeleton());
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
    boost::shared_ptr<LivingEntity> owner = boost::static_pointer_cast<LivingEntity>(shared_from_this());
    boost::shared_ptr<TippableArrow> arrow(new TippableArrow(mLevel, owner));
    arrow->setEnchantmentEffectsFromEntity(owner, distanceFactor);
    return arrow;
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
    return 1.62f;
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
    return 0x000000004D540440ULL;
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
