#include "game/core/Squid.hpp"

#include <cmath>

#include "game/client/render/ParticleType.hpp"
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
    mTentacleAngleScale = mRandom->nextFloat() * 0.2f + 1.6f;
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
    mBodyRotation = 0;
    mBodyRotationOld = 0;
    mTentacleMovement = 0;
    mTentacleMovementOld = 0;
    mTentacleAngle = 0;
    mTentacleAngleOld = 0;
    mRotationVelocity = 0;
    mMovementDuration = 0;
    mMovementTicks = 0;
    mTentacleAngleScale = 0.0f;
    mMovementDelay = 0;
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
    return 0;
}

const SoundEvent* Squid::getHurtSound(const DamageSource*)
{
    return 0;
}

const SoundEvent* Squid::getDeathSound()
{
    return 0;
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
    return 0;
}

void Squid::aiStep()
{
    Mob::aiStep();
    mBodyRotationOld = mBodyRotation;
    mTentacleMovementOld = mTentacleMovement;
    mTentacleAngleOld = mTentacleAngle;
    mMovementTicks = mMovementDelay;
    mTentacleMovement += mMovementDuration;
    if (mTentacleMovement > 1.0f)
    {
        mTentacleMovement -= 1.0f;
        if (!mLevel->mIsClientSide && mRandom->nextInt(10) == 0)
        {
            mMovementDelay = static_cast<int>(mRandom->nextFloat() * 1.6f);
        }
    }
    if (!isInWaterOrBubble())
    {
        mXd = 0.0;
        mYd = 0.0;
        mZd = 0.0;
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
    return 0xd50200100000000ULL;
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
