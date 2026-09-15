#include "game/core/Pufferfish.hpp"

#include <typeinfo>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/ai/RandomSwimmingGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/util/AABB.hpp"

Pufferfish::Pufferfish(Level* level)
    : Fish(level, false),
      mPuffState(PUFFED_STATE_SMALL),
      mNextPuffState(PUFFED_STATE_SMALL),
      mPuffTransitionTime(0),
      mPoisonDuration(200),
      mDamage(2),
      mSmallPuffRadius(1.5f),
      mLargePuffRadius(10.0f),
      mPuffSoundPitch(0.2f),
      mField778(0),
      mField77c(0),
      mField780(0),
      mField784(2)
{
    setSize(0.5f, 0.5f);
    fjDerivedCtorCalls();
}

Pufferfish::~Pufferfish()
{
}

void Pufferfish::create(Level* level)
{
    new Pufferfish(level);
}

void Pufferfish::setNextPuffedState(PuffedState state, int delay)
{
    mNextPuffState = state;
    mPuffTransitionTime = delay;
}

void Pufferfish::damageNearbyMobs()
{
    if (mPuffState <= PUFFED_STATE_MEDIUM)
        return;

    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntities(shared_from_this(), mBoundingBox->grow((double)mPuffSoundPitch,
                                                                    (double)mPuffSoundPitch,
                                                                    (double)mPuffSoundPitch), NULL);
    for (unsigned int i = 0; i < entities->size(); ++i)
    {
        boost::shared_ptr<Entity> entity = entities->at(i);
        if (entity->GetType() == ENTITY_TYPE_SERVER_PLAYER ||
            (entity->GetType() & ENTITY_TYPE_MONSTER) == ENTITY_TYPE_MONSTER)
        {
            boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(entity);
            living->hurt(DamageSource::CreateMobAttack(boost::static_pointer_cast<LivingEntity>(shared_from_this())), mDamage);
            living->addEffect(new MobEffectInstance(MobEffects::POISON, mPoisonDuration), boost::shared_ptr<Entity>());
        }
    }
}

bool Pufferfish::validatePuffedTypes(boost::shared_ptr<Entity> entity)
{
    if (entity == NULL)
        return false;
    unsigned long long type = entity->GetType();
    if ((type & 0x0C000000ULL) != 0x0C000000ULL)
        return false;
    return type != 0x0D502002ULL && type != 0x0D502004ULL &&
           type != 0x0D502003ULL && type != 0x0D502005ULL;
}

void Pufferfish::checkForDangerousMobs()
{
    bool foundDangerousMob = false;
    double radius = mPuffState == PUFFED_STATE_SMALL ? (double)mSmallPuffRadius : (double)mLargePuffRadius;
    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntities(shared_from_this(), mBoundingBox->grow(radius, radius, radius), NULL);
    for (unsigned int i = 0; i < entities->size(); ++i)
    {
        boost::shared_ptr<Entity> entity = entities->at(i);
        if (!validatePuffedTypes(entity))
            continue;

        bool isSpectatingPlayer = false;
        if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
            if (player->isSpectator())
                isSpectatingPlayer = true;
        }
        if (!isSpectatingPlayer && mPuffState == PUFFED_STATE_SMALL)
        {
            foundDangerousMob = true;
            setNextPuffedState(PUFFED_STATE_MEDIUM, 1);
        }
    }
    if (!foundDangerousMob && mPuffState == PUFFED_STATE_LARGE && mNextPuffState != PUFFED_STATE_MEDIUM)
        setNextPuffedState(PUFFED_STATE_MEDIUM, 200);
}

void Pufferfish::baseTick()
{
    Fish::baseTick();
    if (mNextPuffState != mPuffState)
    {
        --mPuffTransitionTime;
        if (mPuffTransitionTime < 1)
        {
            PuffedState oldState = mPuffState;
            mPuffState = mNextPuffState;
            if (oldState == PUFFED_STATE_SMALL)
                setNextPuffedState(PUFFED_STATE_LARGE, 1);
            else if (oldState == PUFFED_STATE_LARGE)
                setNextPuffedState(PUFFED_STATE_SMALL, 20);
        }
    }
    if (mPuffState == PUFFED_STATE_LARGE)
        damageNearbyMobs();
    checkForDangerousMobs();
}

bool Pufferfish::hurt(const DamageSource* source, float damage)
{
    bool result = LivingEntity::hurt(source, damage);
    if (getHealth() < 1.0f && mPuffState == PUFFED_STATE_SMALL)
        setNextPuffedState(PUFFED_STATE_MEDIUM, 1);
    return result;
}

void Pufferfish::registerGoals()
{
    mGoalSelector.addGoal(3, new RandomSwimmingGoal(this, 1.0, 10, 16, 4), true);
}

void Pufferfish::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(1.3);
}

const SoundEvent* Pufferfish::getFlopSound() const
{
    return SoundEvents::PUFFERFISH_FLOP;
}

const SoundEvent* Pufferfish::getHurtSound(const DamageSource*)
{
    return SoundEvents::PUFFERFISH_HURT;
}

const SoundEvent* Pufferfish::getDeathSound()
{
    return SoundEvents::PUFFERFISH_DEATH;
}

ResourceLocation* Pufferfish::getDefaultLootTable()
{
    return &BuiltInLootTables::PUFFERFISH;
}

unsigned long long Pufferfish::GetType()
{
    return 0x0D502003ULL;
}
