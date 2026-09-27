#include "game/core/ShulkerBullet.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/Direction.hpp"
#include "game/core/HitResult.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/projectile/ProjectileUtil.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/Level.hpp"

ShulkerBullet::ShulkerBullet(Level* level)
    : Entity(level, true), mOwner(), mTarget(), mMoveDirection(NULL), mSteps(0),
      mPowerX(0.0), mPowerY(0.0), mPowerZ(0.0), mOwnerUniqueId(), mOwnerPos(),
      mTargetUniqueId(), mTargetPos()
{
    init();
    setSize(0.3125f, 0.3125f);
    mNoPhysics = true;
}

ShulkerBullet::ShulkerBullet(Level* level, double x, double y, double z,
                             double xd, double yd, double zd)
    : Entity(level, true), mOwner(), mTarget(), mMoveDirection(NULL), mSteps(0),
      mPowerX(0.0), mPowerY(0.0), mPowerZ(0.0), mOwnerUniqueId(), mOwnerPos(),
      mTargetUniqueId(), mTargetPos()
{
    init();
    moveTo(x, y, z, mYRot, mXRot);
    mXd = xd;
    mYd = yd;
    mZd = zd;
}

ShulkerBullet::ShulkerBullet(Level* level, const boost::shared_ptr<LivingEntity>& owner,
                             const boost::shared_ptr<Entity>& target,
                             const Direction::Axis* axis)
    : Entity(level, true), mOwner(owner), mTarget(target), mMoveDirection(NULL), mSteps(0),
      mPowerX(0.0), mPowerY(0.0), mPowerZ(0.0), mOwnerUniqueId(), mOwnerPos(),
      mTargetUniqueId(), mTargetPos()
{
    init();
    BlockPos pos(owner.get());
    moveTo(pos.x + 0.5, pos.y + 0.5, pos.z + 0.5, mYRot, mXRot);
    mMoveDirection = Direction::getRandomDirection(*mRandom);
    selectNextMoveDirection(axis);
}

ShulkerBullet::~ShulkerBullet()
{
}

void ShulkerBullet::create(Level* level)
{
    new ShulkerBullet(level);
}

void ShulkerBullet::init()
{
    fjDerivedCtorCalls();
    mOwner.reset();
    mTarget.reset();
    mMoveDirection = Direction::UP;
    mSteps = 0;
    mPowerX = 0.0;
    mPowerY = 0.0;
    mPowerZ = 0.0;
    mTargetUniqueId.clear();
    mTargetPos = BlockPos();
    mOwnerUniqueId.clear();
    mOwnerPos = BlockPos();
    defineSynchedData();
}

void ShulkerBullet::setMoveDirection(const Direction* direction)
{
    mMoveDirection = direction;
}

void ShulkerBullet::selectNextMoveDirection(const Direction::Axis* axis)
{
    BlockPos current(this);
    BlockPos destination = mTarget ? BlockPos(mTarget) : current.below();
    std::vector<const Direction*> choices;
    if (axis != Direction::Axis::X && destination.x != current.x)
        choices.push_back(destination.x < current.x ? Direction::WEST : Direction::EAST);
    if (axis != Direction::Axis::Y && destination.y != current.y)
        choices.push_back(destination.y < current.y ? Direction::DOWN : Direction::UP);
    if (axis != Direction::Axis::Z && destination.z != current.z)
        choices.push_back(destination.z < current.z ? Direction::NORTH : Direction::SOUTH);
    if (choices.empty())
        choices.push_back(Direction::getRandomDirection(*mRandom));
    const Direction* direction = choices[mRandom->nextInt((int)choices.size())];
    setMoveDirection(direction);
    double dx = current.x + 0.5 - mX;
    double dy = current.y + 0.5 - mY;
    double dz = current.z + 0.5 - mZ;
    double length = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (length == 0.0)
    {
        mPowerX = mPowerY = mPowerZ = 0.0;
    }
    else
    {
        mPowerX = dx / length * 0.2;
        mPowerY = dy / length * 0.2;
        mPowerZ = dz / length * 0.2;
    }
    mSteps = mRandom->nextInt(5) * 10 + 10;
    mOnGround = true;
}

