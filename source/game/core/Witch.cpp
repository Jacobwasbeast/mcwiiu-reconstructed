#include "game/core/Witch.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/RangedAttackGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"

static const EntityDataAccessor<bool>* witchUsingItem = NULL;
static AttributeModifier* witchDrinkingSpeedPenalty =
    (new AttributeModifier(eMODIFIER_ID_WitchDrinkingSpeedPenalty, -0.25, 0))->setSerialize(false);

Witch::Witch(Level* level) : Monster(level), mUsingTime(0)
{
    fjDerivedCtorCalls();
    setSize(0.6f, 1.95f);
    mCategories |= 0xc;
}

Witch::~Witch()
{
}

void Witch::create(Level* level)
{
    new Witch(level);
}

void Witch::performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor)
{
    if (isDrinkingPotion())
        return;

    double targetY = target->mY + target->getEyeHeight() - 1.1;
    targetY -= 1.1;
    double dx = target->mX + target->mXd - mX;
    double dz = target->mZ + target->mZd - mZ;
    double distance = Mth::sqrt(dx * dx + dz * dz);
    Potion* potion = Potions::HARMING;

    if (distance >= 8.0 && !target->hasEffect(MobEffects::MOVEMENT_SLOWDOWN))
        potion = Potions::SLOWNESS;
    else if (target->getHealth() >= 8.0f && !target->hasEffect(MobEffects::POISON))
        potion = Potions::POISON;
    else if (distance <= 3.0 && !target->hasEffect(MobEffects::WEAKNESS)
             && mRandom->nextFloat() < 0.25f)
        potion = Potions::WEAKNESS;

    not_null_ptr<ItemInstance> item(new ItemInstance(Items::splashPotion));
    PotionUtils::setPotion(item, potion);
    boost::shared_ptr<ThrownPotion> thrown(new ThrownPotion(mLevel,
        boost::static_pointer_cast<LivingEntity>(shared_from_this()), item));
    thrown->mXRot -= 20.0f;
    thrown->shoot(dx, targetY - mY + distance * 0.2, dz, 0.75f, 8.0f);
    playSound(SoundEvents::WITCH_THROW, 1.0f, 0.8f + mRandom->nextFloat() * 0.4f);
    mLevel->addEntity(boost::static_pointer_cast<Entity>(thrown));
}

bool Witch::isAggressive()
{
    return false;
}

void Witch::setAggressive(bool)
{
}

void Witch::staticCtor()
{
    witchUsingItem = SynchedEntityData::defineId<bool>(0x5D540046ULL, ENTITY_TYPE_MONSTER,
        EntityDataSerializers::BOOLEAN);
}

void Witch::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x5D540046ULL);
}

void Witch::registerGoals()
{
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(2,
        new RangedAttackGoal(static_cast<RangedAttackMob*>(this), this, 1.0, 60, 10.0f), true);
    mGoalSelector.addGoal(2, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(3, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(3, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1,
        new HurtByTargetGoal(this, false, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(2,
        new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
}

void Witch::defineSynchedData()
{
    Mob::defineSynchedData();
    getEntityData()->createDateItem(witchUsingItem, false);
}

const SoundEvent* Witch::getAmbientSound()
{
    return SoundEvents::WITCH_AMBIENT;
}

const SoundEvent* Witch::getHurtSound(const DamageSource*)
{
    return SoundEvents::WITCH_HURT;
}

const SoundEvent* Witch::getDeathSound()
{
    return SoundEvents::WITCH_DEATH;
}

void Witch::setUsingItem(bool usingItem)
{
    getEntityData()->set(witchUsingItem, usingItem, true);
}

bool Witch::isDrinkingPotion()
{
    return getEntityData()->get(witchUsingItem);
}

void Witch::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(26.0);
}

void Witch::aiStep()
{
    if (!mLevel->mIsClientSide)
    {
        if (!isDrinkingPotion())
        {
            Potion* potion = NULL;

            if (mRandom->nextFloat() < 0.15f && isUnderLiquid(Material::WATER)
                && !hasEffect(MobEffects::WATER_BREATHING))
            {
                potion = Potions::WATER_BREATHING;
            }
            else if (mRandom->nextFloat() < 0.15f
                     && (isOnFire()
                         || (getLastDamageSource() != NULL && getLastDamageSource()->isFire()))
                     && !hasEffect(MobEffects::FIRE_RESISTANCE))
            {
                potion = Potions::FIRE_RESISTANCE;
            }
            else if (mRandom->nextFloat() < 0.05f && getHealth() < getMaxHealth())
            {
                potion = Potions::HEALING;
            }
            else if (mRandom->nextFloat() < 0.25f && getTarget()
                     && !hasEffect(MobEffects::MOVEMENT_SPEED)
                     && distanceToSqr(getTarget()) > 121.0)
            {
                potion = Potions::SWIFTNESS;
            }

            if (potion != NULL)
            {
                not_null_ptr<ItemInstance> item(new ItemInstance(Items::potion));
                PotionUtils::setPotion(item, potion);
                setItemSlot(EquipmentSlot::MAINHAND, item);
                mUsingTime = getMainHandItem()->getUseDuration();
                setUsingItem(true);
                playSound(SoundEvents::WITCH_DRINK, 1.0f,
                          0.8f + mRandom->nextFloat() * 0.4f);

                AttributeInstance* speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
                speed->removeModifier(witchDrinkingSpeedPenalty);
                speed->addModifier(new AttributeModifier(*witchDrinkingSpeedPenalty));
            }
        }
        else if (mUsingTime-- <= 0)
        {
            setUsingItem(false);
            not_null_ptr<ItemInstance> item = getMainHandItem();
            setItemSlot(EquipmentSlot::MAINHAND, ItemInstance::EMPTY);

            if (item->getItem() == Items::potion)
            {
                std::vector<MobEffectInstance*>* effects = PotionUtils::getMobEffects(item);
                if (effects != NULL)
                {
                    for (std::vector<MobEffectInstance*>::iterator it = effects->begin();
                         it != effects->end(); ++it)
                    {
                        addEffect(new MobEffectInstance(*it), shared_from_this());
                    }
                    delete effects;
                }
            }

            getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)
                ->removeModifier(witchDrinkingSpeedPenalty);
        }

        if (mRandom->nextFloat() < 0.00075f)
            mLevel->broadcastEntityEvent(shared_from_this(), 15, 0);
    }

    Monster::aiStep();
}

void Witch::handleEntityEvent(unsigned char event, int data)
{
    if (event == 15)
    {
        for (int i = 0; i < 10 + mRandom->nextInt(35); ++i)
            mLevel->addParticle(PARTICLE_WITCH_MAGIC, mX + mRandom->nextGaussian() * 0.13,
                mBoundingBox->mMaxY + 0.5 + mRandom->nextGaussian() * 0.13,
                mZ + mRandom->nextGaussian() * 0.13,
                0.0, 0.0, 0.0, arrayWithLength<int>());
        return;
    }
    Monster::handleEntityEvent(event, data);
}

float Witch::getDamageAfterMagicAbsorb(const DamageSource* source, float damage)
{
    float result = LivingEntity::getDamageAfterMagicAbsorb(source, damage);
    if (source->getEntity().get() == this)
        result = 0.0f;
    if (source->isMagic())
        result *= 0.15f;
    return result;
}

const void* Witch::getDefaultLootTable()
{
    return &BuiltInLootTables::WITCH;
}

double Witch::getEyeHeight()
{
    return 1.62;
}

unsigned long long Witch::GetType()
{
    return 0x5D540046ULL;
}
