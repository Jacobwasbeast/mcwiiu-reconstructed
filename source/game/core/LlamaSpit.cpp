#include "game/core/LlamaSpit.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/world/block/Material.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <string>
#include <vector>

#include "game/core/DamageSource.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Llama.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"

LlamaSpit::LlamaSpit(Level* level, const boost::shared_ptr<Llama>& owner)
    : Entity(level, true), Projectile(level, boost::shared_ptr<LivingEntity>())
{
    Entity::fjDerivedCtorCalls();
    _init();
    mLlamaOwner = owner;
    setSize(0.25f, 0.25f);
}

LlamaSpit::LlamaSpit(Level* level)
    : Entity(level, true), Projectile(level, boost::shared_ptr<LivingEntity>())
{
    Entity::fjDerivedCtorCalls();
    _init();
}

LlamaSpit::LlamaSpit(Level* level, double x, double y, double z, double xd, double yd, double zd)
    : Entity(level, true), Projectile(level, boost::shared_ptr<LivingEntity>())
{
    Entity::fjDerivedCtorCalls();
    _init();
    setPos(x, y, z);
    for (int i = 0; i < 7; ++i)
    {
        double scale = 0.4 + 0.1 * i;
        level->addParticle(PARTICLE_SPIT, x, y, z, xd * scale, yd, zd * scale, arrayWithLength<int>());
    }
    mXd = xd;
    mYd = yd;
    mZd = zd;
}

void LlamaSpit::create(Level* level)
{
    new LlamaSpit(level);
}

void LlamaSpit::_init()
{
    mOwnerTag = NULL;
}

void LlamaSpit::shoot(double x, double y, double z, float velocity, float inaccuracy)
{
    double length = std::sqrt(x * x + y * y + z * z);
    double spread = 0.007499999832361937;
    double gx = mRandom->nextGaussian();
    double gy = mRandom->nextGaussian();
    double gz = mRandom->nextGaussian();
    double xd = (gx * spread * inaccuracy + x / length) * velocity;
    double yd = (gy * spread * inaccuracy + y / length) * velocity;
    double zd = (gz * spread * inaccuracy + z / length) * velocity;
    mXd = xd;
    mYd = yd;
    mZd = zd;
    double horizontal = std::sqrt(xd * xd + zd * zd);
    mYRot = (float)(std::atan2(xd, zd) * Mth::RAD_TO_DEG);
    mXRot = (float)(std::atan2(yd, horizontal) * Mth::RAD_TO_DEG);
    mYRotO = mYRot;
    mXRotO = mXRot;
}

void LlamaSpit::shootFromRotation(boost::shared_ptr<Entity> entity, float xRot, float yRot,
    float offset, float velocity, float inaccuracy)
{
    const double radians = 0.01745329238474369;
    shoot(-std::sin(yRot * radians) * std::cos(xRot * radians),
        -std::sin((xRot + offset) * radians),
        std::cos(yRot * radians) * std::cos(xRot * radians), velocity, inaccuracy);
    mXd += entity->mXd;
    mZd += entity->mZd;
    if (!entity->mOnGround)
    {
        mYd += entity->mYd;
    }
}

boost::shared_ptr<Llama> LlamaSpit::getOwner()
{
    return mLlamaOwner.lock();
}

bool LlamaSpit::onHit(const HitResult* result)
{
    Projectile::onHit(result);
    boost::shared_ptr<Llama> owner = getOwner();
    if (result->mEntity && owner)
    {
        DamageSource* source = DamageSource::CreateIndirectMagic(
            shared_from_this(), owner, DamageSource::FALL);
        result->mEntity->hurt(source, 1.0f);
        delete source;
    }
    if (!Entity::mLevel->mIsClientSide)
    {
        remove();
    }
    return true;
}

void LlamaSpit::restoreOwnerFromSave()
{
    if (mOwnerTag != NULL && mOwnerTag->contains(L"OwnerUUID"))
    {
        const std::wstring ownerUuid = mOwnerTag->getString(L"OwnerUUID");
        AABB* searchBox = mBoundingBox->grow(15.0);
        std::vector<boost::shared_ptr<Entity> >* entities =
            Entity::mLevel->getEntitiesOfClass(typeid(Llama), searchBox);
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
             it != entities->end(); ++it)
        {
            boost::shared_ptr<Llama> llama = boost::static_pointer_cast<Llama>(*it);
            if (llama->getUUID() == ownerUuid)
            {
                mLlamaOwner = llama;
                break;
            }
        }
        delete entities;
    }
    mOwnerTag = NULL;
}

