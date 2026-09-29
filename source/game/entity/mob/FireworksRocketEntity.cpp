#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/GameMode.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <cstdlib>
#include <string>
#include <typeinfo>
#include <vector>

#include "game/client/render/ParticleType.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

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
        mEntityData->set(DATA_ATTACHED_TO_ENTITY, attachedEntity->getId(), true);
    }
}

FireworksRocketEntity::FireworksRocketEntity(Level* level)
    : Entity(level, true), mLife(0), mLifetime(0), mAttachedToEntity(), mHasDirectionalData(false), mDirectionalX(0), mDirectionalY(0), mDirectionalZ(0), mDirectionalInitialized(false), mOwner()
{
    fjDerivedCtorCalls();
    setSize(0.25f, 0.25f);
}

FireworksRocketEntity::FireworksRocketEntity(Level* level, double x, double y, double z, double xd, double yd, double zd,
                                             not_null_ptr<ItemInstance> item, bool hasDirectionalData,
                                             const boost::shared_ptr<Player>& owner)
    : Entity(level, true), mLife(0), mLifetime(0), mAttachedToEntity(), mHasDirectionalData(false), mDirectionalX(0), mDirectionalY(0), mDirectionalZ(0), mDirectionalInitialized(false), mOwner()
{
    _init(x, y, z, item);
    mDirectionalX = static_cast<int>(Math::round(xd));
    mDirectionalZ = static_cast<int>(Math::round(zd));
    mDirectionalY = static_cast<int>(Math::round(yd));
    mXd = xd;
    mYd = yd;
    mZd = zd;
    mDirectionalInitialized = true;
    mHasDirectionalData = hasDirectionalData;
    mOwner = owner;
}

FireworksRocketEntity::~FireworksRocketEntity()
{
}

void FireworksRocketEntity::_init(double x, double y, double z, const not_null_ptr<ItemInstance>& item)
{
    defineSynchedData();
    fjDerivedCtorCalls();
    setSize(0.25f, 0.25f);
    mLife = 0;
    setPos(x, y, z);
    mHasDirectionalData = true;

    int flightDuration = 1;
    if (!item->isEmpty() && item->hasTag())
    {
        bool directional = Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks();
        mEntityData->set(DATA_ITEM, item->copy(), !directional);
        CompoundTag* fireworksTag = item->getTag()->getCompound(L"Fireworks");
        if (fireworksTag != NULL)
        {
            flightDuration = fireworksTag->getByte(L"Flight") + 1;
        }
    }

    mXd = mRandom->nextGaussian() * 0.001;
    mZd = mRandom->nextGaussian() * 0.001;
    mYd = 0.05;
    mDirectionalX = 0;
    mDirectionalZ = 0;

    mLifetime = 40;
    if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
    {
        mLifetime = mRandom->nextInt(6) + mRandom->nextInt(7) + flightDuration * 10;
    }

    mOwner = boost::shared_ptr<Player>();
}

void FireworksRocketEntity::staticCtor()
{
    DATA_ATTACHED_TO_ENTITY = SynchedEntityData::defineId<int>(
        EntityType::FIREWORKS_ROCKET_ENTITY, EntityType::ENTITY, EntityDataSerializers::INT);
    DATA_ITEM = SynchedEntityData::defineId<not_null_ptr<ItemInstance> >(
        EntityType::FIREWORKS_ROCKET_ENTITY, EntityType::ENTITY, EntityDataSerializers::ITEM_INSTANCE);
}

