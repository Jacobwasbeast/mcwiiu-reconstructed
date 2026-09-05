#include "game/item/AreaEffectCloud.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"

const EntityDataAccessor<float>* AreaEffectCloud::DATA_RADIUS = NULL;
const EntityDataAccessor<int>* AreaEffectCloud::DATA_COLOR = NULL;
const EntityDataAccessor<bool>* AreaEffectCloud::DATA_WAITING = NULL;
const EntityDataAccessor<int>* AreaEffectCloud::DATA_PARTICLE = NULL;
const EntityDataAccessor<int>* AreaEffectCloud::DATA_PARTICLE_PARAM_1 = NULL;
const EntityDataAccessor<int>* AreaEffectCloud::DATA_PARTICLE_PARAM_2 = NULL;
const EntityDataAccessor<int>* AreaEffectCloud::DATA_PARTICLE_RATE = NULL;

AreaEffectCloud::AreaEffectCloud(Level* level) : Entity(level, true), mPotion(NULL), mDuration(600), mWaitTime(20), mReapplicationDelay(20), mFixedColor(false), mDurationOnUse(0), mRadiusOnUse(0.0f), mRadiusPerTick(0.0f), mOwner(), mOwnerUUID()
{
    _init();
}

AreaEffectCloud::AreaEffectCloud(Level* level, double x, double y, double z) : Entity(level, true), mPotion(NULL), mDuration(600), mWaitTime(20), mReapplicationDelay(20), mFixedColor(false), mDurationOnUse(0), mRadiusOnUse(0.0f), mRadiusPerTick(0.0f), mOwner(), mOwnerUUID()
{
    setPos(x, y, z);
    _init();
}

AreaEffectCloud::~AreaEffectCloud()
{
}

void AreaEffectCloud::staticCtor()
{
}

void AreaEffectCloud::setRadius(float radius)
{
    setSize(radius * 2.0f, 0.5f);
    if (mLevel != NULL && DATA_RADIUS != NULL)
    {
        getEntityData()->set(DATA_RADIUS, radius, true);
    }
}

void AreaEffectCloud::_init()
{
    fjDerivedCtorCalls();
    mFixedColor = false;
    mReapplicationDelay = 20;
    mDurationOnUse = 0;
    mRadiusOnUse = 0;
    mRadiusPerTick = 0.0f;
    mPotion = NULL;
    mDuration = 600;
    mWaitTime = 20;
    mFixedColor = 0;
    setRadius(3.0f);
}

void AreaEffectCloud::updateColor()
{
    if (mPotion != NULL || !mEffects.empty())
    {
        std::vector<MobEffectInstance*> effects;
        PotionUtils::getAllEffects(mPotion, &effects);
        if (DATA_COLOR != NULL)
        {
            getEntityData()->set(DATA_COLOR, PotionUtils::getColor(&effects), true);
        }
        return;
    }
    if (DATA_COLOR != NULL)
    {
        getEntityData()->set(DATA_COLOR, 0, true);
    }
}

float AreaEffectCloud::getRadius()
{
    return DATA_RADIUS != NULL ? getEntityData()->get(DATA_RADIUS) : 3.0f;
}

boost::shared_ptr<LivingEntity> AreaEffectCloud::getOwner()
{
    return mOwner;
}

void AreaEffectCloud::applyMobEffects()
{
    if (mPotion == NULL && mEffects.empty())
    {
        return;
    }
    mReapplicationDelayMap.clear();
}

bool AreaEffectCloud::isWaiting()
{
    return DATA_WAITING != NULL ? getEntityData()->get(DATA_WAITING) : false;
}

int AreaEffectCloud::getParticle()
{
    return DATA_PARTICLE != NULL ? getEntityData()->get(DATA_PARTICLE) : ePARTICLE_DRAGON_BREATH;
}

int AreaEffectCloud::getColor()
{
    return DATA_COLOR != NULL ? getEntityData()->get(DATA_COLOR) : 0;
}

int AreaEffectCloud::getParticleParam1()
{
    return DATA_PARTICLE_PARAM_1 != NULL ? getEntityData()->get(DATA_PARTICLE_PARAM_1) : 0;
}

int AreaEffectCloud::getParticleParam2()
{
    return DATA_PARTICLE_PARAM_2 != NULL ? getEntityData()->get(DATA_PARTICLE_PARAM_2) : 0;
}

void AreaEffectCloud::generateParticles()
{
    if (mLevel == NULL || isWaiting())
    {
        return;
    }
    mLevel->addParticle(static_cast<ePARTICLE_TYPE>(getParticle()), false, mX, mY, mZ, 0.0, 0.0, 0.0, arrayWithLength<int>());
}

void AreaEffectCloud::suckInNearbyPlayers()
{
}

void AreaEffectCloud::defineSynchedData()
{
    SynchedEntityData* data = getEntityData().get();
    if (data == NULL)
    {
        return;
    }
    if (DATA_COLOR != NULL) data->createDateItem(DATA_COLOR, 0);
    if (DATA_RADIUS != NULL) data->createDateItem(DATA_RADIUS, 3.0f);
    if (DATA_WAITING != NULL) data->createDateItem(DATA_WAITING, false);
    if (DATA_PARTICLE != NULL) data->createDateItem(DATA_PARTICLE, static_cast<int>(ePARTICLE_DRAGON_BREATH));
    if (DATA_PARTICLE_PARAM_1 != NULL) data->createDateItem(DATA_PARTICLE_PARAM_1, 0);
    if (DATA_PARTICLE_PARAM_2 != NULL) data->createDateItem(DATA_PARTICLE_PARAM_2, 0);
    if (DATA_PARTICLE_RATE != NULL) data->createDateItem(DATA_PARTICLE_RATE, 0);
}

