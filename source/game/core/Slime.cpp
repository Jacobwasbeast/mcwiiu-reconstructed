#include "game/core/Slime.hpp"
#include "game/client/render/ParticleType.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/JumpControl.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/core/Difficulty.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

const EntityDataAccessor<int>* Slime::DATA_ID_SIZE = NULL;

Slime::Slime(Level* level)
    : Mob(level)
{
    fjDerivedCtorCalls();
    _init();
    mMoveControl = new SlimeMoveControl(this);
}

Slime::~Slime()
{
}

unsigned long long Slime::GetType()
{
    return 0x5D108000ULL;
}

void Slime::staticCtor()
{
    DATA_ID_SIZE = SynchedEntityData::defineId<int>(0, 0x5D108000ULL,
                                                     EntityDataSerializers::INT);
}

void Slime::create(Level* level)
{
    new Slime(level);
}

void Slime::_init()
{
    mSquishO = 0.0f;
    mSquish = 0.0f;
    mTargetSquish = 0.0f;
    mWasOnGround = false;
}

void Slime::registerGoals()
{
    mGoalSelector.addGoal(1, new SlimeFloatGoal(this), true);
    mGoalSelector.addGoal(2, new SlimeAttackGoal(this), true);
    mGoalSelector.addGoal(3, new SlimeRandomDirectionGoal(this), true);
    mGoalSelector.addGoal(5, new SlimeKeepOnJumpingGoal(this), true);
    Mob::registerGoals();
}

void Slime::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_ID_SIZE, 1);
}

void Slime::setSize(int size, bool reset)
{
    mEntityData->set(DATA_ID_SIZE, size, true);
    Entity::setSize(size * 0.51f, size * 0.51f);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue((double)(size * size));
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue((double)(size * 0.6f + 1.0f));
    if (reset)
    {
        setPos(mX, mY, mZ);
        setBoundingBox(mBoundingBox);
    }
}

int Slime::getSize()
{
    return mEntityData->get(DATA_ID_SIZE);
}

void Slime::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x5d108000ULL);
}

void Slime::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(L"Size", getSize() - 1);
    tag->putBoolean(L"wasOnGround", mWasOnGround);
}

void Slime::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    int size = tag->getInt(L"Size");
    if (size < 0)
        size = 0;
    setSize(size + 1, false);
    mWasOnGround = tag->getBoolean(L"wasOnGround");
}

bool Slime::isTiny()
{
    return getSize() < 2;
}

const void* Slime::getParticleType()
{
    return PARTICLE_SLIME;
}

void Slime::tick()
{
    PIXBeginNamedEvent(0.0f, "Slime tick");
    mSquishO = mSquish;
    mSquish += (mTargetSquish - mSquish) * 0.5f;
    Mob::tick();
    if (mOnGround && !mWasOnGround)
    {
        int count = getSize() * 8;
        for (int i = 0; i < count; ++i)
            mLevel->addParticle(PARTICLE_SLIME, mX, mY, mZ, 0.0, 0.0, 0.0, arrayWithLength<int>());
        mTargetSquish = -0.5f;
        if (getSize() > 0)
            doPlayJumpSound();
    }
    else if (!mOnGround && mWasOnGround)
        mTargetSquish = 1.0f;
    mWasOnGround = mOnGround;
    if (mLevel->mIsClientSide && getSize() > 0)
        setSize(getSize(), false);
    PIXEndNamedEvent();
}

void Slime::decreaseSquish()
{
    mTargetSquish *= 0.6f;
}

int Slime::getJumpDelay()
{
    return mRandom->nextInt(20) + 10;
}

boost::shared_ptr<Slime> Slime::createChild()
{
    boost::shared_ptr<Slime> child(new Slime(mLevel));
    child->setSize(getSize() / 2, true);
    child->setPos(mX, mY, mZ);
    child->mYRot = mYRot;
    child->mXRot = mXRot;
    return child;
}

void Slime::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    Entity::onSyncedDataUpdated(accessor);
    setSize(getSize(), false);
}

void Slime::remove()
{
    int size = getSize();
    if (!mLevel->mIsClientSide && size > 1 && isAlive())
    {
        int count = 2 + mRandom->nextInt(3);
        for (int i = 0; i < count; ++i)
        {
            boost::shared_ptr<Slime> child = createChild();
            mLevel->addEntity(child);
        }
    }
    Mob::remove();
}

