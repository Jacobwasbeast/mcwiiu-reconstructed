#include "game/core/Squid.hpp"

#include <cmath>

#include "game/client/render/ParticleType.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"

Squid::Squid(Level* level) : WaterAnimal(level)
{
    _init();
    setSize(0.8f, 0.8f);
    mRandom->setSeed((long long)getId());
    mTentacleSpeed = 1.0f / (mRandom->nextFloat() + 1.0f) * 0.2f;
}

Squid::~Squid()
{
}

void Squid::create(Level* level)
{
    new Squid(level);
}

void Squid::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0xd502001ULL);
}

void Squid::_init()
{
    mXBodyRot = 0.0f;
    mXBodyRotO = 0.0f;
    mZBodyRot = 0.0f;
    mZBodyRotO = 0.0f;
    mTentacleMovement = 0.0f;
    mOldTentacleMovement = 0.0f;
    mTentacleAngle = 0.0f;
    mOldTentacleAngle = 0.0f;
    mSpeed = 0.0f;
    mTentacleSpeed = 0.0f;
    mRotateSpeed = 0.0f;
    mMovementX = 0.0f;
    mMovementY = 0.0f;
    mMovementZ = 0.0f;
}

void Squid::registerGoals()
{
    mGoalSelector.addGoal(0, new SquidRandomMovementGoal(this), true);
}

void Squid::registerAttributes()
{
    Mob::registerAttributes();
}

double Squid::getEyeHeight()
{
    return static_cast<double>(mBbHeight * 0.5f);
}

const SoundEvent* Squid::getAmbientSound()
{
    return SoundEvents::SQUID_AMBIENT;
}

const SoundEvent* Squid::getHurtSound(const DamageSource*)
{
    return SoundEvents::SQUID_HURT;
}

const SoundEvent* Squid::getDeathSound()
{
    return SoundEvents::SQUID_DEATH;
}

float Squid::getSoundVolume()
{
    return 0.4f;
}

bool Squid::makeStepSound()
{
    return 1;
}

void Squid::playStepSound(const BlockPos&, Block*)
{
}

const void* Squid::getDefaultLootTable()
{
    return &BuiltInLootTables::SQUID;
}

void Squid::aiStep()
{
    Mob::aiStep();
    mXBodyRotO = mXBodyRot;
    mZBodyRotO = mZBodyRot;
    mOldTentacleMovement = mTentacleMovement;
    mOldTentacleAngle = mTentacleAngle;
    float tentacleMovement = mTentacleMovement + mTentacleSpeed;
    if (tentacleMovement > 6.2831855f)
    {
        mTentacleMovement = tentacleMovement - 6.2831855f;
        if (!mLevel->mIsClientSide)
        {
            if (mRandom->nextInt(10) == 0)
            {
                mTentacleSpeed = 1.0f / (mRandom->nextFloat() + 1.0f) * 0.2f;
            }
            mLevel->broadcastEntityEvent(shared_from_this(), 19, 0);
        }
    }
    else
    {
        mTentacleMovement = tentacleMovement;
    }

    if (!isInWaterOrBubble())
    {
        mTentacleAngle = Mth::abs(Mth::sin(mTentacleMovement)) * 3.1415927f * 0.25f;
        mXd = 0.0;
        mZd = 0.0;
        if (hasEffect(MobEffects::LEVITATION))
        {
            mYd = (mYd + (0.05 * (double)(getEffect(MobEffects::LEVITATION)->getAmplifier() + 1) - mYd)) * 0.98;
        }
        else
        {
            double yd = mYd;
            if (!isNoGravity())
            {
                yd -= 0.08;
            }
            mYd = yd * 0.98;
        }
        mXBodyRot += (-90.0f - mXBodyRot) * 0.02f;
    }
    else
    {
        if (mTentacleMovement < 3.1415927f)
        {
            float progress = mTentacleMovement / 3.1415927f;
            mTentacleAngle = Mth::sin(progress * progress * 3.1415927f) * 3.1415927f * 0.25f;
            if ((double)progress > 0.75)
            {
                mSpeed = 1.0f;
                mRotateSpeed = 1.0f;
            }
            else
            {
                mRotateSpeed *= 0.8f;
            }
        }
        else
        {
            mTentacleAngle = 0.0f;
            mSpeed *= 0.9f;
            mRotateSpeed *= 0.99f;
        }

        if (!mLevel->mIsClientSide)
        {
            mXd = (double)(mMovementX * mSpeed);
            mYd = (double)(mMovementY * mSpeed);
            mZd = (double)(mMovementZ * mSpeed);
        }
        double horizontalSpeed = std::sqrt(mXd * mXd + mZd * mZd);
        mYBodyRot += (-(float)std::atan2(mXd, mZd) * 57.295776f - mYBodyRot) * 0.1f;
        mYRot = mYBodyRot;
        mZBodyRot += 3.1415927f * mRotateSpeed * 1.5f;
        mXBodyRot += (-(float)std::atan2(horizontalSpeed, mYd) * 57.295776f - mXBodyRot) * 0.1f;
    }
}