void AreaEffectCloud::setPotion(Potion* potion)
{
    mPotion = potion;
    if (!mFixedColor) updateColor();
}

void AreaEffectCloud::addEffect(MobEffectInstance* effect)
{
    mEffects.push_back(effect);
    if (!mFixedColor) updateColor();
}

void AreaEffectCloud::setFixedColor(int color)
{
    mFixedColor = 1;
    if (DATA_COLOR != NULL) getEntityData()->set(DATA_COLOR, color, true);
}

void AreaEffectCloud::setParticle(const ParticleType* particle)
{
    if (DATA_PARTICLE != NULL && particle != NULL) getEntityData()->set(DATA_PARTICLE, static_cast<int>(particle->getId()), true);
}

void AreaEffectCloud::setParticleParam1(int value)
{
    if (DATA_PARTICLE_PARAM_1 != NULL) getEntityData()->set(DATA_PARTICLE_PARAM_1, value, true);
}

void AreaEffectCloud::setParticleParam2(int value)
{
    if (DATA_PARTICLE_PARAM_2 != NULL) getEntityData()->set(DATA_PARTICLE_PARAM_2, value, true);
}

void AreaEffectCloud::setParticleSpawnRateOnTick(int value)
{
    if (DATA_PARTICLE_RATE != NULL) getEntityData()->set(DATA_PARTICLE_RATE, value, true);
}

void AreaEffectCloud::setWaiting(bool waiting)
{
    if (DATA_WAITING != NULL) getEntityData()->set(DATA_WAITING, waiting, true);
}

int AreaEffectCloud::getDuration()
{
    return mDuration;
}

void AreaEffectCloud::setDuration(int duration)
{
    mDuration = duration;
}

void AreaEffectCloud::tick()
{
    Entity::tick();
    bool waiting = isWaiting();
    double radius = getRadius();
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureInvertedExplodingGrenades())
    {
        suckInNearbyPlayers();
    }
    if (mLevel->mIsClientSide)
    {
        generateParticles();
        return;
    }
    if (mWaitTime + mDuration <= mTickCount)
    {
        remove();
        return;
    }
    bool shouldWait = mTickCount < mWaitTime;
    if (waiting != shouldWait) setWaiting(shouldWait);
    if (!shouldWait)
    {
        if (mRadiusPerTick != 0.0f)
        {
            radius += mRadiusPerTick;
            if (radius < 0.5)
            {
                remove();
                return;
            }
            setRadius(radius);
        }
        if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureInvertedExplodingGrenades())
        {
            applyMobEffects();
        }
    }
}

void AreaEffectCloud::setRadiusOnUse(float radius)
{
    mRadiusOnUse = radius;
}

void AreaEffectCloud::setRadiusPerTick(float radius)
{
    mRadiusPerTick = radius;
}

void AreaEffectCloud::setWaitTime(int waitTime)
{
    mWaitTime = waitTime;
}

void AreaEffectCloud::setOwner(boost::shared_ptr<LivingEntity> owner)
{
    mOwner = owner;
    mOwnerUUID = owner ? owner->getUUID() : std::wstring();
}

void AreaEffectCloud::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag == NULL) return;
    setRadius(tag->getFloat(L"Radius"));
    setRadiusOnUse(tag->getFloat(L"RadiusOnUse"));
    setRadiusPerTick(tag->getFloat(L"RadiusPerTick"));
    setWaitTime(tag->getInt(L"WaitTime"));
    setDuration(tag->getInt(L"Duration"));
    mReapplicationDelay = tag->getInt(L"ReapplicationDelay");
    setDurationOnUse(tag->getInt(L"DurationOnUse"));
    if (tag->contains(L"FixedColor")) setFixedColor(tag->getInt(L"FixedColor"));
    if (tag->contains(L"Particle")) setParticle(ParticleType::byId(static_cast<ePARTICLE_TYPE>(tag->getInt(L"Particle"))));
    setParticleParam1(tag->getInt(L"ParticleParam1"));
    setParticleParam2(tag->getInt(L"ParticleParam2"));
    setParticleSpawnRateOnTick(tag->getInt(L"ParticleSpawnRateOnTick"));
    setPotion(PotionUtils::getPotion(tag));
}

void AreaEffectCloud::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putFloat(L"Radius", getRadius());
    tag->putFloat(L"RadiusOnUse", mRadiusOnUse);
    tag->putFloat(L"RadiusPerTick", mRadiusPerTick);
    tag->putInt(L"WaitTime", mWaitTime);
    tag->putInt(L"ReapplicationDelay", mReapplicationDelay);
    tag->putInt(L"Duration", mDuration);
    tag->putInt(L"DurationOnUse", mDurationOnUse);
    tag->putInt(L"Particle", getParticle());
    tag->putInt(L"ParticleParam1", getParticleParam1());
    tag->putInt(L"ParticleParam2", getParticleParam2());
    tag->putInt(L"ParticleSpawnRateOnTick", DATA_PARTICLE_RATE != NULL ? getEntityData()->get(DATA_PARTICLE_RATE) : 0);
    if (mFixedColor) tag->putInt(L"FixedColor", getColor());
}

void AreaEffectCloud::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (accessor == DATA_RADIUS) setRadius(getRadius());
    Entity::onSyncedDataUpdated(accessor);
}

int AreaEffectCloud::getPistonPushReaction()
{
    return 3;
}

unsigned long long AreaEffectCloud::GetType()
{
    return 0x0800000fULL;
}

void AreaEffectCloud::create(Level* level)
{
    new AreaEffectCloud(level);
}
