#include "game/item/AreaEffectCloud.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
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
    for (std::vector<MobEffectInstance*>::iterator it = mEffects.begin(); it != mEffects.end(); ++it)
    {
        delete *it;
    }
}

void AreaEffectCloud::staticCtor()
{
    const unsigned long long type = 0x000000000800000FULL;
    const unsigned long long parent = 0x0000000008000000ULL;
    DATA_RADIUS = SynchedEntityData::defineId<float>(type, parent, EntityDataSerializers::FLOAT);
    DATA_COLOR = SynchedEntityData::defineId<int>(type, parent, EntityDataSerializers::INT);
    DATA_WAITING = SynchedEntityData::defineId<bool>(type, parent, EntityDataSerializers::BOOLEAN);
    DATA_PARTICLE = SynchedEntityData::defineId<int>(type, parent, EntityDataSerializers::INT);
    DATA_PARTICLE_PARAM_1 = SynchedEntityData::defineId<int>(type, parent, EntityDataSerializers::INT);
    DATA_PARTICLE_PARAM_2 = SynchedEntityData::defineId<int>(type, parent, EntityDataSerializers::INT);
    DATA_PARTICLE_RATE = SynchedEntityData::defineId<int>(type, parent, EntityDataSerializers::INT);
}

void AreaEffectCloud::setRadius(float radius)
{
    setSize(radius * 2.0f, 0.5f);
    if (!mLevel->mIsClientSide)
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
        getEntityData()->set(DATA_COLOR, PotionUtils::getColor(&effects), true);
        return;
    }
    getEntityData()->set(DATA_COLOR, 0, true);
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
    if (mTickCount % 5 != 0)
    {
        return;
    }

    std::vector<MobEffectInstance*> effects;
    if (mReapplicationDelayMap.empty())
    {
        std::vector<MobEffectInstance*>& potionEffects = mPotion->getEffects();
        effects.insert(effects.end(), potionEffects.begin(), potionEffects.end());
    }
    else
    {
        for (boost::unordered_map<int, int>::iterator it = mReapplicationDelayMap.begin(); it != mReapplicationDelayMap.end(); )
        {
            if (it->second <= mTickCount)
            {
                it = mReapplicationDelayMap.erase(it);
            }
            else
            {
                ++it;
            }
        }

        std::vector<MobEffectInstance*>& potionEffects = mPotion->getEffects();
        for (std::vector<MobEffectInstance*>::iterator it = potionEffects.begin(); it != potionEffects.end(); ++it)
        {
            MobEffectInstance* source = *it;
            effects.push_back(new MobEffectInstance(source->getEffect(), source->getDuration() / 4, source->getAmplifier(), source->isAmbient(), source->isVisible()));
        }
    }
    effects.insert(effects.end(), mEffects.begin(), mEffects.end());

    if (effects.empty())
    {
        return;
    }

    double radius = static_cast<double>(getRadius());
    AABB* searchArea = mBoundingBox->grow(radius, radius, radius);
    std::vector<boost::shared_ptr<Entity> >* nearby = mLevel->getEntitiesOfClass(typeid(LivingEntity), searchArea);

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = nearby->begin(); it != nearby->end(); ++it)
    {
        boost::shared_ptr<LivingEntity> target = boost::static_pointer_cast<LivingEntity>(*it);

        if (mReapplicationDelayMap.find(target->getId()) != mReapplicationDelayMap.end())
        {
            continue;
        }

        if (!target->isAlive())
        {
            continue;
        }

        double dx = target->mX - mX;
        double dz = target->mZ - mZ;
        if (dx * dx + dz * dz > radius * radius)
        {
            continue;
        }

        mReapplicationDelayMap[target->getId()] = mTickCount + mReapplicationDelay;

        for (std::vector<MobEffectInstance*>::iterator effectIt = effects.begin(); effectIt != effects.end(); ++effectIt)
        {
            target->addEffect(new MobEffectInstance(*effectIt), getOwner());
        }

        if (mRadiusOnUse != 0.0f)
        {
            radius += static_cast<double>(mRadiusOnUse);
            if (radius < 0.5)
            {
                delete nearby;
                return;
            }
            setRadius(static_cast<float>(radius));
        }

        if (mDurationOnUse != 0)
        {
            mDuration += mDurationOnUse;
            if (mDuration < 1)
            {
                delete nearby;
                return;
            }
        }
    }

    delete nearby;
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
    bool waiting = isWaiting();
    double radius = static_cast<double>(getRadius());
    int particle = getParticle();
    const ParticleType* particleType = ParticleType::byId(static_cast<ePARTICLE_TYPE>(particle));
    int paramCount = particleType != NULL ? particleType->getParamCount() : 0;

    arrayWithLength<int> params;
    if (paramCount >= 1)
    {
        params = arrayWithLength<int>(static_cast<unsigned int>(paramCount), true);
        params.data[0] = getParticleParam1();
        if (paramCount > 1)
        {
            params.data[1] = getParticleParam2();
        }
    }

    if (waiting)
    {
        if (mRandom->nextBoolean())
        {
            for (int i = 0; i < 2; ++i)
            {
                float angle = mRandom->nextFloat() * 6.2831854820251465f;
                float dist = std::sqrt(mRandom->nextFloat()) * 0.8999999761581421f;
                double dx = static_cast<double>(std::cos(angle) * dist);
                double dz = static_cast<double>(std::sin(angle) * dist);

                if (particle == ePARTICLE_DRAGON_BREATH)
                {
                    int color = mRandom->nextBoolean() ? 0xffffff : getColor();
                    mLevel->addAlwaysVisibleParticle(static_cast<ePARTICLE_TYPE>(particle), mX + dx, mY, mZ + dz,
                        static_cast<double>(((color >> 16) & 0xff) / 255.0f),
                        static_cast<double>(((color >> 8) & 0xff) / 255.0f),
                        static_cast<double>((color & 0xff) / 255.0f), params);
                }
                else
                {
                    mLevel->addAlwaysVisibleParticle(static_cast<ePARTICLE_TYPE>(particle), mX + dx, mY, mZ + dz, 0.0, 0.0, 0.0, params);
                }
            }
        }
    }
    else
    {
        int rate = DATA_PARTICLE_RATE != NULL ? getEntityData()->get(DATA_PARTICLE_RATE) : 0;
        int count = rate >= 1 ? rate : static_cast<int>(radius * radius * 50.12387886084616);

        for (int i = 0; i < count; ++i)
        {
            float angle = mRandom->nextFloat() * 6.2831854820251465f;
            float dist = std::sqrt(mRandom->nextFloat());
            double dx = static_cast<double>(std::cos(angle) * dist) * radius;
            double dz = static_cast<double>(std::sin(angle) * dist) * radius;

            if (particle == ePARTICLE_DRAGON_BREATH)
            {
                int color = getColor();
                mLevel->addAlwaysVisibleParticle(static_cast<ePARTICLE_TYPE>(particle), mX + dx, mY, mZ + dz,
                    static_cast<double>(((color >> 16) & 0xff) / 255.0f),
                    static_cast<double>(((color >> 8) & 0xff) / 255.0f),
                    static_cast<double>((color & 0xff) / 255.0f), params);
            }
            else
            {
                double xa = (0.5 - mRandom->nextDouble()) * 0.15;
                double za = (0.5 - mRandom->nextDouble()) * 0.15;
                mLevel->addAlwaysVisibleParticle(static_cast<ePARTICLE_TYPE>(particle), mX + dx, mY, mZ + dz, xa, 0.009999999776482582, za, params);
            }
        }
    }

    if (paramCount >= 1)
    {
        delete[] params.data;
    }
}

