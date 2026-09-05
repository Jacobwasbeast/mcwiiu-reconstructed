#include "game/core/EvocationFangs.hpp"

#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/DamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

EvocationFangs::EvocationFangs(Level* level) : Entity(level, true), mWarmupDelay(0), mSentSpikeEvent(false),
    mLifeTicks(0), mClientSideAttack(false), mOwner(), mOwnerUUID()
{
    _init();
    setSize(0.5f, 0.8f);
}

EvocationFangs::EvocationFangs(Level* level, double x, double y, double z, float yRot, int warmupDelay,
                               const boost::shared_ptr<LivingEntity>& owner) : Entity(level, true),
    mWarmupDelay(0), mSentSpikeEvent(false), mLifeTicks(0), mClientSideAttack(false), mOwner(), mOwnerUUID()
{
    _init();
    mWarmupDelay = warmupDelay;
    setOwner(owner);
    mYRot = yRot * 57.2957763671875f;
    setPos(x, y, z);
}

EvocationFangs::~EvocationFangs()
{
}

void EvocationFangs::_init()
{
    fjDerivedCtorCalls();
    mSentSpikeEvent = false;
    mLifeTicks = 22;
    mClientSideAttack = false;
    mWarmupDelay = 0;
    mOwnerUUID.clear();
}

void EvocationFangs::setOwner(const boost::shared_ptr<LivingEntity>& owner)
{
    mOwner = owner;
    mOwnerUUID = owner ? ((Entity*)owner.get())->getUUID() : std::wstring();
}

boost::shared_ptr<LivingEntity> EvocationFangs::getOwner()
{
    return mOwner;
}

void EvocationFangs::defineSynchedData()
{
}

void EvocationFangs::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag == NULL) return;
    mWarmupDelay = tag->getInt(L"Warmup");
    mOwnerUUID = tag->getString(L"OwnerUUID");
}

void EvocationFangs::addAdditonalSaveData(CompoundTag* tag)
{
    if (tag == NULL) return;
    tag->putInt(L"Warmup", mWarmupDelay);
    if (!mOwnerUUID.empty()) tag->putString(L"OwnerUUID", mOwnerUUID);
}

void EvocationFangs::dealDamageTo(const boost::shared_ptr<LivingEntity>& target)
{
    if (target == NULL || !((Entity*)target.get())->isAttackable()) return;
    boost::shared_ptr<LivingEntity> owner = getOwner();
    boost::shared_ptr<Entity> ownerEntity(owner, (Entity*)owner.get());
    if (owner != NULL && (target.get() == owner.get() || ((Entity*)target.get())->isAlliedTo(ownerEntity))) return;

    boost::shared_ptr<Entity> direct = shared_from_this();
    boost::shared_ptr<Entity> attacker;
    if (owner != NULL) attacker = ownerEntity;
    boost::shared_ptr<DamageSource> damage = DamageSource::CreateIndirectMagic(direct, attacker, NULL);
    ((Entity*)target.get())->hurt(damage.get(), 6.0f);
}

void EvocationFangs::tick()
{
    Entity::tick();
    if (mLevel == NULL) return;

    if (!mLevel->mIsClientSide)
    {
        --mWarmupDelay;
        if (mWarmupDelay < 0)
        {
            if (mWarmupDelay == -8)
            {
                std::vector<boost::shared_ptr<Entity> >* entities =
                    mLevel->getEntitiesOfClass(typeid(Entity), mBoundingBox->grow(6.0, 128.0, 6.0));
                if (entities != NULL)
                {
                    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
                    {
                        boost::shared_ptr<LivingEntity> living(*it, (LivingEntity*)(*it).get());
                        dealDamageTo(living);
                    }
                    delete entities;
                }
            }
            if (!mSentSpikeEvent)
            {
                mLevel->broadcastEntityEvent(shared_from_this(), 4, 0);
                mSentSpikeEvent = true;
            }
            if (--mLifeTicks < 0) remove();
        }
    }
    else if (mClientSideAttack && --mLifeTicks == 14)
    {
        for (int i = 0; i < 4; ++i)
        {
            mLevel->addParticle(PARTICLE_SNOWBALL_POOF, mX + (mRandom->nextDouble() * 2.0 - 1.0) * (double)mBbWidth * 1.75,
                                mY + mRandom->nextDouble() + 1.0625, mZ + (mRandom->nextDouble() * 2.0 - 1.0) * (double)mBbWidth * 1.75,
                                (mRandom->nextDouble() * 2.0 - 1.0) * 0.3, mRandom->nextDouble() * 0.3 + 0.3,
                                (mRandom->nextDouble() * 2.0 - 1.0) * 0.3, arrayWithLength<int>());
        }
    }
}

void EvocationFangs::handleEntityEvent(unsigned char event, int data)
{
    if (data != 4)
    {
        Entity::handleEntityEvent(event, data);
        return;
    }
    mClientSideAttack = true;
    if (!isSilent() && mLevel != NULL)
    {
        mLevel->playLocalSound(mX, mY, mZ, SoundEvents::EVOCATION_FANGS_ATTACK,
                               getSoundSource(), 1.0f,
                               0.85f + mRandom->nextFloat() * 0.2f, false, 16.0f);
    }
}

double EvocationFangs::getAnimationProgress(float partialTicks) const
{
    if (!mClientSideAttack) return 0.0;
    int ticks = mLifeTicks - 2;
    double progress = 1.0;
    if (ticks > 0) progress = 1.0 - ((double)ticks - (double)partialTicks) / 20.0;
    return progress;
}

void EvocationFangs::create(Level* level)
{
    new EvocationFangs(level);
}

unsigned long long EvocationFangs::GetType()
{
    return 0x0800001100000000ULL;
}
