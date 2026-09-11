#include "game/entity/mob/FireworksRocketEntity.hpp"

#include <cmath>
#include <cstdlib>
#include <string>

#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"

const EntityDataAccessor<not_null_ptr<ItemInstance> >* FireworksRocketEntity::DATA_ITEM = NULL;
const EntityDataAccessor<int>* FireworksRocketEntity::DATA_ATTACHED_TO_ENTITY = NULL;

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
    DATA_ITEM = SynchedEntityData::defineId<not_null_ptr<ItemInstance> >(
        0x88000005ULL, 0x08000000ULL, NULL);
    DATA_ATTACHED_TO_ENTITY = SynchedEntityData::defineId<int>(
        0x88000005ULL, 0x08000000ULL, NULL);
}

bool FireworksRocketEntity::isAttachedToEntity()
{
    return !getSharedFlag(31);
}

bool FireworksRocketEntity::shouldRenderAtSqrDistance(double distanceSqr)
{
    return distanceSqr < 4096.0 && !isAttachedToEntity();
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
    if (event != 17)
    {
        Entity::handleEntityEvent(event, data);
        return;
    }

    not_null_ptr<ItemInstance> item = mEntityData->get(DATA_ITEM);
    CompoundTag* fireworksTag = NULL;

    if (!item->isEmpty())
    {
        fireworksTag = static_cast<CompoundTag*>(item->getTagElement(std::wstring(L"Fireworks")));
        if (fireworksTag != NULL
            && Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
        {
            setExplosionColourToPipColour(fireworksTag);
        }
    }

    mLevel->createFireworks(mX, mY, mZ, mXd, mYd, mZd, fireworksTag);
}

void FireworksRocketEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity,
                     new ItemDataWalker(0x88000005ULL, L"FireworksItem"));
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
    if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks()
        || !mHasDirectionalData)
    {
        Entity::updateDeltaAfterMove(block, x, y, z, movedX, movedY, movedZ);
    }
}

void FireworksRocketEntity::defineSynchedData()
{
    Entity::defineSynchedData();
    mEntityData->createDateItem(DATA_ITEM, ItemInstance::EMPTY);
    mEntityData->createDateItem(DATA_ATTACHED_TO_ENTITY, -1);
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
