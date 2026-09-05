#include "game/core/Creeper.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GameRules.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Blocks.hpp"

const EntityDataAccessor<int>* Creeper::DATA_SWELL_DIR = NULL;
const EntityDataAccessor<bool>* Creeper::DATA_IS_POWERED = NULL;
const EntityDataAccessor<bool>* Creeper::DATA_IS_IGNITED = NULL;

Creeper::Creeper(Level* level) : Monster(level), mOldSwell(0), mSwell(0), mMaxSwell(30), mExplosionRadius(3), mDroppedSkulls(0)
{
    _init();
    setSize(0.6f, 1.7f);
}

Creeper::~Creeper()
{
}

void Creeper::staticCtor()
{
    DATA_SWELL_DIR = SynchedEntityData::defineId<int>(0x000000005D540041ULL, ENTITY_TYPE_MONSTER, EntityDataSerializers::INT);
    DATA_IS_POWERED = SynchedEntityData::defineId<bool>(0x000000005D540041ULL, ENTITY_TYPE_MONSTER, EntityDataSerializers::BOOLEAN);
    DATA_IS_IGNITED = SynchedEntityData::defineId<bool>(0x000000005D540041ULL, ENTITY_TYPE_MONSTER, EntityDataSerializers::BOOLEAN);
}

void Creeper::_init()
{
    mOldSwell = 0;
    mSwell = 0;
    mMaxSwell = 30;
    mExplosionRadius = 3;
    mDroppedSkulls = 0;
}

void Creeper::registerGoals()
{
}

void Creeper::registerAttributes()
{
    Monster::registerAttributes();
}

int Creeper::getMaxFallDistance()
{
    boost::shared_ptr<LivingEntity> target = getTarget();
    if (target == NULL) return 3;
    return (int)((double)getHealth() - 1.0) + 3;
}

void Creeper::causeFallDamage(float fallDistance, float damageMultiplier)
{
    LivingEntity::causeFallDamage(fallDistance, damageMultiplier);
    int value = mSwell + (int)(fallDistance * 1.5f);
    int limit = mMaxSwell - 5;
    mSwell = value < limit ? value : limit;
}

void Creeper::defineSynchedData()
{
    Monster::defineSynchedData();
    if (DATA_SWELL_DIR != NULL) getEntityData()->createDateItem(DATA_SWELL_DIR, -1);
    if (DATA_IS_POWERED != NULL) getEntityData()->createDateItem(DATA_IS_POWERED, false);
    if (DATA_IS_IGNITED != NULL) getEntityData()->createDateItem(DATA_IS_IGNITED, false);
}

void Creeper::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer);
}

bool Creeper::isIgnited()
{
    return DATA_IS_IGNITED != NULL && getEntityData()->get(DATA_IS_IGNITED);
}

void Creeper::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    if (tag == NULL) return;
    if (isPowered()) tag->putBoolean(L"powered", true);
    tag->putShort(L"Fuse", (short)mMaxSwell);
    tag->putByte(L"ExplosionRadius", (unsigned char)mExplosionRadius);
    tag->putBoolean(L"ignited", isIgnited());
}

void Creeper::ignite()
{
    if (DATA_IS_IGNITED != NULL) getEntityData()->set(DATA_IS_IGNITED, true, true);
}

void Creeper::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag == NULL) return;
    if (DATA_IS_POWERED != NULL) getEntityData()->set(DATA_IS_POWERED, tag->getBoolean(L"powered"), true);
    if (tag->contains(L"Fuse", 99)) mMaxSwell = tag->getShort(L"Fuse");
    if (tag->contains(L"ExplosionRadius", 99)) mExplosionRadius = tag->getByte(L"ExplosionRadius");
    if (tag->getBoolean(L"ignited")) ignite();
}

int Creeper::getSwellDir()
{
    return DATA_SWELL_DIR == NULL ? -1 : getEntityData()->get(DATA_SWELL_DIR);
}

