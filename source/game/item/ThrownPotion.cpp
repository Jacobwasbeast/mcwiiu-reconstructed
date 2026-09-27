#include "game/item/ThrownPotion.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/Direction.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potion.hpp"
#include "game/item/Potions.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

namespace
{
}

const EntityDataAccessor<not_null_ptr<ItemInstance> >* ThrownPotion::DATA_ITEM = NULL;

ThrownPotion::ThrownPotion(Level* level)
    : ThrowableProjectile(level)
{
    _init();
}

ThrownPotion::ThrownPotion(Level* level, boost::shared_ptr<LivingEntity> owner, not_null_ptr<ItemInstance> item)
    : ThrowableProjectile(level, owner)
{
    _init();
    getEntityData()->set(DATA_ITEM, item, false);
}

ThrownPotion::ThrownPotion(Level* level, double x, double y, double z, not_null_ptr<ItemInstance> item)
    : ThrowableProjectile(level, x, y, z)
{
    _init();
    if (!item->isEmpty())
    {
        setItem(item);
        if (level->mIsClientSide && item->getItem() == Items::splashPotion)
        {
            level->playLocalSound(x, y, z, SoundEvents::SPLASH_POTION_THROW, SoundSource::eSoundSource_Players, 0.5f,
                                  0.4f / (mRandom->nextFloat() * 0.4f + 0.8f), false, 16.0f);
        }
    }
}

ThrownPotion::~ThrownPotion()
{
}

void ThrownPotion::create(Level* level)
{
    new ThrownPotion(level);
}

not_null_ptr<ItemInstance> ThrownPotion::getItem()
{
    not_null_ptr<ItemInstance> item = getEntityData()->get(DATA_ITEM);
    if (item->getItem() != Items::splashPotion && item->getItem() != Items::lingeringPotion)
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::splashPotion));
    return item;
}

void ThrownPotion::dowseFire(const BlockPos& pos, const Direction* direction)
{
    if (Block::getId(mLevel->getBlockState(pos)->getBlock()) == BlockId::AIR)
    {
        mLevel->extinguishFire(boost::shared_ptr<Player>(), pos, direction->getOpposite());
    }
}

bool ThrownPotion::isWaterSensitiveEntity(const boost::shared_ptr<Entity>& entity)
{
    if (!entity)
        return true;
    unsigned long long type = entity->GetType();
    return type == EntityType::ENDER_MAN || type == EntityType::BLAZE || type == EntityType::SNOW_MAN;
}

void ThrownPotion::applyWater()
{
    if (mLevel == NULL || mBoundingBox == NULL)
        return;
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(
        typeid(Entity), mBoundingBox->grow(4.0, 2.0, 4.0), new WATER_SENSITIVE(), false);
    if (entities == NULL)
        return;
    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        if (distanceToSqr(*it) < 16.0 && isWaterSensitiveEntity(*it))
            (*it)->clearFire();
    }
}

bool ThrownPotion::isLingering()
{
    return getItem()->getItem() == Items::lingeringPotion;
}

void ThrownPotion::makeAreaOfEffectCloud(const not_null_ptr<ItemInstance>& item, Potion* potion)
{
    AreaEffectCloud* cloud = new AreaEffectCloud(mLevel, mX, mY, mZ);

    std::vector<MobEffectInstance*>* mobEffects = PotionUtils::getMobEffects(item);
    if (mobEffects->size() == 1)
    {
        MobEffectInstance* effectInstance = mobEffects->at(0);
        int lingeringDuration = Minecraft::GetInstance()->GetMiniGame()->getLingeringDuration(
            effectInstance->getAmplifier(), cloud->getDuration());
        cloud->setDuration(lingeringDuration);
    }

    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureInvertedExplodingGrenades())
        cloud->setDuration(80);

    cloud->setOwner(getOwner());
    cloud->setRadius(3.0f);
    cloud->setRadiusOnUse(-0.5f);
    cloud->setWaitTime(10);
    cloud->setRadiusPerTick((float)(-cloud->getRadius() / (double)cloud->getDuration()));
    cloud->setPotion(potion);

    std::vector<MobEffectInstance*>* effects = PotionUtils::getCustomEffects(item);
    std::vector<MobEffectInstance*>::iterator it;
    for (it = effects->begin(); it != effects->end(); ++it)
    {
        MobEffectInstance* orig = *it;
        cloud->addEffect(new MobEffectInstance(orig->getEffect(), orig->getDuration(), orig->getAmplifier()));
    }

    mLevel->addEntity(boost::shared_ptr<Entity>(cloud));
}