boost::shared_ptr<Entity> LlamaSpit::getHitEntity(Vec3* from, Vec3* to)
{
    boost::shared_ptr<Entity> hit;
    std::vector<boost::shared_ptr<Entity> >* entities = Entity::mLevel->getEntities(
        shared_from_this(), mBoundingBox->expand(mXd, mYd, mZd)->grow(1.0), NULL);
    double bestDistance = 0.0;
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
         it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        bool isOwner = entity.get() == mLlamaOwner.lock().get();
        if (!isOwner)
        {
            HitResult* clip = entity->mBoundingBox->grow(0.3f)->clip(from, to);
            if (clip != NULL)
            {
                double distance = from->distanceToSqr(clip->mLocation);
                if (distance < bestDistance || bestDistance == 0.0)
                {
                    hit = entity;
                    bestDistance = distance;
                }
            }
        }
    }
    return hit;
}

void LlamaSpit::tick()
{
    Entity::tick();
    if (mOwnerTag != NULL)
    {
        restoreOwnerFromSave();
    }

    Vec3* from = Vec3::newTemp(mX, mY, mZ);
    Vec3* to = Vec3::newTemp(mX + mXd, mY + mYd, mZ + mZd);
    HitResult* hit = Entity::mLevel->clip(from, to);
    from = Vec3::newTemp(mX, mY, mZ);
    to = Vec3::newTemp(mX + mXd, mY + mYd, mZ + mZd);
    if (hit != NULL)
    {
        to = Vec3::newTemp(hit->mLocation->x, hit->mLocation->y, hit->mLocation->z);
    }

    boost::shared_ptr<Entity> entity = getHitEntity(from, to);
    if (entity != NULL)
    {
        hit = new HitResult(entity);
    }

    if (hit != NULL)
    {
        onHit(hit);
        delete hit;
    }

    mX += mXd;
    mY += mYd;
    mZ += mZd;
    double horizontal = std::sqrt(mXd * mXd + mZd * mZd);
    mYRot = (float)(std::atan2(mXd, mZd) * Mth::RAD_TO_DEG);
    float xRot = (float)(std::atan2(mYd, horizontal) * Mth::RAD_TO_DEG);
    float yRot = mYRot;

    while (xRot - mXRotO < -180.0f)
    {
        mXRotO -= 360.0f;
    }
    while (xRot - mXRotO >= 180.0f)
    {
        mXRotO += 360.0f;
    }
    while (yRot - mYRotO < -180.0f)
    {
        mYRotO -= 360.0f;
    }
    while (yRot - mYRotO >= 180.0f)
    {
        mYRotO += 360.0f;
    }
    mXRot = mXRotO + (xRot - mXRotO) * 0.2f;
    mYRot = mYRotO + (yRot - mYRotO) * 0.2f;

    if (!Entity::mLevel->containsMaterial(mBoundingBox, Material::AIR))
    {
        remove();
    }
    else if (isInWaterOrBubble())
    {
        remove();
    }
    else
    {
        mXd *= 0.99f;
        mYd *= 0.99f;
        mZd *= 0.99f;
        if (!isNoGravity())
        {
            mYd -= 0.06f;
        }
        setPos(mX, mY, mZ);
    }
}

void LlamaSpit::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
    if (mXRotO == 0.0f && mYRotO == 0.0f)
    {
        double horizontal = std::sqrt(xd * xd + zd * zd);
        mXRot = (float)(std::atan2(yd, horizontal) * Mth::RAD_TO_DEG);
        mYRot = (float)(std::atan2(xd, zd) * Mth::RAD_TO_DEG);
        mYRotO = mYRot;
        mXRotO = mXRot;
        moveTo(mX, mY, mZ, mYRot, mXRot);
    }
}

void LlamaSpit::defineSynchedData()
{
}

void LlamaSpit::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(L"Owner", Tag::TAG_COMPOUND))
    {
        mOwnerTag = tag->getCompound(L"Owner")->copy();
    }
}

void LlamaSpit::addAdditonalSaveData(CompoundTag* tag)
{
    boost::shared_ptr<Llama> owner = getOwner();
    if (owner)
    {
        CompoundTag* ownerTag = new CompoundTag();
        std::wstring ownerUuid = owner->getUUID();
        ownerTag->putString(L"OwnerUUID", ownerUuid);
        tag->put(L"Owner", ownerTag);
    }
}

unsigned long long LlamaSpit::GetType()
{
    return EntityType::LLAMA_SPIT;
}

bool LlamaSpit::shouldTick()
{
    return true;
}
