#include "game/core/Snowball.hpp"

#include "game/core/DamageSource.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/world/Level.hpp"

Snowball::Snowball(Level* level, boost::shared_ptr<LivingEntity> owner)
    : ThrowableProjectile(level, owner)
{
    _init();
}

Snowball::Snowball(Level* level)
    : ThrowableProjectile(level)
{
    _init();
}

Snowball::Snowball(Level* level, double x, double y, double z)
    : ThrowableProjectile(level, x, y, z)
{
    _init();
}

Snowball::~Snowball()
{
}

void Snowball::create(Level* level)
{
    new Snowball(level);
}

void Snowball::_init()
{
    fjDerivedCtorCalls();
}

bool Snowball::onHit(const HitResult* result)
{
    if (result->mEntity)
    {
        float damage = 3.0f;
        unsigned long long targetType = result->mEntity->GetType();
        if (targetType == 0x5D540045ULL)
        {
            damage = 32.0f;
        }
        else if ((targetType & 0x0C800000ULL) == 0x0C800000ULL
                 && Minecraft::InMiniGame(static_cast<EMiniGameId>(1), false))
        {
            damage = 0.1f;
        }

        boost::shared_ptr<Entity> projectile = shared_from_this();
        boost::shared_ptr<DamageSource> source = DamageSource::CreateThrown(projectile, result->mEntity);
        result->mEntity->hurt(source.get(), damage);
    }

    if (!mLevel->mIsClientSide)
    {
        remove();
    }
    else
    {
        fjSetParticleSpawnPos(mX, mY, mZ);
    }
    return true;
}

void Snowball::addDataWalkers(DataFixerUpper* fixer)
{
    ThrowableProjectile::addDataWalkers(fixer, 0x8A200001ULL);
}

void Snowball::handleEntityEvent(unsigned char event, int data)
{
    if (event == 3)
    {
        Vec3* position = fjGetParticleSpawnPos();
        if (position != NULL)
        {
            for (int i = 0; i < 8; ++i)
            {
                mLevel->addParticle(PARTICLE_SNOWBALL_POOF, position->x, position->y, position->z,
                                    0.0, 0.0, 0.0, arrayWithLength<int>());
            }
        }
        return;
    }
    Entity::handleEntityEvent(event, data);
}

unsigned long long Snowball::GetType()
{
    return 0x8A200001ULL;
}