bool FireworksRocketEntity::isAttachedToEntity()
{
    return mEntityData->get(DATA_ATTACHED_TO_ENTITY) >= 0;
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
    if (mXRotO == 0.0f && mYRotO == 0.0f)
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
    not_null_ptr<ItemInstance> item = mEntityData->get(DATA_ITEM);
    if (item->isEmpty())
    {
        return;
    }
    CompoundTag* fireworksTag = static_cast<CompoundTag*>(item->getTagElement(std::wstring(L"Fireworks")));
    if (fireworksTag == NULL)
    {
        return;
    }
    ListTag* explosions = fireworksTag->getList(L"Explosions", Tag::TAG_COMPOUND);
    float power = 5.0f;
    if (explosions != NULL && !explosions->isEmpty())
    {
        power = static_cast<float>(explosions->size()) * 2.0f + 5.0f;
    }
    if (power <= 0.0f)
    {
        return;
    }

    boost::shared_ptr<LivingEntity> attached = mAttachedToEntity.lock();
    boost::shared_ptr<DamageSource> explosionSource = DamageSource::CreateExplosion(attached);
    if (attached)
    {
        attached->hurt(explosionSource.get(), power);
    }

    double radius = 5.0;
    Vec3* center = Vec3::newTemp(mX, mY, mZ);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntitiesOfClass(typeid(LivingEntity), mBoundingBox->grow(radius));
    if (entities != NULL)
    {
        std::vector<boost::shared_ptr<Entity> >::iterator it;
        for (it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(*it);
            if (living == attached)
            {
                continue;
            }
            double distSq = distanceToSqr(*it);
            if (radius * radius < distSq)
            {
                continue;
            }
            Vec3* toFeet = Vec3::newTemp(living->mX, living->mY, living->mZ);
            HitResult* hit = mLevel->clip(center, toFeet, false, true, false, false, NULL, false);
            bool visible = (hit == NULL || hit->mType == HitResult::MISS);
            if (!visible)
            {
                Vec3* toMid = Vec3::newTemp(living->mX,
                    living->mY + static_cast<double>(living->mBbHeight) * 0.5,
                    living->mZ);
                hit = mLevel->clip(center, toMid, false, true, false, false, NULL, false);
                visible = (hit == NULL || hit->mType == HitResult::MISS);
            }
            if (visible)
            {
                double falloff = std::sqrt((radius - distanceTo(*it)) / radius);
                living->hurt(explosionSource.get(), power * static_cast<float>(falloff));
            }
        }
        delete entities;
    }
}

void FireworksRocketEntity::tick()
{
    PIXBeginNamedEvent(0.0f, "Fireworks rocket entity tick");

    mXOld = mX;
    mYOld = mY;
    mZOld = mZ;
    Entity::tick();

    if (!isAttachedToEntity())
    {
        if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
        {
            if (mDirectionalX != 0 || mDirectionalY != 0 || mDirectionalZ != 0)
            {
                mXd = HandleDirectionalAcceleration(mDirectionalX, mXd);
                mYd = HandleDirectionalAcceleration(mDirectionalY, mYd);
                mZd = HandleDirectionalAcceleration(mDirectionalZ, mZd);
            }
            else
            {
                mYd += 0.04;
            }
        }

        move(eMoverType_Self, mXd, mYd, mZd, false);
    }
    else
    {
        boost::shared_ptr<LivingEntity> attached = mAttachedToEntity.lock();

        if (attached == NULL)
        {
            boost::shared_ptr<Entity> entity = mLevel->getEntity(mEntityData->get(DATA_ATTACHED_TO_ENTITY));

            if (entity != NULL && (entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
            {
                attached = boost::static_pointer_cast<LivingEntity>(entity);
            }
        }

        if (attached != NULL)
        {
            if (attached->isFallFlying())
            {
                Vec3* look = attached->getLookAngle();
                attached->mXd += look->x * 0.1 + (look->x * 1.5 - attached->mXd) * 0.5;
                attached->mYd += look->y * 0.1 + (look->y * 1.5 - attached->mYd) * 0.5;
                attached->mZd += look->z * 0.1 + (look->z * 1.5 - attached->mZd) * 0.5;
            }

            setPos(attached->mX, attached->mY, attached->mZ);
            mXd = attached->mXd;
            mYd = attached->mYd;
            mZd = attached->mZd;
        }
    }

    double facingX;
    double facingY;
    double facingZ;

    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
    {
        facingX = mXd;
        facingY = mYd;
        facingZ = mZd;
    }
    else if (mDirectionalX != 0 || mDirectionalY != 0 || mDirectionalZ != 0)
    {
        facingX = mDirectionalX;
        facingY = mDirectionalY;
        facingZ = mDirectionalZ;
    }
    else
    {
        facingX = 0.0;
        facingY = 1.0;
        facingZ = 0.0;
    }

    double horizontal = Mth::sqrt(facingX * facingX + facingZ * facingZ);
    mYRot = (float)(atan2(facingX, facingZ) * 57.2957763671875f);
    mXRot = (float)(atan2(facingY, horizontal) * 57.2957763671875f);

    while (mXRot - mXRotO < -180.0f)
    {
        mXRotO -= 360.0f;
    }

    while (mXRot - mXRotO >= 180.0f)
    {
        mXRotO += 360.0f;
    }

    while (mYRot - mYRotO < -180.0f)
    {
        mYRotO -= 360.0f;
    }

    while (mYRot - mYRotO >= 180.0f)
    {
        mYRotO += 360.0f;
    }

    mXRot = mXRotO + (mXRot - mXRotO) * 0.2f;
    mYRot = mYRotO + (mYRot - mYRotO) * 0.2f;

    if (mLife == 0 && mLevel->mIsClientSide && !isSilent())
    {
        mLevel->playLocalSound(mX, mY, mZ, SoundEvents::FIREWORK_LAUNCH, SoundSource::eSoundSource_Ambient, 3.0f, 1.0f,
                               false, 16.0f);
    }

    ++mLife;

    int particleInterval = 2;
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
    {
        particleInterval = 4;
    }

    if (mLevel->mIsClientSide)
    {
        if (mLife % particleInterval == 0)
        {
            double py;
            double vx;
            double vy;
            double vz;

            if (mDirectionalX == 0 && mDirectionalY == 0 && mDirectionalZ == 0
                && !Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
            {
                py = mY - 0.3;
                vy = -(mYd * 0.5);
                vz = mRandom->nextGaussian() * 0.05;
                vx = mRandom->nextGaussian() * 0.05;
            }
            else
            {
                py = mY;
                double drag = Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks() ? 0.1 : 0.5;
                vy = mRandom->nextGaussian() * 0.05 - mYd * drag;
                vz = mRandom->nextGaussian() * 0.05 - mZd * drag;
                vx = mRandom->nextGaussian() * 0.05 - mXd * drag;
            }

            if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
            {
                mLevel->addParticle(PARTICLE_FIREWORKS_SPARK, mX, py, mZ, vx, vy, vz, arrayWithLength<int>());
            }
            else
            {
                int playerIndex = mOwner != NULL ? mOwner->mPlayerIndex : 0;
                unsigned int nametagColour = PlayerRenderer::getNametagColour(playerIndex);
                if (playerIndex == 0)
                {
                    nametagColour = 0xffffffff;
                }

                arrayWithLength<int> colour(4, true);
                colour.data[0] = (nametagColour >> 16) & 0xff;
                colour.data[1] = (nametagColour >> 8) & 0xff;
                colour.data[2] = nametagColour & 0xff;
                colour.data[3] = 0xff;
                mLevel->addParticle(PARTICLE_FIREWORKS_SPARK, mX, py, mZ, vx, vy, vz, colour);
            }
        }
    }
    else
    {
        if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks() && mCollision)
        {
            if (mOwner != NULL)
            {
                bool mayDestroy = boost::static_pointer_cast<Player>(mOwner)->GetGameMode()->MayDestroy(BlockPos(mX, mY, mZ));
                mLevel->explode(shared_from_this(), mX, mY, mZ, 2.0f, false, mayDestroy, false, false, false, -1.0f);
            }

            remove();
        }

        if (!mLevel->mIsClientSide && mLife > mLifetime)
        {
            dealExplosionDamage();
            remove();
        }
    }

    if (mRemoved && !mLevel->mIsClientSide)
    {
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::FIREWORKS_EXPLODE, 0);
    }

    PIXEndNamedEvent();
}

void FireworksRocketEntity::setExplosionColourToPipColour(CompoundTag* tag)
{
    if (tag == NULL)
    {
        return;
    }
    ListTag* explosions = tag->getList(L"Explosions", Tag::TAG_COMPOUND);
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
    if (event != EntityEvent::FIREWORKS_EXPLODE)
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
                     new ItemDataWalker(EntityType::FIREWORKS_ROCKET_ENTITY, L"FireworksItem"));
}

