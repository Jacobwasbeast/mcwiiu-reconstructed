#include "game/core/Blaze.hpp"

#include <cmath>

#include "game/core/DamageSource.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/client/render/ParticleType.hpp"

const EntityDataAccessor<unsigned char>* Blaze::DATA_FLAGS;

Blaze::Blaze(Level* level)
    : Monster(level)
    , mHeightOffset(0.5f)
    , mHeightOffsetUpdateTime(0)
    , mFlameTime(0)
{
    fjDerivedCtorCalls();
    setPathfindingMalus(NULL, 0.0f);
    mHeightOffset = 0.5f;
    mFlameTime = 0;
    mHeightOffsetUpdateTime = 0;
}

Blaze::~Blaze()
{
}

void Blaze::staticCtor()
{
    DATA_FLAGS = SynchedEntityData::defineId<unsigned char>(0x5D540045ULL,
                                                              ENTITY_TYPE_MONSTER,
                                                              EntityDataSerializers::BYTE);
}

void Blaze::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer);
}

void Blaze::registerGoals()
{
    Mob::registerGoals();
}

void Blaze::registerAttributes()
{
    Monster::registerAttributes();
}

void Blaze::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_FLAGS, static_cast<unsigned char>(0));
}

const SoundEvent* Blaze::getAmbientSound()
{
    return SoundEvents::BLAZE_AMBIENT;
}

const SoundEvent* Blaze::getHurtSound(const DamageSource*)
{
    return SoundEvents::BLAZE_HURT;
}

const SoundEvent* Blaze::getDeathSound()
{
    return SoundEvents::BLAZE_DEATH;
}

int Blaze::getLightColor()
{
    return 0xF000F0;
}

float Blaze::getBrightness()
{
    return 1.0f;
}

void Blaze::aiStep()
{
    if (!mOnGround && mYd < 0.0)
    {
        mYd *= 0.6;
    }

    if (mLevel != NULL && mRandom->nextInt(24) == 0 && !isSilent())
    {
        mLevel->addParticle(PARTICLE_LARGE_SMOKE, mX + (mRandom->nextDouble() - 0.5) * mBbWidth,
                            mY + mRandom->nextDouble() * mBbHeight,
                            mZ + (mRandom->nextDouble() - 0.5) * mBbWidth,
                            0.0, 0.0, 0.0, arrayWithLength<int>());
    }

    Monster::aiStep();
}

void Blaze::customServerAiStep()
{
    if (isInWaterRainOrBubble())
    {
        setSecondsOnFire(1);
    }

    if (--mHeightOffsetUpdateTime <= 0)
    {
        mHeightOffsetUpdateTime = 100;
        mHeightOffset = static_cast<float>(mRandom->nextGaussian() * 3.0 + 0.5);
    }

    Mob::customServerAiStep();
}

void Blaze::causeFallDamage(float, float)
{
}

const void* Blaze::getDefaultLootTable()
{
    return &BuiltInLootTables::BLAZE;
}

bool Blaze::isCharged()
{
    return (mEntityData->get(DATA_FLAGS) & 1) != 0;
}

bool Blaze::isOnFire()
{
    return isCharged();
}

void Blaze::dropDeathLoot(bool hitByPlayer, int lootingLevel)
{
    if (!hitByPlayer)
    {
        return;
    }

    int rods = mRandom->nextInt(lootingLevel + 2);
    for (int i = 0; i < rods; ++i)
    {
        spawnAtLocation(NULL, 1);
    }

    int xp = mRandom->nextInt(lootingLevel + 3);
    for (int i = 0; i < xp; ++i)
    {
        spawnAtLocation(NULL, 1);
    }
}

unsigned long long Blaze::setCharged(bool charged)
{
    unsigned char flags = mEntityData->get(DATA_FLAGS);
    flags = charged ? static_cast<unsigned char>(flags | 1) : static_cast<unsigned char>(flags & 0xFE);
    mEntityData->set(DATA_FLAGS, flags, true);
    return 0;
}

bool Blaze::isDarkEnoughToSpawn()
{
    return true;
}

double Blaze::getFollowDistance()
{
    AttributeInstance* attribute = getAttribute(NULL);
    return attribute == NULL ? 16.0 : attribute->getValue();
}

unsigned long long Blaze::GetType()
{
    return 0x5D540045ULL;
}

void Blaze::create(Level* level)
{
    new Blaze(level);
}

Blaze::BlazeAttackGoal::BlazeAttackGoal(Blaze* blaze)
    : Goal()
    , mBlaze(blaze)
    , mAttackTime(0)
    , mAttackStep(0)
{
    setRequiredControlFlags(3);
}

Blaze::BlazeAttackGoal::~BlazeAttackGoal()
{
}

bool Blaze::BlazeAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mBlaze->getTarget();
    return target != NULL && target->isAlive();
}

void Blaze::BlazeAttackGoal::start()
{
    mAttackTime = 0;
}

void Blaze::BlazeAttackGoal::stop()
{
    mBlaze->setCharged(false);
}

void Blaze::BlazeAttackGoal::tick()
{
    boost::shared_ptr<LivingEntity> target = mBlaze->getTarget();
    if (target == NULL)
    {
        return;
    }

    double distance = mBlaze->distanceToSqr(target);
    double followDistance = mBlaze->getFollowDistance();
    mBlaze->getNavigation()->moveTo(target, 1.0);
    static_cast<LookControl*>(mBlaze->getLookControl())->setLookAt(target, 10.0f, 10.0f);

    if (distance < 4.0 * followDistance * followDistance)
    {
        if (--mAttackTime <= 0)
        {
            mAttackTime = 20;
            ++mAttackStep;
            if (mAttackStep == 1)
            {
                mBlaze->setCharged(true);
            }
            else if (mAttackStep >= 5)
            {
                mAttackStep = 0;
                mAttackTime = 100;
                mBlaze->setCharged(false);
            }
        }
    }
}

std::wstring Blaze::BlazeAttackGoal::getName()
{
    return std::wstring(L"BlazeAttack");
}
