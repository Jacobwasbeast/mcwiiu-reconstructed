#include "game/entity/mob/FireworksRocketEntity.hpp"

#include <cmath>
#include <cstdlib>
#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"

FireworksRocketEntity::FireworksRocketEntity(Level* level, const boost::shared_ptr<Player>& player, const not_null_ptr<ItemInstance>& item)
    : Entity(level, true), mLife(0), mLifetime(0), mAttachedToEntity(), mHasDirectionalData(false), mDirectionalX(0), mDirectionalY(0), mDirectionalZ(0), mDirectionalInitialized(false), mOwner(player)
{
    _init(mX, mY, mZ, item);
    mHasDirectionalData = true;
}

FireworksRocketEntity::FireworksRocketEntity(Level* level, double x, double y, double z, not_null_ptr<ItemInstance> item)
    : Entity(level, true), mLife(0), mLifetime(0), mAttachedToEntity(), mHasDirectionalData(false), mDirectionalX(0), mDirectionalY(0), mDirectionalZ(0), mDirectionalInitialized(false), mOwner()
{
    _init(x, y, z, item);
}

FireworksRocketEntity::FireworksRocketEntity(Level* level, const not_null_ptr<ItemInstance>& item, const boost::shared_ptr<LivingEntity>& entity)
    : Entity(level, true), mLife(0), mLifetime(0), mAttachedToEntity(entity), mHasDirectionalData(false), mDirectionalX(0), mDirectionalY(0), mDirectionalZ(0), mDirectionalInitialized(false), mOwner()
{
    if (entity)
    {
        Entity* attachedEntity = (Entity*)entity.get();
        _init(attachedEntity->mX, attachedEntity->mY, attachedEntity->mZ, item);
        mEntityData->set(DATA_SHARED_FLAGS_ID, attachedEntity->getId(), true);
    }
}

FireworksRocketEntity::FireworksRocketEntity(Level* level)
    : Entity(level, true), mLife(0), mLifetime(0), mAttachedToEntity(), mHasDirectionalData(false), mDirectionalX(0), mDirectionalY(0), mDirectionalZ(0), mDirectionalInitialized(false), mOwner()
{
    fjDerivedCtorCalls();
    setSize(0.25f, 0.25f);
}

FireworksRocketEntity::FireworksRocketEntity(unsigned long long x, unsigned long long y, unsigned long long z, unsigned long long xd, unsigned long long yd, unsigned long long zd, unsigned int, unsigned int, unsigned char attached, unsigned int)
    : Entity(NULL, true), mLife(0), mLifetime(0), mAttachedToEntity(), mHasDirectionalData(attached != 0), mDirectionalX(static_cast<int>(xd)), mDirectionalY(static_cast<int>(yd)), mDirectionalZ(static_cast<int>(zd)), mDirectionalInitialized(false), mOwner()
{
    mX = static_cast<double>(x);
    mY = static_cast<double>(y);
    mZ = static_cast<double>(z);
    mLevel = NULL;
}

FireworksRocketEntity::~FireworksRocketEntity()
{
}

void FireworksRocketEntity::_init(double x, double y, double z, const not_null_ptr<ItemInstance>& item)
{
    setPos(x, y, z);
    setSize(0.25f, 0.25f);
    mLife = 0;
    mLifetime = 6 + static_cast<int>(std::rand() % 6);
    mDirectionalInitialized = false;
}

void FireworksRocketEntity::staticCtor()
{
}

bool FireworksRocketEntity::isAttachedToEntity()
{
    return mAttachedToEntity.get() != NULL;
}

bool FireworksRocketEntity::shouldRenderAtSqrDistance(double distanceSqr)
{
    return distanceSqr < 4096.0;
}

bool FireworksRocketEntity::shouldRender(double x, double y, double z)
{
    return Entity::shouldRender(x, y, z) && !isAttachedToEntity();
}

void FireworksRocketEntity::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
    if (mXRot == 0.0f && mYRot == 0.0f)
    {
        mYRot = static_cast<float>(std::atan2(xd, zd) * 57.2957763671875);
        mYRotO = mYRot;
        double horizontal = Mth::sqrt(xd * xd + zd * zd);
        mXRot = static_cast<float>(std::atan2(yd, horizontal) * 57.2957763671875);
        mXRotO = mXRot;
    }
}

double FireworksRocketEntity::HandleDirectionalAcceleration(int direction, double acceleration)
{
    if (acceleration == 1.0)
    {
        if (mDirectionalInitialized)
        {
            direction -= 0.95;
            mDirectionalInitialized = false;
        }
        return direction + 0.04;
    }
    if (acceleration == -1.0)
    {
        if (mDirectionalInitialized)
        {
            direction += 0.95;
            mDirectionalInitialized = false;
        }
        return direction - 0.04;
    }
    return direction;
}

void FireworksRocketEntity::dealExplosionDamage()
{
    if (mLevel != NULL)
    {
        mLevel->levelEvent(2001, BlockPos(mX, mY, mZ), 0);
    }
}

void FireworksRocketEntity::tick()
{
    ++mLife;
    if (mAttachedToEntity)
    {
        Entity* attachedEntity = (Entity*)mAttachedToEntity.get();
        setPos(attachedEntity->mX, attachedEntity->mY, attachedEntity->mZ);
    }
    else
    {
        mYd += 0.04;
        move(eMoverType_Self, mXd, mYd, mZd, false);
    }
    if (mLife > mLifetime)
    {
        dealExplosionDamage();
        remove();
    }
}

void FireworksRocketEntity::setExplosionColourToPipColour(CompoundTag* tag)
{
    if (tag == NULL)
    {
        return;
    }
    ListTag* explosions = tag->getList(L"Explosions", 10);
    if (explosions == NULL)
    {
        return;
    }
    CompoundTag* explosion = explosions->getCompound(0);
    if (explosion == NULL)
    {
        return;
    }
    int colour = 0;
    arrayWithLength<int> colours;
    colours = arrayWithLength<int>(1, true);
    colours.data[0] = colour;
    explosion->putIntArray(L"Colors", colours);
}

void FireworksRocketEntity::handleEntityEvent(unsigned char event, int data)
{
    if (event == 17)
    {
        dealExplosionDamage();
    }
    else
    {
        Entity::handleEntityEvent(event, data);
    }
}

void FireworksRocketEntity::addDataWalkers(DataFixerUpper*)
{
}

void FireworksRocketEntity::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"Life", mLife);
    tag->putInt(L"LifeTime", mLifetime);
}

void FireworksRocketEntity::readAdditionalSaveData(CompoundTag* tag)
{
    mLife = tag->getInt(L"Life");
    mLifetime = tag->getInt(L"LifeTime");
}

bool FireworksRocketEntity::isAttackable()
{
    return false;
}

boost::shared_ptr<LivingEntity> FireworksRocketEntity::getOwner() const
{
    return mAttachedToEntity;
}

void FireworksRocketEntity::updateDeltaAfterMove(Block* block, double x, double y, double z, double& movedX, double& movedY, double& movedZ)
{
    Entity::updateDeltaAfterMove(block, x, y, z, movedX, movedY, movedZ);
}

void FireworksRocketEntity::defineSynchedData()
{
    Entity::defineSynchedData();
}

void FireworksRocketEntity::create(Level* level)
{
    new FireworksRocketEntity(level);
}

unsigned long long FireworksRocketEntity::GetType()
{
    return 0x88000005ULL;
}

bool FireworksRocketEntity::shouldTick()
{
    return true;
}
