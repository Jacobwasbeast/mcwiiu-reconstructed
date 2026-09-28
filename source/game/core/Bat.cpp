#include "game/core/Bat.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/core/PIXNamedEvents.hpp"

extern bool gHalloweenEvent;


const EntityDataAccessor<unsigned char>* Bat::DATA_ID_FLAGS = NULL;

Bat::Bat(Level* level)
    : AmbientCreature(level), mTargetPosition(NULL)
{
    fjDerivedCtorCalls();
    setSize(0.5f, 0.9f);
    setResting(true);
    mCategories |= EntityCategory::eEntityCategory_Bat;
}

Bat::~Bat()
{
    delete mTargetPosition;
}

void Bat::staticCtor()
{
    DATA_ID_FLAGS = SynchedEntityData::defineId(EntityType::BAT, EntityType::AMBIENT_CREATURE,
                                                 EntityDataSerializers::BYTE);
}

void Bat::setResting(bool resting)
{
    unsigned char flags = mEntityData->get(DATA_ID_FLAGS);
    mEntityData->set(DATA_ID_FLAGS,
                         resting ? (unsigned char)(flags | 1) : (unsigned char)(flags & 0xfe),
                         true);
}

void Bat::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_ID_FLAGS, (unsigned char)0);
}

float Bat::getSoundVolume()
{
    return 0.1f;
}

float Bat::getVoicePitch()
{
    return LivingEntity::getVoicePitch() * 0.95f;
}

const SoundEvent* Bat::getAmbientSound()
{
    if (isResting() && mRandom->nextInt(4) != 0)
        return NULL;
    return SoundEvents::BAT_AMBIENT;
}

const SoundEvent* Bat::getHurtSound(const DamageSource*)
{
    return SoundEvents::BAT_HURT;
}

const SoundEvent* Bat::getDeathSound()
{
    return SoundEvents::BAT_DEATH;
}

bool Bat::isPushable(boost::shared_ptr<Entity>)
{
    return false;
}

void Bat::doPush(boost::shared_ptr<Entity>)
{
}

void Bat::pushEntities()
{
}

void Bat::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(6.0);
}

bool Bat::isResting()
{
    return (mEntityData->get(DATA_ID_FLAGS) & 1) != 0;
}

void Bat::tick()
{
    PIXBeginNamedEvent(0.0f, "Bat tick");
    Mob::tick();
    if (!isResting())
    {
        mYd *= 0.6000000238418579;
    }
    else
    {
        int y = (int)mY;
        mXd = 0.0;
        mYd = 0.0;
        mZd = 0.0;
        if (mY < (double)y)
            --y;
        mY = ((double)y + 1.0) - (double)mBbHeight;
    }
    PIXEndNamedEvent();
}

void Bat::customServerAiStep()
{
    Mob::customServerAiStep();

    BlockPos pos(shared_from_this());
    BlockPos above(pos.above());
    if (isResting())
    {
        const BlockState* state = mLevel->getBlockState(above);
        if (!state->getBlock()->isSolidBlockingCubeAndNotSignalSource(state))
        {
            setResting(false);
            mLevel->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_BAT_LIFTOFF, pos, 0);
        }
        else
        {
            if (mRandom->nextInt(200) == 0)
                mYRot = (float)mRandom->nextInt(360);
            boost::shared_ptr<Player> player = mLevel->getNearestPlayer(shared_from_this(), 4.0, -1.0);
            if (player)
            {
                setResting(false);
                mLevel->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_BAT_LIFTOFF, pos, 0);
            }
        }
        return;
    }

    bool chooseTarget = mTargetPosition == NULL;
    if (!chooseTarget)
    {
        if (mLevel->isEmptyBlock(*mTargetPosition) && mTargetPosition->y >= 1)
        {
            if (mRandom->nextInt(30) != 0 &&
                mTargetPosition->distSqr((int)mX, (int)mY, (int)mZ) >= 4.0)
                chooseTarget = false;
            else
                chooseTarget = true;
        }
        else
            chooseTarget = true;
    }
    if (chooseTarget)
    {
        delete mTargetPosition;
        mTargetPosition = new BlockPos((int)mX + mRandom->nextInt(7) - mRandom->nextInt(7),
                                       (int)mY + mRandom->nextInt(6) - 2,
                                       (int)mZ + mRandom->nextInt(7) - mRandom->nextInt(7));
    }

    double xDelta = ((double)mTargetPosition->x + 0.5) - mX;
    double yDelta = ((double)mTargetPosition->y + 0.1) - mY;
    double zDelta = ((double)mTargetPosition->z + 0.5) - mZ;
    float xDirection = xDelta > 0.0 ? 1.0f : (xDelta < 0.0 ? -1.0f : 0.0f);
    float yDirection = yDelta > 0.0 ? 1.0f : (yDelta < 0.0 ? -1.0f : 0.0f);
    float zDirection = zDelta > 0.0 ? 1.0f : (zDelta < 0.0 ? -1.0f : 0.0f);
    mXd = (xDirection * 0.5f - mXd) * 0.1f + mXd;
    mYd = (yDirection * 0.7f - mYd) * 0.1f + mYd;
    mZd = (zDirection * 0.5f - mZd) * 0.1f + mZd;
    mYRot += Mth::wrapDegrees((float)(std::atan2(mZd, mXd) * 57.2957763671875 - 90.0f - mYRot));
    mFlyingSpeed = 0.5f;
    if (mRandom->nextInt(100) == 0 &&
        mLevel->getBlockState(above)->getBlock()->isSolidBlockingCubeAndNotSignalSource(
            mLevel->getBlockState(above)))
        setResting(true);
}

bool Bat::makeStepSound()
{
    return false;
}

void Bat::causeFallDamage(float, float)
{
}

void Bat::checkFallDamage(double, bool, Block*, const BlockPos&)
{
}

bool Bat::isIgnoringBlockTriggers()
{
    return true;
}

bool Bat::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;
    if (!mLevel->mIsClientSide && isResting())
        setResting(false);
    return LivingEntity::hurt(source, damage);
}

void Bat::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::BAT);
}

void Bat::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    mEntityData->set(DATA_ID_FLAGS, tag->getByte(L"BatFlags"), true);
}

void Bat::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putByte(L"BatFlags", mEntityData->get(DATA_ID_FLAGS));
}

bool Bat::checkSpawnRules()
{
    BlockPos pos(mX, mBoundingBox->mMinY, mZ);
    if (pos.y < mLevel->getSeaLevel())
    {
        int brightness = mLevel->getRawBrightness(pos);
        int threshold;
        if (!gHalloweenEvent)
        {
            if (mRandom->nextBoolean())
                return false;
            threshold = mRandom->nextInt(4);
        }
        else
            threshold = mRandom->nextInt(7);
        if (brightness <= threshold)
            return Mob::checkSpawnRules();
    }
    return false;
}

double Bat::getEyeHeight()
{
    return (double)(mBbHeight * 0.5f);
}

ResourceLocation* Bat::getDefaultLootTable()
{
    return &BuiltInLootTables::BAT;
}

unsigned long long Bat::GetType()
{
    return EntityType::BAT;
}

void Bat::create(Level* level)
{
    new Bat(level);
}
