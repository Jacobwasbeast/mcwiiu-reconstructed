#include "game/core/Ghast.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/EServerSetting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Settings.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/projectile/LargeFireball.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/MobNearestPlayerTargetGoal.hpp"

static const EntityDataAccessor<bool>* DATA_CHARGING = NULL;

Ghast::Ghast(Level* level) : FlyingMob(level), mUnused(0), mExplosionPower(1)
{
    Mob::fjDerivedCtorCalls();
    _init();
    setSize(4.0f, 4.0f);
    mFireImmune = true;
    mExperienceReward = 5;
    delete mMoveControl;
    mMoveControl = new GhastMoveControl(this);
}

void Ghast::create(Level* level)
{
    new Ghast(level);
}

void Ghast::staticCtor()
{
    DATA_CHARGING = SynchedEntityData::defineId<bool>(EntityType::GHAST, EntityType::FLYING_MOB,
                                                   EntityDataSerializers::BOOLEAN);
}

void Ghast::_init()
{
    mExplosionPower = 1;
    mUnused = 0;
}

void Ghast::registerGoals()
{
    mGoalSelector.addGoal(5, new RandomFloatAroundGoal(this), true);
    mGoalSelector.addGoal(7, new GhastLookGoal(this), true);
    mGoalSelector.addGoal(7, new GhastShootFireballGoal(this), true);
    mTargetSelector.addGoal(1, new MobNearestPlayerTargetGoal(this), true);
}

bool Ghast::isCharging()
{
    return mEntityData->get(DATA_CHARGING);
}

void Ghast::setCharging(bool charging)
{
    mEntityData->set(DATA_CHARGING, charging, true);
}

int Ghast::getExplosionPower()
{
    return mExplosionPower;
}

bool Ghast::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;
    boost::shared_ptr<Entity> direct = source->getDirectEntity();
    if (direct && direct->GetType() == EntityType::LARGE_FIREBALL)
    {
        boost::shared_ptr<Entity> owner = source->getEntity();
        if (owner && (owner->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
        {
            LivingEntity::hurt(source, 1000.0f);
            boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(source->getEntity());
            player->awardStat(GenericStats::ghast(), GenericStats::param_ghast());
            return true;
        }
    }
    return LivingEntity::hurt(source, damage);
}

void Ghast::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_CHARGING, false);
}

void Ghast::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(10.0);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(100.0);
}

const SoundEvent* Ghast::getAmbientSound() { return SoundEvents::GHAST_AMBIENT; }
const SoundEvent* Ghast::getHurtSound(const DamageSource*) { return SoundEvents::GHAST_SCREAM; }
const SoundEvent* Ghast::getDeathSound() { return SoundEvents::GHAST_DEATH; }
ResourceLocation* Ghast::getDefaultLootTable() { return &BuiltInLootTables::GHAST; }
float Ghast::getSoundVolume() { return 0.5f; }

bool Ghast::checkSpawnRules()
{
    return mRandom->nextInt(20) == 0 && Mob::checkSpawnRules() && mLevel->getDifficulty() != NULL;
}

int Ghast::getMaxSpawnClusterSize() { return 1; }

void Ghast::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::GHAST);
}

void Ghast::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(std::wstring(L"ExplosionPower"), mExplosionPower);
}

void Ghast::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag->contains(std::wstring(L"ExplosionPower")))
        mExplosionPower = tag->getInt(std::wstring(L"ExplosionPower"));
}

double Ghast::getEyeHeight() { return 2.6f; }

void Ghast::tick()
{
    Mob::tick();
    if (!mLevel->mIsClientSide)
    {
        if (mLevel->getDifficulty() == Difficulty::PEACEFUL
            || Minecraft::GetInstance()->isTutorial()
            || !gMinecraftServer->mSettings->getBoolean(eServerSetting_SpawnMonsters, true))
        {
            remove();
        }
    }
}

unsigned long long Ghast::GetType() { return EntityType::GHAST; }

Ghast::GhastLookGoal::GhastLookGoal(Ghast* ghast) : Goal(), mGhast(ghast)
{
    setRequiredControlFlags(2);
}
Ghast::GhastLookGoal::~GhastLookGoal() {}
bool Ghast::GhastLookGoal::canUse() { return true; }
void Ghast::GhastLookGoal::tick()
{
    if (!mGhast->getTarget())
    {
        mGhast->mYBodyRot = mGhast->mYRot = -(float)std::atan2(mGhast->mXd, mGhast->mZd) * 57.2957763671875f;
        return;
    }

    boost::shared_ptr<LivingEntity> target = mGhast->getTarget();
    double distance = 64.0;
    if (target->distanceToSqr(mGhast->shared_from_this()) < distance * distance)
    {
        mGhast->mYBodyRot = mGhast->mYRot = -(float)std::atan2(target->mX - mGhast->mX, target->mZ - mGhast->mZ) * 57.2957763671875f;
    }
}
std::wstring Ghast::GhastLookGoal::getName() { return L"GhastLook"; }

