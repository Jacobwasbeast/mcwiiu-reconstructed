#include "game/entity/projectile/LargeFireball.hpp"

#include <string>

#include "game/core/DamageSource.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/HitResult.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/Level.hpp"

LargeFireball::LargeFireball(Level* level)
    : Fireball(level), mExplosionPower(1)
{
}

LargeFireball::LargeFireball(Level* level, double x, double y, double z, double xd, double yd, double zd)
    : Fireball(level, x, y, z, xd, yd, zd), mExplosionPower(1)
{
}

LargeFireball::LargeFireball(Level* level, boost::shared_ptr<LivingEntity> owner, double xd, double yd, double zd)
    : Fireball(level, owner, xd, yd, zd), mExplosionPower(1)
{
}

LargeFireball::~LargeFireball()
{
}

void LargeFireball::create(Level* level)
{
    new LargeFireball(level);
}

void LargeFireball::addDataWalkers(DataFixerUpper* fixer)
{
    Fireball::addDataWalkers(fixer, 0x8a400000ULL);
}

void LargeFireball::onHit(const HitResult* result)
{
    if (mLevel->mIsClientSide)
    {
        return;
    }

    if (result->mEntity)
    {
        boost::shared_ptr<DamageSource> damageSource = DamageSource::CreateFireball(
            boost::static_pointer_cast<Fireball>(shared_from_this()), mOwner);
        if (result->mEntity->hurt(damageSource.get(), 6.0f))
        {
            result->mEntity->setSecondsOnFire(3);
        }
    }

    const bool mobGriefing = mLevel->getGameRules()->getBoolean(1);
    mLevel->explode(shared_from_this(), mX, mY, mZ, static_cast<float>(mExplosionPower),
                    false, mobGriefing, false, false, false, -1.0f);
    remove();
}

void LargeFireball::addAdditonalSaveData(CompoundTag* tag)
{
    Fireball::addAdditonalSaveData(tag);
    tag->putInt(L"ExplosionPower", mExplosionPower);
}

void LargeFireball::readAdditionalSaveData(CompoundTag* tag)
{
    Fireball::readAdditionalSaveData(tag);
    if (tag->contains(L"ExplosionPower", 99))
    {
        mExplosionPower = tag->getInt(L"ExplosionPower");
    }
}

unsigned long long LargeFireball::GetType()
{
    return 0x000000008A400003ULL;
}
