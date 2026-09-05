#include "game/core/LlamaSpit.hpp"

#include <cmath>
#include <string>
#include <vector>

#include "game/core/DamageSource.hpp"
#include "game/core/HitResult.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"

LlamaSpit::LlamaSpit(Level* level, const boost::shared_ptr<Llama>& owner)
    : Entity(level, true), Projectile(level, boost::shared_ptr<LivingEntity>())
{
    _init();
    mLlamaOwner = owner;
    if (owner)
    {
        setSize(0.25f, 0.25f);
    }
}

LlamaSpit::LlamaSpit(Level* level)
    : Entity(level, true), Projectile(level, boost::shared_ptr<LivingEntity>())
{
    _init();
}

LlamaSpit::LlamaSpit(Level* level, double x, double y, double z, double xd, double yd, double zd)
    : Entity(level, true), Projectile(level, boost::shared_ptr<LivingEntity>())
{
    _init();
    setPos(x, y, z);
    mXd = xd;
    mYd = yd;
    mZd = zd;
}

LlamaSpit::~LlamaSpit()
{
    delete mOwnerTag;
}

void LlamaSpit::create(Level* level)
{
    new LlamaSpit(level);
}

void LlamaSpit::_init()
{
    mLlamaOwner = boost::shared_ptr<Llama>();
    mOwnerTag = NULL;
}

void LlamaSpit::shoot(double x, double y, double z, float velocity, float inaccuracy)
{
    double length = std::sqrt(x * x + y * y + z * z);
    double spread = 0.007499999832361937;
    double gx = mRandom->nextGaussian();
    double gy = mRandom->nextGaussian();
    double gz = mRandom->nextGaussian();
    mXd = (gx * spread * inaccuracy + x / length) * velocity;
    mYd = (gy * spread * inaccuracy + y / length) * velocity;
    mZd = (gz * spread * inaccuracy + z / length) * velocity;
    double horizontal = std::sqrt(mXd * mXd + mZd * mZd);
    mYRot = (float)(std::atan2(mXd, mZd) * 57.2957763671875);
    mYRotO = mYRot;
    mXRot = (float)(std::atan2(mYd, horizontal) * 57.2957763671875);
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
    return mLlamaOwner;
}

bool LlamaSpit::onHit(const HitResult* result)
{
    Projectile::onHit(result);
    boost::shared_ptr<Llama> owner = getOwner();
    if (result->mEntity && owner)
    {
        boost::shared_ptr<Entity> ownerEntity;
        boost::shared_ptr<LivingEntity> livingOwner;
        boost::shared_ptr<DamageSource> source = DamageSource::CreateIndirectMagic(
            ownerEntity, result->mEntity, NULL);
        result->mEntity->hurt(source.get(), 1.0f);
        result->mEntity->markHurt();
    }
    if (!Entity::mLevel->mIsClientSide)
    {
        remove();
    }
    return true;
}

void LlamaSpit::restoreOwnerFromSave()
{
    if (mOwnerTag == NULL)
    {
        return;
    }
}

boost::shared_ptr<Entity> LlamaSpit::getHitEntity(Vec3* from, Vec3* to)
{
    AABB* box = mBoundingBox->expand(to->x - from->x, to->y - from->y, to->z - from->z)->grow(1.0);
    std::vector<boost::shared_ptr<Entity> > entities;
    if (Entity::mLevel != NULL)
    {
        std::vector<boost::shared_ptr<Entity> >* found =
            Entity::mLevel->getEntitiesOfClass(typeid(Entity), box);
        if (found != NULL)
        {
            entities = *found;
        }
    }
    double bestDistance = 1.0;
    boost::shared_ptr<Entity> hit;
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities.begin(); it != entities.end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        if (entity.get() == this || !entity->isPickable())
        {
            continue;
        }
        AABB* entityBox = entity->mBoundingBox->grow(0.3);
        HitResult* clip = entityBox->clip(from, to);
        if (clip != NULL)
        {
            double distance = from->distanceToSqr(clip->mLocation);
            if (distance < bestDistance)
            {
                bestDistance = distance;
                hit = entity;
            }
        }
    }
    return hit;
}

void LlamaSpit::tick()
{
    baseTick();
    Vec3* from = Vec3::newTemp(mX, mY, mZ);
    Vec3* to = Vec3::newTemp(mX + mXd, mY + mYd, mZ + mZd);
    HitResult* hit = Entity::mLevel->clip(from, to);
    if (hit == NULL || hit->mType == HitResult::MISS)
    {
        boost::shared_ptr<Entity> entity = getHitEntity(from, to);
        if (entity)
        {
            hit = new HitResult(entity);
        }
    }
    if (hit != NULL)
    {
        onHit(hit);
        delete hit;
        return;
    }
    mX += mXd;
    mY += mYd;
    mZ += mZd;
    setPos(mX, mY, mZ);
    double horizontal = std::sqrt(mXd * mXd + mZd * mZd);
    mYRot = (float)(std::atan2(mXd, mZd) * 57.2957763671875);
    mXRot = (float)(std::atan2(mYd, horizontal) * 57.2957763671875);
    mXd *= 0.99;
    mYd *= 0.99;
    mZd *= 0.99;
    mYd -= 0.06;
}

void LlamaSpit::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
    if (mXRot == 0.0f && mYRot == 0.0f)
    {
        double horizontal = std::sqrt(xd * xd + zd * zd);
        mYRot = (float)(std::atan2(xd, zd) * 57.2957763671875);
        mYRotO = mYRot;
        mXRot = (float)(std::atan2(yd, horizontal) * 57.2957763671875);
        mXRotO = mXRot;
    }
}

void LlamaSpit::defineSynchedData()
{
}

void LlamaSpit::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(L"Owner", 10))
    {
        mOwnerTag = tag->getCompound(L"Owner");
    }
}

void LlamaSpit::addAdditonalSaveData(CompoundTag* tag)
{
    boost::shared_ptr<Llama> owner = getOwner();
    if (owner)
    {
        CompoundTag* ownerTag = new CompoundTag();
        tag->putString(L"OwnerUUID", L"");
        tag->put(L"Owner", ownerTag);
    }
}

unsigned long long LlamaSpit::GetType()
{
    return 0x8800001200000000ULL;
}

bool LlamaSpit::shouldTick()
{
    return true;
}
