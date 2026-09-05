#include "game/core/Witch.hpp"

#include <cmath>

#include "game/core/DamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/world/Level.hpp"

static const EntityDataAccessor<bool>* witchUsingItem = NULL;

Witch::Witch(Level* level) : Monster(level), RangedAttackMob(), mUsingItem(false)
{
    setSize(0.6f, 1.95f);
}

Witch::~Witch()
{
}

void Witch::create(Level* level)
{
    new Witch(level);
}

void Witch::performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor)
{
    if (isDrinkingPotion() || !target)
        return;

    double dx = target->mX + target->mXd - mX;
    double dz = target->mZ + target->mZd - mZ;
    double distance = std::sqrt(dx * dx + dz * dz);
    Potion* potion = Potions::HARMING;
    if (target->getHealth() >= target->getMaxHealth() * 0.5f)
        potion = distance < 8.0 ? Potions::POISON : Potions::SLOWNESS;
    not_null_ptr<ItemInstance> item(new ItemInstance(Items::splashPotion));
    PotionUtils::setPotion(item, potion);
    boost::shared_ptr<ThrownPotion> thrown(new ThrownPotion(mLevel,
        boost::static_pointer_cast<LivingEntity>(shared_from_this()), item));
    thrown->mX = mX;
    thrown->mY = mY + 1.6;
    thrown->mZ = mZ;
    thrown->shoot(dx, target->mY + target->getEyeHeight() - 1.1 - thrown->mY, dz, 0.75f, 8.0f);
    mLevel->addEntity(boost::static_pointer_cast<Entity>(thrown));
}

bool Witch::isAggressive()
{
    return false;
}

void Witch::setAggressive(bool)
{
}

void Witch::staticCtor()
{
    witchUsingItem = SynchedEntityData::defineId<bool>(0x5D540046ULL, ENTITY_TYPE_MONSTER,
        EntityDataSerializers::BOOLEAN);
}

void Witch::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer);
}

void Witch::registerGoals()
{
    Mob::registerGoals();
}

void Witch::defineSynchedData()
{
    Mob::defineSynchedData();
    getEntityData()->createDateItem(witchUsingItem, false);
}

const SoundEvent* Witch::getAmbientSound()
{
    return SoundEvents::WITCH_AMBIENT;
}

const SoundEvent* Witch::getHurtSound(const DamageSource*)
{
    return SoundEvents::WITCH_HURT;
}

const SoundEvent* Witch::getDeathSound()
{
    return SoundEvents::WITCH_DEATH;
}

void Witch::setUsingItem(bool usingItem)
{
    mUsingItem = usingItem;
    getEntityData()->set(witchUsingItem, usingItem, true);
}

bool Witch::isDrinkingPotion()
{
    return getEntityData()->get(witchUsingItem);
}

void Witch::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(26.0);
}

void Witch::aiStep()
{
    if (isDrinkingPotion())
    {
        if (mUseItemRemaining-- <= 0)
            setUsingItem(false);
    }
    else
    {
        Monster::aiStep();
        return;
    }
    Monster::aiStep();
}

void Witch::handleEntityEvent(unsigned char event, int data)
{
    if (event == 15)
    {
        for (int i = 0; i < 10 + mRandom->nextInt(35); ++i)
            mLevel->addParticle(PARTICLE_WITCH_MAGIC, mX + mRandom->nextGaussian() * 0.13,
                mY + mRandom->nextGaussian() * 0.13 + mBbHeight, mZ + mRandom->nextGaussian() * 0.13,
                0.0, 0.0, 0.0, arrayWithLength<int>());
        return;
    }
    Monster::handleEntityEvent(event, data);
}

float Witch::getDamageAfterMagicAbsorb(const DamageSource* source, float damage)
{
    double result = LivingEntity::getDamageAfterMagicAbsorb(source, damage);
    if (source->isMagic())
        result *= 0.15;
    return (float)result;
}

const void* Witch::getDefaultLootTable()
{
    return &BuiltInLootTables::WITCH;
}

double Witch::getEyeHeight()
{
    return 1.62;
}

unsigned long long Witch::GetType()
{
    return 0x5D540046ULL;
}
