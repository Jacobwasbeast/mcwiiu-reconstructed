#include "game/core/Creeper.hpp"

#include <algorithm>
#include <typeinfo>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Ocelot.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/SwellGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

static const unsigned long long ENTITY_TYPE_CREEPER = 0x5D540041ULL;
static const unsigned long long ENTITY_TYPE_SKELETON = 0x5D540441ULL;
static const unsigned long long ENTITY_TYPE_PROJECTILE_MASK = 0x8A020000ULL;
static const unsigned long long ENTITY_TYPE_PLAYER_MASK = 0x0C800000ULL;
static const EntityDataAccessor<int>* DATA_SWELL_DIR = NULL;
static const EntityDataAccessor<bool>* DATA_IS_POWERED = NULL;
static const EntityDataAccessor<bool>* DATA_IS_IGNITED = NULL;

Creeper::Creeper(Level* level) : Monster(level)
{
    fjDerivedCtorCalls();
    _init();
    setSize(0.6f, 1.7f);
}

Creeper::~Creeper() = default;

void Creeper::staticCtor()
{
    DATA_SWELL_DIR = SynchedEntityData::defineId<int>(ENTITY_TYPE_CREEPER, ENTITY_TYPE_MONSTER, EntityDataSerializers::INT);
    DATA_IS_POWERED = SynchedEntityData::defineId<bool>(ENTITY_TYPE_CREEPER, ENTITY_TYPE_MONSTER, EntityDataSerializers::BOOLEAN);
    DATA_IS_IGNITED = SynchedEntityData::defineId<bool>(ENTITY_TYPE_CREEPER, ENTITY_TYPE_MONSTER, EntityDataSerializers::BOOLEAN);
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
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, new SwellGoal(this), true);
    mGoalSelector.addGoal(3, new AvoidEntityGoal(this, typeid(Ocelot), 6.0f, 1.0, 1.2), true);
    mGoalSelector.addGoal(4, new MeleeAttackGoal(this, 1.0, false), true);
    mGoalSelector.addGoal(5, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(6, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new NearestAttackableTargetGoal(this, typeid(Player), true, false, true), true);
    mTargetSelector.addGoal(2, new HurtByTargetGoal(this, false, arrayWithLength<unsigned long long>()), true);
}

void Creeper::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
}

int Creeper::getMaxFallDistance()
{
    boost::shared_ptr<LivingEntity> target = getTarget();
    if (!target) return 3;
    return (int)(getHealth() - 1.0f) + 3;
}

void Creeper::causeFallDamage(float fallDistance, float damageMultiplier)
{
    LivingEntity::causeFallDamage(fallDistance, damageMultiplier);
    mSwell = std::min(mSwell + (int)(fallDistance * 1.5f), mMaxSwell - 5);
}

void Creeper::defineSynchedData()
{
    Monster::defineSynchedData();
    getEntityData()->createDateItem(DATA_SWELL_DIR, -1);
    getEntityData()->createDateItem(DATA_IS_POWERED, false);
    getEntityData()->createDateItem(DATA_IS_IGNITED, false);
}

void Creeper::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, ENTITY_TYPE_CREEPER);
}

bool Creeper::isIgnited()
{
    return getEntityData()->get(DATA_IS_IGNITED);
}

void Creeper::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    if (isPowered()) tag->putBoolean(L"powered", true);
    tag->putShort(L"Fuse", (short)mMaxSwell);
    tag->putByte(L"ExplosionRadius", (unsigned char)mExplosionRadius);
    tag->putBoolean(L"ignited", isIgnited());
}

void Creeper::ignite()
{
    getEntityData()->set(DATA_IS_IGNITED, true, true);
}

void Creeper::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    getEntityData()->set(DATA_IS_POWERED, tag->getBoolean(L"powered"), true);
    if (tag->contains(L"Fuse", 99)) mMaxSwell = tag->getShort(L"Fuse");
    if (tag->contains(L"ExplosionRadius", 99)) mExplosionRadius = tag->getByte(L"ExplosionRadius");
    if (tag->getBoolean(L"ignited")) ignite();
}

int Creeper::getSwellDir()
{
    return getEntityData()->get(DATA_SWELL_DIR);
}

void Creeper::setSwellDir(int swellDir)
{
    getEntityData()->set(DATA_SWELL_DIR, swellDir, true);
}

void Creeper::spawnLingeringCloud()
{
    std::vector<MobEffectInstance*>* effects = getActiveEffects();
    if (!effects->empty())
    {
        boost::shared_ptr<AreaEffectCloud> cloud(new AreaEffectCloud(mLevel, mX, mY, mZ));
        cloud->setRadius(2.5f);
        cloud->setRadiusOnUse(-0.5f);
        cloud->setWaitTime(10);
        cloud->setDuration(cloud->getDuration() / 2);
        cloud->setRadiusPerTick(-cloud->getRadius() / (float)cloud->getDuration());
        for (std::vector<MobEffectInstance*>::iterator effect = effects->begin(); effect != effects->end(); ++effect)
        {
            cloud->addEffect(new MobEffectInstance(*effect));
        }
        mLevel->addEntity(cloud);
    }
    delete effects;
}