void Slime::push(boost::shared_ptr<Entity> entity)
{
    if (isDealsDamage())
        dealDamage(boost::shared_ptr<LivingEntity>(entity, dynamic_cast<LivingEntity*>(entity.get())));
    Entity::push(entity);
}

void Slime::playerTouch(boost::shared_ptr<Player> player)
{
    if (isDealsDamage())
        dealDamage(boost::shared_ptr<LivingEntity>(player, (LivingEntity*)player.get()));
    Entity::playerTouch(player);
}

void Slime::dealDamage(boost::shared_ptr<LivingEntity> entity)
{
    if (entity && isDealsDamage())
        entity->hurt(0, getAttackDamage());
}

double Slime::getEyeHeight()
{
    return (double)mBbHeight * 0.625;
}

bool Slime::isDealsDamage()
{
    return !isTiny();
}

int Slime::getAttackDamage()
{
    return getSize();
}

const SoundEvent* Slime::getHurtSound(const DamageSource*)
{
    return isTiny() ? SoundEvents::SMALL_SLIME_HURT : SoundEvents::SLIME_HURT;
}

const SoundEvent* Slime::getDeathSound()
{
    return isTiny() ? SoundEvents::SMALL_SLIME_DEATH : SoundEvents::SLIME_DEATH;
}

const SoundEvent* Slime::getSquishSound()
{
    return isTiny() ? SoundEvents::SMALL_SLIME_SQUISH : SoundEvents::SLIME_SQUISH;
}

ResourceLocation* Slime::getDefaultLootTable()
{
    return isTiny() ? NULL : &BuiltInLootTables::SLIME;
}

bool Slime::checkSpawnRules()
{
    int x = static_cast<int>(mX);
    if (mX < static_cast<double>(x))
        --x;
    int z = static_cast<int>(mZ);
    if (mZ < static_cast<double>(z))
        --z;
    BlockPos pos(x, 0, z);
    LevelChunk* chunk = mLevel->getChunkAt(pos);
    if ((mLevel->getLevelData()->getGeneratorType() != LevelType::FLAT || mRandom->nextInt(4) == 1) &&
        mLevel->getDifficulty() != Difficulty::PEACEFUL)
    {
        Biome* biome = mLevel->getBiome(pos);
        if (biome == Biome::SWAMPLAND && mY > 50.0 && mY < 70.0 && mRandom->nextFloat() < 0.5f)
        {
            if (mRandom->nextFloat() < mLevel->getMoonBrightness())
            {
                BlockPos entityPos(this);
                if (mLevel->getRawBrightness(entityPos) <= mRandom->nextInt(8))
                    return Mob::checkSpawnRules();
            }
        }
        if (mRandom->nextInt(10) == 0)
        {
            if (mY < 40.0 && (chunk->getBiomePresentFlag() & 4) != 0)
                return Mob::checkSpawnRules();
        }
    }
    return false;
}

float Slime::getSoundVolume()
{
    return 0.4f * (float)getSize();
}

int Slime::getMaxHeadXRot()
{
    return 0;
}

bool Slime::doPlayJumpSound()
{
    playSound(getJumpSound(), getSoundVolume(), 1.0f);
    return !isTiny();
}

void Slime::jumpFromGround()
{
    LivingEntity::jumpFromGround();
    mTargetSquish = 1.0f;
}

MobGroupData* Slime::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& dataString)
{
    setSize(1 + mRandom->nextInt(3), true);
    return Mob::finalizeMobSpawn(difficulty, data, dataString);
}

const SoundEvent* Slime::getJumpSound()
{
    return isTiny() ? SoundEvents::SMALL_SLIME_JUMP : SoundEvents::SLIME_JUMP;
}

Slime::SlimeAttackGoal::SlimeAttackGoal(Slime* slime)
    : mSlime(slime), mGrowTiredTimer(0)
{
}

Slime::SlimeAttackGoal::~SlimeAttackGoal()
{
}

bool Slime::SlimeAttackGoal::canUse()
{
    return mSlime->getTarget() && mSlime->isDealsDamage();
}

void Slime::SlimeAttackGoal::start()
{
    mGrowTiredTimer = 300;
    Goal::start();
}

bool Slime::SlimeAttackGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mSlime->getTarget();
    if (!target || !target->isAlive())
        return false;
    if (target->isBaby())
        return false;
    return --mGrowTiredTimer > 0;
}

