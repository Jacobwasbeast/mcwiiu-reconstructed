#include "game/entity/projectile/SmallFireball.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/DamageSource.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/HitResult.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

SmallFireball::SmallFireball(Level* level)
    : Fireball(level)
{
    fjDerivedCtorCalls();
    setSize(0.3125f, 0.3125f);
}

SmallFireball::SmallFireball(Level* level, boost::shared_ptr<LivingEntity> owner,
                             double xd, double yd, double zd)
    : Fireball(level, owner, xd, yd, zd)
{
    fjDerivedCtorCalls();
    setSize(0.3125f, 0.3125f);
}

SmallFireball::SmallFireball(Level* level, double x, double y, double z,
                             double xd, double yd, double zd)
    : Fireball(level, x, y, z, xd, yd, zd)
{
    fjDerivedCtorCalls();
    setSize(0.3125f, 0.3125f);
}

void SmallFireball::create(Level* level)
{
    new SmallFireball(level);
}

void SmallFireball::addDataWalkers(DataFixerUpper* fixer)
{
    Fireball::addDataWalkers(fixer, EntityType::SMALL_FIREBALL);
}

void SmallFireball::onHit(const HitResult* result)
{
    if (mLevel->mIsClientSide)
    {
        return;
    }

    if (result->mEntity == NULL)
    {
        if (mOwner && (mOwner->GetType() & EntityType::MOB) == EntityType::MOB
            && !mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
        {
            remove();
            return;
        }

        const BlockPos pos = result->getBlockPos().relative(result->mDirection);
        if (mLevel->isEmptyBlock(pos))
        {
            mLevel->setBlock(pos, Blocks::fire->defaultBlockState(), Block::UPDATE_ALL, false);
        }
    }
    else
    {
        if (result->mEntity->isFireImmune())
        {
            remove();
            return;
        }

        boost::shared_ptr<Fireball> fireball =
            boost::static_pointer_cast<Fireball>(shared_from_this());
        boost::shared_ptr<Entity> owner =
            boost::static_pointer_cast<Entity>(mOwner);
        boost::shared_ptr<DamageSource> damageSource =
            DamageSource::CreateFireball(fireball, owner);
        if (result->mEntity->hurt(damageSource.get(), 5.0f))
        {
            doEnchantDamageEffects(mOwner, result->mEntity);
            result->mEntity->setSecondsOnFire(5);
        }
    }

    remove();
}

bool SmallFireball::isPickable()
{
    return false;
}

bool SmallFireball::hurt(const DamageSource*, float)
{
    return false;
}

unsigned long long SmallFireball::GetType()
{
    return EntityType::SMALL_FIREBALL;
}