Ghast::GhastMoveControl::GhastMoveControl(Ghast* ghast) : MoveControl(ghast), mGhast(ghast), mCountdown(0) {}
Ghast::GhastMoveControl::~GhastMoveControl() {}
bool Ghast::GhastMoveControl::canReach(double x, double y, double z, double distance)
{
    for (int step = 1; (double)step < distance; ++step)
    {
        AABB* box = mGhast->mBoundingBox->move((x - mGhast->mX) / distance * step, (y - mGhast->mY) / distance * step, (z - mGhast->mZ) / distance * step);
        if (!mGhast->mLevel->getCollisionAABBs(mGhast->shared_from_this(), box, false, false, false)->empty())
            return false;
    }
    return true;
}
void Ghast::GhastMoveControl::tick()
{
    if (mOperation != MOVE_TO)
        return;
    double dx = mWantedX - mGhast->mX;
    double dy = mWantedY - mGhast->mY;
    double dz = mWantedZ - mGhast->mZ;
    if (mCountdown-- <= 0)
    {
        mCountdown += mGhast->getRandom()->nextInt(5) + 2;
        double distance = Mth::sqrt(dx * dx + dy * dy + dz * dz);
        if (canReach(mWantedX, mWantedY, mWantedZ, distance))
        {
            mGhast->mXd += dx / distance * 0.1;
            mGhast->mYd += dy / distance * 0.1;
            mGhast->mZd += dz / distance * 0.1;
        }
        else
            mOperation = WAIT;
    }
}

Ghast::GhastShootFireballGoal::GhastShootFireballGoal(Ghast* ghast) : Goal(), mGhast(ghast), mChargeTime(0) {}
Ghast::GhastShootFireballGoal::~GhastShootFireballGoal() {}
bool Ghast::GhastShootFireballGoal::canUse() { return mGhast->getTarget() != boost::shared_ptr<LivingEntity>(); }
void Ghast::GhastShootFireballGoal::start() { mChargeTime = 0; }
void Ghast::GhastShootFireballGoal::stop() { mGhast->setCharging(false); }
void Ghast::GhastShootFireballGoal::tick()
{
    boost::shared_ptr<LivingEntity> target = mGhast->getTarget();
    bool inRangeAndVisible = target->distanceToSqr(mGhast->shared_from_this()) < 64.0 * 64.0 && mGhast->canSee(target);
    if (inRangeAndVisible)
    {
        ++mChargeTime;
        if (mChargeTime == 10)
            mGhast->mLevel->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_GHAST_WARNING, BlockPos(mGhast), 0);
        if (mChargeTime == 20)
        {
            mGhast->mLevel->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_GHAST_FIREBALL, BlockPos(mGhast), 0);
            Vec3* viewVector = mGhast->getViewVector(1.0f);
            boost::shared_ptr<LivingEntity> owner = boost::dynamic_pointer_cast<LivingEntity>(mGhast->shared_from_this());
            double xd = target->mX - (mGhast->mX + viewVector->x * 4.0);
            double yd = (target->mY + (double)(target->mBbHeight * 0.5f)) - (0.5 + (mGhast->mY + (double)(mGhast->mBbHeight * 0.5f)));
            double zd = target->mZ - (mGhast->mZ + viewVector->z * 4.0);
            LargeFireball* fireball = new LargeFireball(mGhast->mLevel, owner, xd, yd, zd);
            fireball->mExplosionPower = mGhast->getExplosionPower();
            fireball->mX = viewVector->x * 4.0 + mGhast->mX;
            fireball->mY = mGhast->mY + (double)(mGhast->mBbHeight * 0.5f) + 0.5;
            fireball->mZ = viewVector->z * 4.0 + mGhast->mZ;
            mGhast->mLevel->addEntity(boost::shared_ptr<Entity>(fireball));
            mChargeTime = -40;
        }
        mGhast->setCharging(mChargeTime > 10);
        return;
    }
    if (mChargeTime > 0)
        --mChargeTime;
    mGhast->setCharging(mChargeTime > 10);
}
std::wstring Ghast::GhastShootFireballGoal::getName() { return L"GhastShootFireball"; }

Ghast::RandomFloatAroundGoal::RandomFloatAroundGoal(Ghast* ghast) : Goal(), mGhast(ghast)
{
    setRequiredControlFlags(1);
}
Ghast::RandomFloatAroundGoal::~RandomFloatAroundGoal() {}
bool Ghast::RandomFloatAroundGoal::canUse()
{
    MoveControl* control = static_cast<MoveControl*>(mGhast->getMoveControl());
    if (control->hasWanted())
    {
        double dx = control->getWantedX() - mGhast->mX;
        double dy = control->getWantedY() - mGhast->mY;
        double dz = control->getWantedZ() - mGhast->mZ;
        double distance = dx * dx + dy * dy + dz * dz;
        if (distance >= 1.0 && distance <= 3600.0)
            return false;
    }
    return true;
}
bool Ghast::RandomFloatAroundGoal::canContinueToUse() { return false; }
void Ghast::RandomFloatAroundGoal::start()
{
    Random* random = mGhast->mRandom;
    MoveControl* control = static_cast<MoveControl*>(mGhast->getMoveControl());
    control->setWantedPosition(mGhast->mX + (random->nextFloat() * 2.0f - 1.0f) * 16.0,
        mGhast->mY + (random->nextFloat() * 2.0f - 1.0f) * 16.0,
        mGhast->mZ + (random->nextFloat() * 2.0f - 1.0f) * 16.0, 1.0);
}
std::wstring Ghast::RandomFloatAroundGoal::getName() { return L"RandomFloatAround"; }
