#include "game/entity/projectile/LargeFireball.hpp"
#include "game/core/EntityType.hpp"

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
    Entity::fjDerivedCtorCalls();
}

LargeFireball::LargeFireball(Level* level, double x, double y, double z, double xd, double yd, double zd)
    : Fireball(level, x, y, z, xd, yd, zd), mExplosionPower(1)
{
    Entity::fjDerivedCtorCalls();
}

LargeFireball::LargeFireball(Level* level, boost::shared_ptr<LivingEntity> owner, double xd, double yd, double zd)
    : Fireball(level, owner, xd, yd, zd), mExplosionPower(1)
{
    Entity::fjDerivedCtorCalls();
}

void LargeFireball::create(Level* level)
{
    new LargeFireball(level);
}

void LargeFireball::addDataWalkers(DataFixerUpper* fixer)
{
    Fireball::addDataWalkers(fixer, EntityType::FIREBALL);
}

void LargeFireball::onHit(const HitResult* result)
{
    if (mLevel->mIsClientSide)
    {
        return;
    }

    if (result->mEntity)
    {
        DamageSource* damageSource = DamageSource::CreateFireball(
            boost::static_pointer_cast<Fireball>(shared_from_this()), mOwner);
        if (result->mEntity->hurt(damageSource, 6.0f))
        {
            result->mEntity->setSecondsOnFire(3);
        }
        delete damageSource;
    }

    const bool mobGriefing = mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING);
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
    if (tag->contains(L"ExplosionPower", Tag::TAG_ANY_NUMERIC))
    {
        mExplosionPower = tag->getInt(L"ExplosionPower");
    }
}

unsigned long long LargeFireball::GetType()
{
    return EntityType::LARGE_FIREBALL;
}