void Slime::SlimeAttackGoal::tick()
{
    boost::shared_ptr<LivingEntity> target = mSlime->getTarget();
    if (target)
    {
        mSlime->lookAt(target, 10.0f, 10.0f);
        mSlime->getMoveControl()->setWantedPosition(target->mX, target->mY, target->mZ, 1.0);
    }
}

std::wstring Slime::SlimeAttackGoal::getName()
{
    return L"SlimeAttack";
}

Slime::SlimeFloatGoal::SlimeFloatGoal(Slime* slime)
    : mSlime(slime)
{
    setRequiredControlFlags(5);
    GroundPathNavigation* navigation = dynamic_cast<GroundPathNavigation*>(mSlime->getNavigation());
    if (navigation)
        navigation->setCanFloat(true);
}

Slime::SlimeFloatGoal::~SlimeFloatGoal()
{
}

bool Slime::SlimeFloatGoal::canUse()
{
    return mSlime->isInWater() || mSlime->isInLava();
}

void Slime::SlimeFloatGoal::tick()
{
    static_cast<SlimeMoveControl*>(mSlime->getMoveControl())->setWantedMovement(1.0);
}

std::wstring Slime::SlimeFloatGoal::getName()
{
    return L"SlimeFloat";
}

Slime::SlimeKeepOnJumpingGoal::SlimeKeepOnJumpingGoal(Slime* slime)
    : mSlime(slime)
{
    setRequiredControlFlags(5);
}

Slime::SlimeKeepOnJumpingGoal::~SlimeKeepOnJumpingGoal()
{
}

bool Slime::SlimeKeepOnJumpingGoal::canUse()
{
    return true;
}

void Slime::SlimeKeepOnJumpingGoal::tick()
{
    static_cast<JumpControl*>(mSlime->getJumpControl())->jump();
    static_cast<SlimeMoveControl*>(mSlime->getMoveControl())->setWantedMovement(1.0);
}

std::wstring Slime::SlimeKeepOnJumpingGoal::getName()
{
    return L"SlimeKeepOnJumping";
}

Slime::SlimeMoveControl::SlimeMoveControl(Slime* slime)
    : MoveControl(slime), mYRot(0.0f), mJumpDelay(0), mSlime(slime), mWasOnGround(false)
{
}

Slime::SlimeMoveControl::~SlimeMoveControl()
{
}

void Slime::SlimeMoveControl::setDirection(float yRot, bool jump)
{
    mYRot = yRot;
    mWasOnGround = jump;
}

void Slime::SlimeMoveControl::setWantedMovement(double speed)
{
    mSpeedModifier = speed;
    mOperation = MOVE_TO;
}

void Slime::SlimeMoveControl::tick()
{
    mSlime->mYRot = rotlerp(mSlime->mYRot, mYRot, 90.0f);
    mSlime->mYBodyRot = mSlime->mYRot;
    if (mOperation == MOVE_TO)
        mSlime->setSpeed((float)(mSpeedModifier * mSlime->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue()));
    else
        mSlime->setSpeed(0.0f);
    if (mWasOnGround)
    {
        if (mJumpDelay-- <= 0)
        {
            mJumpDelay = mSlime->getJumpDelay();
            static_cast<JumpControl*>(mSlime->getJumpControl())->jump();
            mSlime->doPlayJumpSound();
        }
    }
}

Slime::SlimeRandomDirectionGoal::SlimeRandomDirectionGoal(Slime* slime)
    : mSlime(slime), mChosenDegrees(0.0f), mChosenTicks(0)
{
    setRequiredControlFlags(2);
}

Slime::SlimeRandomDirectionGoal::~SlimeRandomDirectionGoal()
{
}

bool Slime::SlimeRandomDirectionGoal::canUse()
{
    return !mSlime->isInWater() && !mSlime->isInLava();
}

void Slime::SlimeRandomDirectionGoal::tick()
{
    if (--mChosenTicks <= 0)
    {
        mChosenTicks = mSlime->mRandom->nextInt(60) + 40;
        mChosenDegrees = (float)mSlime->mRandom->nextInt(360);
    }
    static_cast<SlimeMoveControl*>(mSlime->getMoveControl())->setDirection(mChosenDegrees, false);
}

std::wstring Slime::SlimeRandomDirectionGoal::getName()
{
    return L"SlimeRandomDirection";
}
