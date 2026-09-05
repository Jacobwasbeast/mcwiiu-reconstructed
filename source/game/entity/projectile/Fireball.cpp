#include "game/entity/projectile/Fireball.hpp"

#include <cmath>
#include <string>
#include <vector>

#include "game/core/HitResult.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/ListTag.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"

class LivingEntity : public Entity
{
};

class DamageSource
{
public:
    boost::shared_ptr<Entity> getDirectEntity() const;
};

class ProjectileUtil
{
public:
    static HitResult* forwardsRaycast(const boost::shared_ptr<Entity>& entity,
                                      bool checkEntities,
                                      const std::vector<boost::shared_ptr<Entity> >* ignoredEntities,
                                      const std::vector<unsigned long long>* ignoredEntityTypes);
    static void rotateTowardsMovement(const boost::shared_ptr<Entity>& entity, float rotationSpeed);
};

Fireball::Fireball(Level* level)
    : Entity(level, true), mOwner(), mLife(0), mUnknown(0), mPowerX(0.0), mPowerY(0.0), mPowerZ(0.0)
{
    _init();
    setSize(1.0f, 1.0f);
}

Fireball::Fireball(Level* level, double x, double y, double z, double xd, double yd, double zd)
    : Entity(level, true), mOwner(), mLife(0), mUnknown(0), mPowerX(0.0), mPowerY(0.0), mPowerZ(0.0)
{
    _init();
    setSize(1.0f, 1.0f);
    moveTo(x, y, z, mYRot, mXRot);
    setPos(x, y, z);
    const double length = std::sqrt(xd * xd + yd * yd + zd * zd);
    if (length == 0.0)
    {
        mPowerX = 0.0;
        mPowerY = 0.0;
        mPowerZ = 0.0;
    }
    else
    {
        mPowerX = xd / length * 0.1;
        mPowerY = yd / length * 0.1;
        mPowerZ = zd / length * 0.1;
    }
}

Fireball::Fireball(Level* level, boost::shared_ptr<LivingEntity> owner, double xd, double yd, double zd)
    : Entity(level, true), mOwner(), mLife(0), mUnknown(0), mPowerX(0.0), mPowerY(0.0), mPowerZ(0.0)
{
    _init();
    setSize(1.0f, 1.0f);
    if (owner)
    {
        mOwner = owner;
        moveTo(owner->mX, owner->mY, owner->mZ, owner->mYRot, owner->mXRot);
    }
    setPos(mX, mY, mZ);
    const double spread = 0.1;
    xd += mRandom->nextGaussian() * spread;
    yd += mRandom->nextGaussian() * spread;
    zd += mRandom->nextGaussian() * spread;
    const double length = std::sqrt(xd * xd + yd * yd + zd * zd);
    if (length == 0.0)
    {
        mPowerX = 0.0;
        mPowerY = 0.0;
        mPowerZ = 0.0;
    }
    else
    {
        mPowerX = xd / length * 0.1;
        mPowerY = yd / length * 0.1;
        mPowerZ = zd / length * 0.1;
    }
}

Fireball::~Fireball()
{
}

bool Fireball::shouldTick()
{
    return true;
}

void Fireball::addDataWalkers(DataFixerUpper*, unsigned long long)
{
}

void Fireball::_init()
{
    mOwner.reset();
    mLife = 0;
    mUnknown = 0;
    mPowerX = 0.0;
    mPowerY = 0.0;
    mPowerZ = 0.0;
}

void Fireball::defineSynchedData()
{
}

bool Fireball::shouldRenderAtSqrDistance(double distanceSqr)
{
    const double size = mBoundingBox->getSize();
    const double distance = size * 4.0 * 64.0;
    return distanceSqr < distance * distance;
}

void Fireball::tick()
{
    if (mLevel == NULL || mLevel->mIsClientSide || mLevel->hasChunkAt(BlockPos(mX, mY, mZ)))
    {
        Entity::tick();
        if (shouldBurn())
        {
            setSecondsOnFire(1);
        }

        std::vector<boost::shared_ptr<Entity> > ignoredEntities;
        std::vector<unsigned long long> ignoredTypes;
        if (mOwner)
        {
            ignoredEntities.push_back(boost::static_pointer_cast<Entity>(mOwner));
        }
        HitResult* hit = ProjectileUtil::forwardsRaycast(shared_from_this(), true, &ignoredEntities, &ignoredTypes);
        if (hit != NULL)
        {
            onHit(hit);
            delete hit;
        }

        mXo = mX;
        mYo = mY;
        mZo = mZ;
        mX += mXd;
        mY += mYd;
        mZ += mZd;
        ProjectileUtil::rotateTowardsMovement(shared_from_this(), 0.2f);

        double inertia = getInertia();
        mXd += mPowerX;
        mYd += mPowerY;
        mZd += mPowerZ;
        mXd *= inertia;
        mYd *= inertia;
        mZd *= inertia;
        setPos(mX, mY, mZ);
    }
    else
    {
        remove();
    }
}

double Fireball::getInertia()
{
    return 0.05000000074505806;
}

void Fireball::addAdditonalSaveData(CompoundTag* tag)
{
    tag->put(L"direction", newDoubleList(3, mXd, mYd, mZd));
    tag->put(L"power", newDoubleList(3, mPowerX, mPowerY, mPowerZ));
    tag->putInt(L"life", mLife);
}

void Fireball::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(L"power", 9))
    {
        ListTag* power = tag->getList(L"power", 6);
        if (power->size() == 3)
        {
            mPowerX = power->getDouble(0);
            mPowerY = power->getDouble(1);
            mPowerZ = power->getDouble(2);
        }
    }
    mLife = tag->getInt(L"life");
    if (tag->contains(L"direction", 9))
    {
        ListTag* direction = tag->getList(L"direction", 6);
        mXd = direction->getDouble(0);
        mYd = direction->getDouble(1);
        mZd = direction->getDouble(2);
    }
    else
    {
        remove();
    }
}

bool Fireball::isPickable()
{
    return true;
}

double Fireball::getPickRadius()
{
    return 1.0;
}

bool Fireball::hurt(const DamageSource* source, float)
{
    if (isInvulnerableTo(source))
    {
        return false;
    }
    markHurt();
    boost::shared_ptr<Entity> direct = source->getDirectEntity();
    if (!direct)
    {
        return false;
    }
    Vec3* direction = direct->getLookAngle();
    if (direction != NULL)
    {
        mXd = direction->x * 0.1;
        mYd = direction->y * 0.1;
        mZd = direction->z * 0.1;
        mPowerX = mXd;
        mPowerY = mYd;
        mPowerZ = mZd;
    }
    boost::shared_ptr<LivingEntity> owner = boost::dynamic_pointer_cast<LivingEntity>(direct);
    if (owner)
    {
        mOwner = owner;
    }
    return true;
}

float Fireball::getBrightness()
{
    return 1.0f;
}

int Fireball::getLightColor()
{
    return 0xF000F0;
}

const ParticleType* Fireball::getTrailParticleType()
{
    return 0;
}

bool Fireball::shouldBurn()
{
    return true;
}

std::vector<unsigned long long>* Fireball::getEntityTypesToIgnoreCollisionsWith()
{
    return 0;
}

unsigned long long Fireball::GetType()
{
    return 0x8A40000000000000ULL;
}