void Creeper::explodeCreeper()
{
    if (!mLevel->mIsClientSide)
    {
        bool mobGriefing = mLevel->getGameRules()->getBoolean(1);
        float power = isPowered() ? 2.0f : 1.0f;
        mDead = true;
        mLevel->explode(shared_from_this(), mX, mY, mZ, (float)mExplosionRadius * power,
                        false, mobGriefing, false, false, false, -1.0f);
        remove();
        spawnLingeringCloud();
    }
}

void Creeper::tick()
{
    PIXBeginNamedEvent(0.0f, "Creeper tick");
    mOldSwell = mSwell;
    if (isAlive())
    {
        if (isIgnited()) setSwellDir(1);
        int swellDir = getSwellDir();
        if (swellDir >= 1 && mSwell == 0) playSound(SoundEvents::CREEPER_PRIMED, 1.0f, 0.5f);
        mSwell += swellDir;
        if (mSwell < 0) mSwell = 0;
        if (mSwell >= mMaxSwell)
        {
            mSwell = mMaxSwell;
            explodeCreeper();
        }
    }
    Monster::tick();
    PIXEndNamedEvent();
}

const SoundEvent* Creeper::getHurtSound(const DamageSource* source) { return SoundEvents::CREEPER_HURT; }
const SoundEvent* Creeper::getDeathSound() { return SoundEvents::CREEPER_DEATH; }

bool Creeper::canDropMobsSkull()
{
    return mDroppedSkulls < 1 && mLevel->getGameRules()->getBoolean(4);
}

void Creeper::increaseDroppedSkulls() { ++mDroppedSkulls; }

void Creeper::die(const DamageSource* source)
{
    LivingEntity::die(source);

    if (mLevel->getGameRules()->getBoolean(4))
    {
        boost::shared_ptr<Entity> attacker = source->getEntity();
        if (attacker && attacker->GetType() == ENTITY_TYPE_SKELETON)
        {
            int firstRecord = Item::VALIDgetId(Items::record13);
            int recordCount = Item::VALIDgetId(Items::recordWait) - firstRecord + 1;
            spawnAtLocation(Item::byId(firstRecord + mRandom->nextInt(recordCount)), 1);
        }
        else if (attacker && attacker->GetType() == ENTITY_TYPE_CREEPER)
        {
            boost::shared_ptr<Creeper> creeper = boost::dynamic_pointer_cast<Creeper>(attacker);
            if (creeper.get() != this && creeper->isPowered() && creeper->canDropMobsSkull())
            {
                creeper->increaseDroppedSkulls();
                spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 4)), 0.0f);
            }
        }
    }

    boost::shared_ptr<Entity> directEntity = source->getDirectEntity();
    boost::shared_ptr<Entity> attacker = source->getEntity();
    if (directEntity && attacker
        && (directEntity->GetType() & ENTITY_TYPE_PROJECTILE_MASK) == ENTITY_TYPE_PROJECTILE_MASK
        && (attacker->GetType() & ENTITY_TYPE_PLAYER_MASK) == ENTITY_TYPE_PLAYER_MASK)
    {
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(attacker);
        player->awardStat(GenericStats::archer(), GenericStats::param_archer());
    }
}

bool Creeper::doHurtTarget(boost::shared_ptr<Entity> target) { return true; }

bool Creeper::isPowered()
{
    return getEntityData()->get(DATA_IS_POWERED);
}

float Creeper::getSwelling(float partialTicks)
{
    return ((float)mOldSwell + (float)(mSwell - mOldSwell) * partialTicks) / (float)(mMaxSwell - 2);
}

ResourceLocation* Creeper::getDefaultLootTable() { return &BuiltInLootTables::CREEPER; }

void Creeper::thunderHit(const LightningBolt* lightning)
{
    Entity::thunderHit(lightning);
    getEntityData()->set(DATA_IS_POWERED, true, true);
}

unsigned int Creeper::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == 259) return 0xA3BAC985;
    return (unsigned int)-1;
}

bool Creeper::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (!item->isEmpty() && item->getItem() == Items::flintAndSteel)
    {
        mLevel->playSound(player, mX + 0.5, mY + 0.5, mZ + 0.5, SoundEvents::FLINTANDSTEEL_USE,
                          getSoundSource(), 1.0f, mRandom->nextFloat() * 0.4f + 0.8f, 16.0f);
        player->swing(hand);
        if (!mLevel->mIsClientSide)
        {
            ignite();
            item->hurtAndBreak(1, player);
            return true;
        }
    }
    return Mob::mobInteract(player, hand);
}

unsigned long long Creeper::GetType() { return ENTITY_TYPE_CREEPER; }

void Creeper::create(Level* level) { new Creeper(level); }
