#include "game/core/ThrownEnderpearl.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Endermite.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/HitResult.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/TheEndGatewayEntity.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/WorldBorder.hpp"

ThrownEnderpearl::ThrownEnderpearl(Level* level)
    : ThrowableProjectile(level), mThrower()
{
    _init();
}

ThrownEnderpearl::ThrownEnderpearl(Level* level, boost::shared_ptr<LivingEntity> owner)
    : ThrowableProjectile(level, owner), mThrower(owner)
{
    _init();
}

ThrownEnderpearl::ThrownEnderpearl(Level* level, double x, double y, double z)
    : ThrowableProjectile(level, x, y, z), mThrower()
{
    _init();
}

ThrownEnderpearl::~ThrownEnderpearl()
{
}

void ThrownEnderpearl::create(Level* level)
{
    new ThrownEnderpearl(level);
}

void ThrownEnderpearl::PlayTeleportSound()
{
    if (mLevel != NULL)
        mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ,
            SoundEvents::ENDERMEN_TELEPORT, getSoundSource(), 1.0f, 1.0f, 16.0f);
}

bool ThrownEnderpearl::onHit(const HitResult* result)
{
    Projectile::onHit(result);
    boost::shared_ptr<LivingEntity> owner = getOwner();

    if (result->mEntity)
    {
        if (result->mEntity == mThrower)
            return false;

        DamageSource* damage = DamageSource::CreateThrown(
            shared_from_this(), boost::static_pointer_cast<Entity>(owner));
        result->mEntity->hurt(damage, 0.0f);
    }

    if (result->mType == HitResult::BLOCK && mLevel != NULL)
    {
        BlockPos position = result->getBlockPos();
        boost::shared_ptr<BlockEntity> blockEntity = mLevel->getBlockEntity(position);
        TheEndGatewayEntity* gateway = blockEntity
            ? dynamic_cast<TheEndGatewayEntity*>(blockEntity.get()) : NULL;
        if (gateway != NULL)
        {
            if (owner)
            {
                gateway->teleportEntity(boost::static_pointer_cast<Entity>(owner));
                remove();
            }
            else
            {
                gateway->teleportEntity(shared_from_this());
            }
            return true;
        }
    }

    if (mLevel != NULL)
    {
        for (int i = 0; i < 32; ++i)
        {
            mLevel->addParticle(PARTICLE_PORTAL,
                mX, mY + mRandom->nextDouble() * 2.0, mZ,
                mRandom->nextGaussian(), 0.0, mRandom->nextGaussian(),
                arrayWithLength<int>());
        }
    }

    if (mLevel != NULL && !mLevel->mIsClientSide)
    {
        if (!mLevel->getWorldBorder()->isWithinBounds(BlockPos(this)))
        {
            remove();
            return true;
        }

        if (owner)
        {
            if (owner->GetType() == 0x0c800001ULL)
            {
                boost::shared_ptr<ServerPlayer> player =
                    boost::static_pointer_cast<ServerPlayer>(owner);
                if (!player->mRemoved && !player->hasDisconnected()
                    && player->mLevel == mLevel && !player->isSleeping())
                {
                    double distance = player->distanceTo(mX, mY, mZ);
                    if (distance > 100.0)
                    {
                        boost::shared_ptr<Player> statPlayer =
                            boost::static_pointer_cast<Player>(owner);
                        statPlayer->awardStat(GenericStats::beamMeUp(),
                            GenericStats::param_beamMeUp((int)distance));
                    }

                    if (mRandom->nextFloat() < 0.05f
                        && mLevel->getGameRules()->getBoolean(3))
                    {
                        boost::shared_ptr<Endermite> endermite(new Endermite(mLevel));
                        endermite->setPlayerSpawned(true);
                        endermite->moveTo(player->mX, player->mY, player->mZ,
                            player->mYRot, player->mXRot);
                        mLevel->addEntity(endermite);
                    }

                    if (owner->isPassenger())
                        owner->stopRiding();
                    owner->teleportTo(mX, mY, mZ);
                    PlayTeleportSound();
                    owner->mFallDistance = 0.0f;
                    owner->hurt(DamageSource::FALL, 5.0f);
                }
            }
            else
            {
                owner->teleportTo(mX, mY, mZ);
                PlayTeleportSound();
                owner->mFallDistance = 0.0f;
            }
        }
        remove();
    }
    return true;
}

void ThrownEnderpearl::addDataWalkers(DataFixerUpper*)
{
}

void ThrownEnderpearl::_init()
{
    fjDerivedCtorCalls();
    mThrower.reset();
}

void ThrownEnderpearl::tick()
{
    boost::shared_ptr<LivingEntity> owner = getOwner();
    if (owner && (owner->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER
        && !owner->isAlive())
    {
        remove();
        return;
    }
    ThrowableProjectile::tick();
}

unsigned long long ThrownEnderpearl::GetType()
{
    return 0x8A200003ULL;
}