void Creeper::setSwellDir(int swellDir)
{
    if (DATA_SWELL_DIR != NULL) getEntityData()->set(DATA_SWELL_DIR, swellDir, true);
}

bool Creeper::spawnLingeringCloud()
{
    std::vector<MobEffectInstance*>* effects = getActiveEffects();
    if (effects == NULL || effects->empty() || mLevel == NULL) return false;
    boost::shared_ptr<AreaEffectCloud> cloud(new AreaEffectCloud(mLevel, mX, mY, mZ));
    cloud->setRadius(3.0f);
    cloud->setRadiusOnUse(-0.5f);
    cloud->setWaitTime(10);
    cloud->setDuration(cloud->getDuration() / 2);
    cloud->setRadiusPerTick(-cloud->getRadius() / (float)cloud->getDuration());
    for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
    {
        cloud->addEffect(new MobEffectInstance(**it));
    }
    boost::shared_ptr<Entity> entity(cloud, (Entity*)cloud.get());
    mLevel->addEntity(entity);
    return true;
}

void Creeper::explodeCreeper()
{
    if (mLevel == NULL) return;
    mLevel->explode(shared_from_this(), mX, mY, mZ, (float)mExplosionRadius, false, true, 0, true, true, 0.0f);
    remove();
}

void Creeper::tick()
{
    mOldSwell = mSwell;
    if (isIgnited()) setSwellDir(1);
    int direction = getSwellDir();
    if (direction < 1) mSwell = mSwell - direction;
    else if (mSwell == 0) playSound(SoundEvents::CREEPER_PRIMED, 1.0f, 0.5f);
    mSwell += direction;
    if (mSwell < 0) mSwell = 0;
    if (mSwell >= mMaxSwell)
    {
        mSwell = mMaxSwell;
        explodeCreeper();
    }
    Monster::tick();
}

const SoundEvent* Creeper::getHurtSound(const DamageSource* source) { return SoundEvents::CREEPER_HURT; }
const SoundEvent* Creeper::getDeathSound() { return SoundEvents::CREEPER_DEATH; }

bool Creeper::canDropMobsSkull()
{
    return mDroppedSkulls < 1 && mLevel != NULL && mLevel->getGameRules()->getBoolean(4);
}

void Creeper::increaseDroppedSkulls() { ++mDroppedSkulls; }

void Creeper::die(const DamageSource* source)
{
    LivingEntity::die(source);
}

bool Creeper::doHurtTarget(boost::shared_ptr<Entity> target) { return true; }

bool Creeper::isPowered()
{
    return DATA_IS_POWERED != NULL && getEntityData()->get(DATA_IS_POWERED);
}

double Creeper::getSwelling(float partialTicks)
{
    return ((double)mOldSwell + (double)(mSwell - mOldSwell) * partialTicks) / (double)(mMaxSwell - 2);
}

const void* Creeper::getDefaultLootTable() { return &BuiltInLootTables::CREEPER; }

void Creeper::thunderHit(const LightningBolt* lightning)
{
    Entity::thunderHit(lightning);
    if (DATA_IS_POWERED != NULL) getEntityData()->set(DATA_IS_POWERED, true, true);
}

unsigned int Creeper::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_pItemInstance && Item::getId(tooltipData.m_pItemInstance->getItem()) == 259) return 0xA3BAC985;
    return (unsigned int)-1;
}

bool Creeper::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = getItemInHand(hand);
    if (!item->isEmpty() && Item::getId(item->getItem()) == 259)
    {
        ignite();
        boost::shared_ptr<LivingEntity> living(player, (LivingEntity*)player.get());
        item->hurtAndBreak(1, living);
        return true;
    }
    return Mob::mobInteract(player, hand);
}

unsigned long long Creeper::GetType() { return 0x000000005D540041ULL; }

void Creeper::create(Level* level) { new Creeper(level); }
