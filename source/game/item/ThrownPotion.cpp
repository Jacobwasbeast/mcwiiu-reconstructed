#include "game/item/ThrownPotion.hpp"

#include <typeinfo>
#include <cmath>

#include "game/core/Direction.hpp"
#include "game/core/HitResult.hpp"
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
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

const EntityDataAccessor<not_null_ptr<ItemInstance> >* ThrownPotion::DATA_ITEM = NULL;

ThrownPotion::ThrownPotion(Level* level)
    : ThrowableProjectile(level), mItem()
{
    _init();
}

ThrownPotion::ThrownPotion(Level* level, boost::shared_ptr<LivingEntity> owner, not_null_ptr<ItemInstance> item)
    : ThrowableProjectile(level, owner), mItem()
{
    _init();
    setItem(item);
}

ThrownPotion::ThrownPotion(Level* level, double x, double y, double z, not_null_ptr<ItemInstance> item)
    : ThrowableProjectile(level, x, y, z), mItem()
{
    _init();
    setItem(item);
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
    return mItem;
}

void ThrownPotion::dowseFire(const BlockPos& pos, const Direction* direction)
{
    if (Block::getId(mLevel->getBlockState(pos)->getBlock()) == 0)
    {
        mLevel->extinguishFire(boost::shared_ptr<Player>(), pos, direction->getOpposite());
    }
}

bool ThrownPotion::isWaterSensitiveEntity(const boost::shared_ptr<Entity>& entity)
{
    if (!entity)
        return true;
    unsigned long long type = entity->GetType();
    return type == 0x5D540043ULL || type == 0x5D540045ULL || type == 0x2D580001ULL;
}

void ThrownPotion::applyWater()
{
    if (mLevel == NULL || mBoundingBox == NULL)
        return;
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(Entity), mBoundingBox->grow(4.0, 2.0, 4.0));
    if (entities == NULL)
        return;
    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        if (isWaterSensitiveEntity(*it) && distanceToSqr(*it) < 16.0)
            (*it)->clearFire();
    }
}

bool ThrownPotion::isLingering()
{
    return getItem()->getItem() != NULL && Item::getId(getItem()->getItem()) == 0;
}

void ThrownPotion::makeAreaOfEffectCloud(const not_null_ptr<ItemInstance>& item, Potion* potion)
{
    AreaEffectCloud* cloud = new AreaEffectCloud(mLevel, mX, mY, mZ);
    cloud->setOwner(boost::shared_ptr<LivingEntity>());
    cloud->setPotion(potion);
    std::vector<MobEffectInstance*>* effects = PotionUtils::getCustomEffects(item);
    if (effects != NULL)
    {
        std::vector<MobEffectInstance*>::iterator it;
        for (it = effects->begin(); it != effects->end(); ++it)
            cloud->addEffect(*it);
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
    Potion* potion = PotionUtils::getPotion(getItem());
    std::vector<MobEffectInstance*>* effects = PotionUtils::getMobEffects(getItem());
    if (isLingering())
        makeAreaOfEffectCloud(getItem(), potion);
    else
        applySplash(result, effects);
    if (mLevel != NULL)
        mLevel->levelEvent(potion != NULL && potion->hasInstantEffects() ? 0x7D7 : 0x7D2, BlockPos(this), PotionUtils::getColor(getItem()));
    remove();
    return true;
}

void ThrownPotion::staticCtor()
{
    DATA_ITEM = SynchedEntityData::defineId(0, 0x8A200004ULL,
                                            EntityDataSerializers::ITEM_INSTANCE);
}

void ThrownPotion::_init()
{
    fjDerivedCtorCalls();
}

void ThrownPotion::setItem(not_null_ptr<ItemInstance> item)
{
    mItem = item;
    if (mEntityData)
        mEntityData->set(DATA_ITEM, item, true);
}

void ThrownPotion::addDataWalkers(DataFixerUpper* fixer)
{
    if (fixer != NULL)
        fixer->addWalker(DataFixTypes::eDataFixTypes_ItemInstance,
                         (DataWalker*)new ItemDataWalker(0x8a200004ULL, L"Potion"));
}

void ThrownPotion::readAdditionalSaveData(CompoundTag* tag)
{
    ThrowableProjectile::readAdditionalSaveData(tag);
    if (tag != NULL && tag->contains(L"Potion"))
    {
        not_null_ptr<ItemInstance> item(new ItemInstance(tag->getCompound(L"Potion")));
        if (!item->isEmpty())
            setItem(item);
    }
}

void ThrownPotion::addAdditonalSaveData(CompoundTag* tag)
{
    ThrowableProjectile::addAdditonalSaveData(tag);
    if (tag != NULL && !mItem->isEmpty())
        tag->put(L"Potion", mItem->save(new CompoundTag()));
}

double ThrownPotion::getGravity()
{
    return 0.05000000074505806;
}

void ThrownPotion::defineSynchedData()
{
    ThrowableProjectile::defineSynchedData();
    mEntityData->createDateItem(DATA_ITEM, ItemInstance::EMPTY);
}

unsigned long long ThrownPotion::GetType()
{
    return 0x8A200004ULL;
}

bool ThrownPotion::WATER_SENSITIVE::apply(boost::shared_ptr<Entity> entity) const
{
    return ThrownPotion::isWaterSensitiveEntity(entity);
}

ThrownPotion::WATER_SENSITIVE::~WATER_SENSITIVE()
{
}
