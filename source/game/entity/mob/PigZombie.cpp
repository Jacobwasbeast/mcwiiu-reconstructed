#include "game/entity/mob/PigZombie.hpp"

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

namespace
{
    AttributeModifier gPigZombieAttackingSpeedBoost(
        eMODIFIER_ID_PigZombieAttackingSpeedBoost, 0.05, 0);
}

PigZombie::PigZombie(Level* level)
    : Zombie(level, false), mRemainingPersistentAngerTime(0), mAngerSoundDelay(0)
{
    fjDerivedCtorCalls();
    _init();
}

PigZombie::~PigZombie()
{
}

void PigZombie::create(Level* level)
{
    new PigZombie(level);
}

void PigZombie::_init()
{
    mRemainingPersistentAngerTime = 0;
    mAngerSoundDelay = 0;
    mCategories |= PigZombieCategoryMask;
}

void PigZombie::setLastHurtByMob(boost::shared_ptr<LivingEntity> entity)
{
    LivingEntity::setLastHurtByMob(entity);
}

void PigZombie::addBehaviourGoals()
{
    mTargetSelector.addGoal(1, new PigZombieHurtByOtherGoal(this), true);
}

void PigZombie::registerAttributes()
{
    Zombie::registerAttributes();
    getAttributes()->getInstance(eATTRIBUTE_ID_ZombieSpawnReinforcements)->setBaseValue(1.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.23);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(5.0);
}

bool PigZombie::isAngry()
{
    return mRemainingPersistentAngerTime > 0;
}

void PigZombie::customServerAiStep()
{
    AttributeInstance* speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
    if (!isAngry())
    {
        if (speed->getModifier(eMODIFIER_ID_PigZombieAttackingSpeedBoost) != NULL)
            speed->removeModifier(eMODIFIER_ID_PigZombieAttackingSpeedBoost);
    }
    else
    {
        if (!isBaby() && speed->getModifier(eMODIFIER_ID_PigZombieAttackingSpeedBoost) == NULL)
            speed->addModifier(new AttributeModifier(gPigZombieAttackingSpeedBoost));
        --mRemainingPersistentAngerTime;
    }

    if (mAngerSoundDelay > 0 && --mAngerSoundDelay == 0)
    {
        playSound(SoundEvents::ZOMBIE_PIG_ANGRY, getSoundVolume() * 2.0f,
                  (1.0f + (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f) * 1.8f);
    }
    Mob::customServerAiStep();
}

bool PigZombie::checkSpawnRules()
{
    if (mLevel->getDifficulty() == Difficulty::PEACEFUL ||
        !mLevel->isUnobstructed(mBoundingBox))
        return false;

    boost::shared_ptr<Entity> self = shared_from_this();
    std::vector<AABB*>* collisions =
        mLevel->getCollisionAABBs(self, mBoundingBox, false, false, false);
    bool result = collisions->empty() && !mLevel->containsAnyLiquid(mBoundingBox);
    delete collisions;
    return result;
}

void PigZombie::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, ENTITY_TYPE_PIG_ZOMBIE);
}

void PigZombie::addAdditonalSaveData(CompoundTag* tag)
{
    Zombie::addAdditonalSaveData(tag);
    tag->putShort(std::wstring(L"Anger"), (short)mRemainingPersistentAngerTime);
}

void PigZombie::readAdditionalSaveData(CompoundTag* tag)
{
    Zombie::readAdditionalSaveData(tag);
    mRemainingPersistentAngerTime = tag->getShort(std::wstring(L"Anger"));
}

void PigZombie::makeAngry(boost::shared_ptr<Entity> entity)
{
    mRemainingPersistentAngerTime = mRandom->nextInt(400) + 400;
    mAngerSoundDelay = mRandom->nextInt(40);
    if ((entity->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
        setLastHurtByMob(boost::dynamic_pointer_cast<LivingEntity>(entity));
}

bool PigZombie::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;

    boost::shared_ptr<Entity> attacker = source->getEntity();
    if (attacker && (attacker->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        makeAngry(attacker);
    return Zombie::hurt(source, damage);
}

const SoundEvent* PigZombie::getAmbientSound()
{
    return SoundEvents::ZOMBIE_PIG_AMBIENT;
}

const SoundEvent* PigZombie::getHurtSound(const DamageSource*)
{
    return SoundEvents::ZOMBIE_PIG_HURT;
}

const SoundEvent* PigZombie::getDeathSound()
{
    return SoundEvents::ZOMBIE_PIG_DEATH;
}

ResourceLocation* PigZombie::getDefaultLootTable()
{
    return &BuiltInLootTables::ZOMBIE_PIGMAN;
}

ResourceLocation* PigZombie::getDefaultGearTable()
{
    return &BuiltInLootTables::ZOMBIE_PIGMAN_GEAR;
}

bool PigZombie::mobInteract(const boost::shared_ptr<Player>&, InteractionHand::EInteractionHand)
{
    return false;
}

void PigZombie::populateDefaultEquipmentSlots(const DifficultyInstance& difficulty)
{
    Mob::populateDefaultEquipmentSlots(difficulty);
}

not_null_ptr<ItemInstance> PigZombie::getSkull()
{
    return ItemInstance::EMPTY;
}

bool PigZombie::isPreventingPlayerRest(boost::shared_ptr<Player>)
{
    return isAngry();
}

unsigned long long PigZombie::GetType()
{
    return ENTITY_TYPE_PIG_ZOMBIE;
}

PigZombie::PigZombieHurtByOtherGoal::PigZombieHurtByOtherGoal(PigZombie* mob)
    : HurtByTargetGoal(mob, true, arrayWithLength<unsigned long long>())
{
}

PigZombie::PigZombieHurtByOtherGoal::~PigZombieHurtByOtherGoal()
{
}

void PigZombie::PigZombieHurtByOtherGoal::alertOther(
    boost::shared_ptr<PathfinderMob> mob, boost::shared_ptr<LivingEntity> target)
{
    HurtByTargetGoal::alertOther(mob, target);
    if (mob->GetType() == ENTITY_TYPE_PIG_ZOMBIE)
    {
        boost::shared_ptr<PigZombie> pig = boost::dynamic_pointer_cast<PigZombie>(mob);
        pig->makeAngry(target);
    }
}