void Squid::travel(float xa, float ya, float za)
{
    LivingEntity::travel(xa, ya, za);
}

bool Squid::checkSpawnRules()
{
    return mY > 45.0 && mY < static_cast<double>(mLevel->getSeaLevel()) && WaterAnimal::checkSpawnRules();
}

Vec3* Squid::randomInkDir()
{
    float x = mRandom->nextFloat() * mBbWidth * 2.0f - mBbWidth;
    float y = mRandom->nextFloat() * mBbHeight + 0.5f;
    float z = mRandom->nextFloat() * mBbWidth * 2.0f - mBbWidth;
    return Vec3::newTemp(x, y, z)->normalize();
}

void Squid::spawnInkParticles()
{
    unsigned int count = mLevel->mIsClientSide ? 8 : 15;
    for (unsigned int i = 0; i < count; ++i)
    {
        Vec3* direction = randomInkDir();
        direction->multiply(0.1, 0.1, 0.1);
        mLevel->addParticle(PARTICLE_INK, mX, mY, mZ, direction->x, direction->y, direction->z, arrayWithLength<int>());
    }
}

void Squid::handleEntityEvent(unsigned char event, int data)
{
    if (data == 0x13)
    {
        mTentacleMovement = 0;
    }
    else if (data == 0x26)
    {
        spawnInkParticles();
    }
    else
    {
        Mob::handleEntityEvent(event, data);
    }
}

void Squid::setMovementVector(float x, float y, float z)
{
    mMovementX = x;
    mMovementY = y;
    mMovementZ = z;
}

bool Squid::hasMovementVector()
{
    return mMovementX != 0.0f || mMovementY != 0.0f || mMovementZ != 0.0f;
}

bool Squid::hurt(const DamageSource* source, float damage)
{
    bool hurt = LivingEntity::hurt(source, damage);
    if (hurt)
    {
        spawnInkParticles();
        handleEntityEvent(0, 0x26);
    }
    return hurt;
}

unsigned int Squid::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    return Mob::GetInteractTooltip(holder);
}

unsigned long long Squid::GetType()
{
    return 0x0D502001ULL;
}

Squid::SquidRandomMovementGoal::SquidRandomMovementGoal(Squid* squid) : Goal(), mSquid(squid)
{
}

Squid::SquidRandomMovementGoal::~SquidRandomMovementGoal()
{
}

bool Squid::SquidRandomMovementGoal::canUse()
{
    return true;
}

void Squid::SquidRandomMovementGoal::tick()
{
    if (mSquid->getAirSupply() < 101)
    {
        mSquid->setMovementVector(0.0f, 0.0f, 0.0f);
    }
    else if (mSquid->getRandom()->nextInt(50) == 0 || !mSquid->hasMovementVector())
    {
        float angle = mSquid->getRandom()->nextFloat() * 6.2831855f;
        mSquid->setMovementVector(std::cos(angle) * 0.2f,
                                  mSquid->getRandom()->nextFloat() * 0.2f - 0.1f,
                                  std::sin(angle) * 0.2f);
    }
}

std::wstring Squid::SquidRandomMovementGoal::getName()
{
    return std::wstring(L"SquidRandomMovement");
}