void ShulkerBullet::onHit(const HitResult* result)
{
    if (!result->mEntity)
    {
        static_cast<ServerLevel*>(mLevel)->sendParticles(PARTICLE_LARGE_EXPLODE, mX, mY, mZ, 2, 0.2, 0.2, 0.2, 0.0,
                                                         arrayWithLength<int>());
        playSound(SoundEvents::SHULKER_BULLET_HIT, 1.0f, 1.0f);
    }
    else
    {
        boost::shared_ptr<DamageSource> damageSource = DamageSource::CreateIndirectMobAttack(shared_from_this(), mOwner, true);
        if (result->mEntity->hurt(damageSource.get(), 4.0f))
        {
            doEnchantDamageEffects(mOwner, result->mEntity);
            if ((result->mEntity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
            {
                boost::shared_ptr<LivingEntity> target = boost::static_pointer_cast<LivingEntity>(result->mEntity);
                target->addEffect(new MobEffectInstance(MobEffects::LEVITATION, 200), shared_from_this());
            }
        }
    }
    remove();
}

void ShulkerBullet::tick()
{
    if (!mLevel->mIsClientSide && mLevel->getDifficulty() == Difficulty::PEACEFUL)
    {
        remove();
        return;
    }
    Entity::tick();
    if (mRemoved)
        return;
    if (!mLevel->mIsClientSide)
    {
        mXd += (mPowerX - mXd) * 0.2;
        mYd += (mPowerY - mYd) * 0.2;
        mZd += (mPowerZ - mZd) * 0.2;
    }
    setPos(mX + mXd, mY + mYd, mZ + mZd);
    if (mMoveDirection != NULL)
        ProjectileUtil::rotateTowardsMovement(shared_from_this(), 0.5f);
    if (!mLevel->mIsClientSide)
    {
        if (mSteps-- <= 0)
            selectNextMoveDirection(mMoveDirection ? mMoveDirection->getAxis() : NULL);
    }
}

SoundSource::ESoundSource ShulkerBullet::getSoundSource()
{
    return SoundSource::eSoundSource_Hostile;
}

void ShulkerBullet::addAdditonalSaveData(CompoundTag* tag)
{
    if (mOwner != NULL)
    {
        BlockPos pos(boost::static_pointer_cast<Entity>(mOwner));
        CompoundTag* owner = new CompoundTag();
        owner->putString(L"UUID", mOwner->getUUID());
        owner->putInt(L"X", pos.x);
        owner->putInt(L"Y", pos.y);
        owner->putInt(L"Z", pos.z);
        tag->put(L"Owner", owner);
    }
    if (mTarget != NULL)
    {
        BlockPos pos(mTarget);
        CompoundTag* target = new CompoundTag();
        target->putString(L"UUID", mTarget->getUUID());
        target->putInt(L"X", pos.x);
        target->putInt(L"Y", pos.y);
        target->putInt(L"Z", pos.z);
        tag->put(L"Target", target);
    }
    if (mMoveDirection != NULL)
        tag->putInt(L"Dir", mMoveDirection->get3DDataValue());
    tag->putInt(L"Steps", mSteps);
    tag->putDouble(L"TXD", mPowerX);
    tag->putDouble(L"TYD", mPowerY);
    tag->putDouble(L"TZD", mPowerZ);
}

void ShulkerBullet::readAdditionalSaveData(CompoundTag* tag)
{
    mSteps = tag->getInt(L"Steps");
    mPowerX = tag->getDouble(L"TXD");
    mPowerY = tag->getDouble(L"TYD");
    mPowerZ = tag->getDouble(L"TZD");
    if (tag->contains(L"Dir", Tag::TAG_ANY_NUMERIC))
        mMoveDirection = Direction::from3DDataValue(tag->getInt(L"Dir"));
    if (tag->contains(L"Owner", Tag::TAG_COMPOUND))
    {
        CompoundTag* owner = tag->getCompound(L"Owner");
        mOwnerUniqueId = owner->getString(L"UUID");
        mOwnerPos = BlockPos(owner->getInt(L"X"), owner->getInt(L"Y"), owner->getInt(L"Z"));
    }
    if (tag->contains(L"Target", Tag::TAG_COMPOUND))
    {
        CompoundTag* target = tag->getCompound(L"Target");
        mTargetUniqueId = target->getString(L"UUID");
        mTargetPos = BlockPos(target->getInt(L"X"), target->getInt(L"Y"), target->getInt(L"Z"));
    }
}

bool ShulkerBullet::shouldRenderAtSqrDistance(double distanceSqr)
{
    return distanceSqr < 16384.0;
}

float ShulkerBullet::getBrightness()
{
    return 1.0f;
}

int ShulkerBullet::getLightColor()
{
    return 0xf000f0;
}

bool ShulkerBullet::isPickable()
{
    return true;
}

bool ShulkerBullet::hurt(const DamageSource* source, float damage)
{
    if (!mLevel->mIsClientSide)
    {
        remove();
    }
    return true;
}

bool ShulkerBullet::isOnFire()
{
    return false;
}

void ShulkerBullet::defineSynchedData()
{
}

unsigned long long ShulkerBullet::GetType()
{
    return EntityType::SHULKER_BULLET;
}