void FireworksRocketEntity::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"Life", mLife);
    tag->putInt(L"LifeTime", mLifetime);
    not_null_ptr<ItemInstance> item = mEntityData->get(DATA_ITEM);
    if (!item->isEmpty())
    {
        CompoundTag* itemTag = item->save(new CompoundTag());
        tag->put(L"FireworksItem", itemTag);
    }
}

void FireworksRocketEntity::readAdditionalSaveData(CompoundTag* tag)
{
    mLife = tag->getInt(L"Life");
    mLifetime = tag->getInt(L"LifeTime");
    CompoundTag* itemTag = tag->getCompound(L"FireworksItem");
    if (itemTag != NULL)
    {
        not_null_ptr<ItemInstance> item(new ItemInstance(itemTag));
        if (!item->isEmpty())
        {
            mEntityData->set(DATA_ITEM, item, true);
        }
    }
}

bool FireworksRocketEntity::isAttackable()
{
    return false;
}

boost::shared_ptr<LivingEntity> FireworksRocketEntity::getOwner() const
{
    return mOwner;
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
    mEntityData->createDateItem(DATA_ITEM, ItemInstance::EMPTY);
    mEntityData->createDateItem(DATA_ATTACHED_TO_ENTITY, -1);
}

void FireworksRocketEntity::create(Level* level)
{
    new FireworksRocketEntity(level);
}

unsigned long long FireworksRocketEntity::GetType()
{
    return EntityType::FIREWORKS_ROCKET_ENTITY;
}

bool FireworksRocketEntity::shouldTick()
{
    return true;
}