void ThrownPotion::applySplash(const HitResult* result, std::vector<MobEffectInstance*>* effects)
{
    if (mLevel == NULL || mBoundingBox == NULL || effects == NULL)
        return;
    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntitiesOfClass(typeid(LivingEntity), mBoundingBox->grow(4.0, 2.0, 4.0));
    if (entities == NULL)
        return;
    boost::shared_ptr<Entity> source = shared_from_this();
    boost::shared_ptr<LivingEntity> owner = getOwner();
    for (std::vector<boost::shared_ptr<Entity> >::iterator entityIt = entities->begin();
         entityIt != entities->end(); ++entityIt)
    {
        boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(*entityIt);
        if (!living || !living->isAffectedByPotions())
            continue;
        double distance = distanceToSqr(*entityIt);
        if (distance >= 16.0)
            continue;
        double scale = -(std::sqrt(distance) * 0.25 - 1.0);
        if (result != NULL && result->mEntity == *entityIt)
            scale = 1.0;
        for (std::vector<MobEffectInstance*>::iterator effectIt = effects->begin();
             effectIt != effects->end(); ++effectIt)
        {
            MobEffectInstance* effectInstance = *effectIt;
            if (effectInstance == NULL || effectInstance->getEffect() == NULL)
                continue;
            MobEffect* effect = effectInstance->getEffect();
            if (effect->isInstantenous())
            {
                effect->applyInstantenousEffect(source, owner, living,
                                                 effectInstance->getAmplifier(), scale);
            }
            else
            {
                int duration = static_cast<int>(scale * (effectInstance->getDuration() - 0.0) * 0.75 + 0.5);
                if (duration > 20)
                    living->addEffect(new MobEffectInstance(effect, duration,
                                                             effectInstance->getAmplifier()), source);
            }
        }
    }
}

bool ThrownPotion::onHit(const HitResult* result)
{
    Projectile::onHit(result);
    if (mLevel->mIsClientSide)
        return true;

    not_null_ptr<ItemInstance> item = getItem();
    Potion* potion = PotionUtils::getPotion(item);
    std::vector<MobEffectInstance*>* effects = PotionUtils::getMobEffects(item);
    bool noEffects = effects == NULL || effects->empty();
    bool isWaterSplash = potion == Potions::WATER && noEffects;

    if (isWaterSplash && result->mType == HitResult::BLOCK)
    {
        BlockPos pos = result->getBlockPos().relative(result->mDirection);
        dowseFire(pos, result->mDirection);
        arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
        for (unsigned int i = 0; i < directions.length; ++i)
            dowseFire(pos.relative(directions.data[i]), directions.data[i]);
    }

    bool invertedGrenades = Minecraft::GetInstance()->GetMiniGame()->HasFeatureInvertedExplodingGrenades();

    if (isWaterSplash && !invertedGrenades)
    {
        applyWater();
        if (isLingering())
            makeAreaOfEffectCloud(item, potion);
    }
    else if (!(noEffects && !invertedGrenades))
    {
        if (isLingering())
            makeAreaOfEffectCloud(item, potion);
        else
            applySplash(result, effects);
    }

    bool hasInstant = potion->hasInstantEffects();
    mLevel->levelEvent(hasInstant ? LevelEvent::PARTICLES_INSTANT_POTION_SPLASH : LevelEvent::PARTICLES_SPELL_POTION_SPLASH, BlockPos(this), PotionUtils::getColor(item));
    delete effects;
    remove();
    return true;
}

void ThrownPotion::staticCtor()
{
    DATA_ITEM = SynchedEntityData::defineId(EntityType::THROWN_POTION, EntityType::THROWABLE_PROJECTILE,
                                            EntityDataSerializers::ITEM_INSTANCE);
}

void ThrownPotion::_init()
{
    fjDerivedCtorCalls();
}

void ThrownPotion::setItem(not_null_ptr<ItemInstance> item)
{
    getEntityData()->set(DATA_ITEM, item, true);
    getEntityData()->markDirty(DATA_ITEM);
}

void ThrownPotion::addDataWalkers(DataFixerUpper* fixer)
{
    if (fixer != NULL)
        fixer->addWalker(DataFixTypes::eDataFixTypes_ItemInstance,
                         (DataWalker*)new ItemDataWalker(EntityType::THROWN_POTION, L"Potion"));
}

void ThrownPotion::readAdditionalSaveData(CompoundTag* tag)
{
    ThrowableProjectile::readAdditionalSaveData(tag);
    not_null_ptr<ItemInstance> item(new ItemInstance(tag->getCompound(L"Potion")));
    if (!item->isEmpty())
        setItem(item);
    else
        remove();
}

void ThrownPotion::addAdditonalSaveData(CompoundTag* tag)
{
    ThrowableProjectile::addAdditonalSaveData(tag);
    not_null_ptr<ItemInstance> item = getItem();
    if (!item->isEmpty())
        tag->put(L"Potion", item->save(new CompoundTag()));
}

double ThrownPotion::getGravity()
{
    return 0.05000000074505806;
}

void ThrownPotion::defineSynchedData()
{
    getEntityData()->createDateItem(DATA_ITEM, ItemInstance::EMPTY);
}

unsigned long long ThrownPotion::GetType()
{
    return EntityType::THROWN_POTION;
}

bool ThrownPotion::WATER_SENSITIVE::apply(boost::shared_ptr<Entity> entity) const
{
    return ThrownPotion::isWaterSensitiveEntity(entity);
}

ThrownPotion::WATER_SENSITIVE::~WATER_SENSITIVE()
{
    (void)0;
}