void AreaEffectCloud::suckInNearbyPlayers()
{
    if (mLevel == NULL || mBoundingBox == NULL)
    {
        return;
    }

    std::vector<boost::shared_ptr<Entity> > entities;
    AABB* searchArea = mBoundingBox->grow(4.0, 4.0, 4.0);
    mLevel->getEntities(boost::shared_ptr<Entity>(), searchArea, &entities);

    const double radius = static_cast<double>(getRadius());
    const boost::shared_ptr<LivingEntity> owner = getOwner();
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities.begin();
         it != entities.end(); ++it)
    {
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(*it);
        if (!player || player->mRemoved || player == owner)
        {
            continue;
        }

        const double dx = player->mX - mX;
        const double dy = (player->mY + player->mBbHeight) - mY;
        const double dz = player->mZ - mZ;
        const double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (distance <= 0.0 || distance >= radius)
        {
            continue;
        }

        const double strength = -0.05 * std::min(1.0, (radius - distance) * 0.25);
        player->mXd += dx / distance * strength;
        player->mYd += dy / distance * strength;
        player->mZd += dz / distance * strength;
    }
}

void AreaEffectCloud::defineSynchedData()
{
    getEntityData()->createDateItem(DATA_COLOR, 0);
    getEntityData()->createDateItem(DATA_RADIUS, 3.0f);
    getEntityData()->createDateItem(DATA_WAITING, false);
    getEntityData()->createDateItem(DATA_PARTICLE, static_cast<int>(ePARTICLE_DRAGON_BREATH));
    getEntityData()->createDateItem(DATA_PARTICLE_PARAM_1, 0);
    getEntityData()->createDateItem(DATA_PARTICLE_PARAM_2, 0);
    getEntityData()->createDateItem(DATA_PARTICLE_RATE, 0);
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
    if (!mLevel->mIsClientSide)
    {
        getEntityData()->set(DATA_PARTICLE_RATE, value, true);
    }
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

void AreaEffectCloud::setDurationOnUse(int duration)
{
    mDurationOnUse = duration;
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
