#include "game/core/ThrownEnderpearl.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/HitResult.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/TheEndGatewayEntity.hpp"
#include "game/entity/projectile/Projectile.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

ThrownEnderpearl::ThrownEnderpearl(Level* level)
    : ThrowableProjectile(level), mUnknown()
{
    _init();
}

ThrownEnderpearl::ThrownEnderpearl(Level* level, boost::shared_ptr<LivingEntity> owner)
    : ThrowableProjectile(level, owner), mUnknown()
{
    _init();
}

ThrownEnderpearl::ThrownEnderpearl(Level* level, double x, double y, double z)
    : ThrowableProjectile(level, x, y, z), mUnknown()
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
        mLevel->playLocalSound(mX, mY, mZ, SoundEvents::ENDERMEN_TELEPORT,
            SoundSource::eSoundSource_Neutral, 1.0f, 1.0f, false, 16.0f);
}

bool ThrownEnderpearl::onHit(const HitResult* result)
{
    boost::shared_ptr<LivingEntity> owner = getOwner();

    if (result->mType == HitResult::ENTITY && result->mEntity && owner
        && result->mEntity.get() == static_cast<Entity*>(owner.get()))
    {
        return false;
    }

    if (owner)
    {
        boost::shared_ptr<DamageSource> damage = DamageSource::CreateThrown(
            shared_from_this(), boost::static_pointer_cast<Entity>(owner));
        owner->hurt(damage.get(), 0.0f);
    }

    if (result->mType == HitResult::BLOCK && mLevel != NULL)
    {
        BlockPos position = result->getBlockPos();
        boost::shared_ptr<BlockEntity> blockEntity = mLevel->getBlockEntity(position);
        TheEndGatewayEntity* gateway = blockEntity
            ? dynamic_cast<TheEndGatewayEntity*>(blockEntity.get()) : NULL;
        if (gateway != NULL)
        {
            gateway->teleportEntity(shared_from_this());
            remove();
            return true;
        }
    }

    if (mLevel != NULL)
    {
        for (int i = 0; i < 32; ++i)
        {
            mLevel->addParticle(PARTICLE_PORTAL,
                mX + mRandom->nextDouble() * 2.0 - 1.0, mY,
                mZ + mRandom->nextDouble() * 2.0 - 1.0,
                mRandom->nextGaussian(), 0.0, mRandom->nextGaussian(),
                arrayWithLength<int>());
        }
    }

    if (owner)
    {
        owner->teleportTo(mX, mY, mZ);
        PlayTeleportSound();
        owner->mFallDistance = 0.0f;
        mLevel->levelEvent(2003, BlockPos(boost::static_pointer_cast<Entity>(owner)), 0);
        GenericStats::beamMeUp();
    }
    remove();
    return true;
}

void ThrownEnderpearl::addDataWalkers(DataFixerUpper*)
{
}

void ThrownEnderpearl::_init()
{
    fjDerivedCtorCalls();
    mUnknown.reset();
}

void ThrownEnderpearl::tick()
{
    boost::shared_ptr<LivingEntity> owner = getOwner();
    if (owner && (owner->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER
        && owner->mRemoved)
    {
        remove();
        return;
    }
    ThrowableProjectile::tick();
}

unsigned long long ThrownEnderpearl::GetType()
{
    return 0x8a20000300000000ULL;
}
