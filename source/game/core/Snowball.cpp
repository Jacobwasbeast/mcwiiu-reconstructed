#include "game/core/Snowball.hpp"

#include "game/core/DamageSource.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

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
    Projectile::onHit(result);

    if (result->mEntity)
    {
        if (!mLevel->mIsClientSide)
        {
            if (Minecraft::InMiniGame(eMiniGameId_Any, true) && mOwner
                && (mOwner->GetType() & 0x0C800000ULL) == 0x0C800000ULL)
            {
                boost::static_pointer_cast<Player>(mOwner)->GetGameMode()->RecordSnowballHit();

                if ((result->mEntity->GetType() & 0x0C800000ULL) == 0x0C800000ULL)
                {
                    boost::shared_ptr<Player> player =
                        boost::static_pointer_cast<Player>(result->mEntity);
                    if (player && player->GetGameMode() != NULL)
                    {
                        player->GetGameMode()->RecordHitBySnowball();
                    }
                }
            }
        }

        float damage = 0.0f;
        if (result->mEntity->GetType() == 0x5D540045ULL)
        {
            damage = 3.0f;
        }
        else if ((result->mEntity->GetType() & 0x0C800000ULL) == 0x0C800000ULL
                 && Minecraft::InMiniGame(eMiniGameId_Battle, false))
        {
            damage = 0.1f;
        }

        DamageSource* source = DamageSource::CreateThrown(shared_from_this(), getOwner());
        result->mEntity->hurt(source, damage);
        delete source;
    }

    if (!mLevel->mIsClientSide)
    {
        mLevel->broadcastEntityEvent(shared_from_this(), 3, 0);
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
        for (int i = 0; i < 8; ++i)
        {
            mLevel->addParticle(PARTICLE_SNOWBALL_POOF, position->x, position->y, position->z,
                                0.0, 0.0, 0.0, arrayWithLength<int>());
        }
        return;
    }
    Entity::handleEntityEvent(event, data);
}

unsigned long long Snowball::GetType()
{
    return 0x8A200001ULL;
}
