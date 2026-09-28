#include "game/entity/mob/WitherSkeleton.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Creeper.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

WitherSkeleton::WitherSkeleton(Level* level)
    : AbstractSkeleton(level), mWitherParent()
{
    fjDerivedCtorCalls();
    setSize(0.7f, 2.4f);
    mBlocksBuilding = true;
}

WitherSkeleton::~WitherSkeleton()
{
}

void WitherSkeleton::create(Level* level)
{
    new WitherSkeleton(level);
}

void WitherSkeleton::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::WITHER_SKELETON);
}

void WitherSkeleton::registerGoals()
{
    AbstractSkeleton::registerGoals();
}

MobGroupData* WitherSkeleton::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                                const std::wstring& dataString)
{
    MobGroupData* result = AbstractSkeleton::finalizeMobSpawn(difficulty, data, std::wstring());
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(4.0);
    reassessWeaponGoal();
    return result;
}

double WitherSkeleton::getEyeHeight()
{
    return 2.1f;
}

bool WitherSkeleton::doHurtTarget(boost::shared_ptr<Entity> target)
{
    if (!Monster::doHurtTarget(target))
        return false;

    if (target && (target->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(target);
        living->addEffect(new MobEffectInstance(MobEffects::WITHER, 200),
                          boost::shared_ptr<Entity>());
    }
    return true;
}

void WitherSkeleton::setWitherParent(const boost::shared_ptr<WitherBoss>& parent)
{
    mWitherParent = parent;
}

boost::shared_ptr<WitherBoss> WitherSkeleton::getWitherParent()
{
    return mWitherParent.lock();
}

void WitherSkeleton::die(const DamageSource* source)
{
    AbstractSkeleton::die(source);

    if (source == NULL)
        return;

    boost::shared_ptr<Entity> entity = source->getEntity();
    if (!entity || entity->GetType() != EntityType::CREEPER)
        return;

    boost::shared_ptr<Creeper> creeper = boost::static_pointer_cast<Creeper>(entity);
    if (!creeper->isPowered() || !creeper->canDropMobsSkull())
        return;

    creeper->increaseDroppedSkulls();
    spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 1)), 0.0f);
}

void WitherSkeleton::tick()
{
    AbstractSkeleton::tick();

    boost::shared_ptr<LivingEntity> target = getTarget();
    boost::shared_ptr<WitherBoss> parent = mWitherParent.lock();
    if (target && parent && target.get() == parent.get())
    {
        setLastHurtByMob(boost::shared_ptr<LivingEntity>());
        setTarget(boost::shared_ptr<LivingEntity>());
    }
}

ResourceLocation* WitherSkeleton::getDefaultLootTable()
{
    return &BuiltInLootTables::WITHER_SKELETON;
}

ResourceLocation* WitherSkeleton::getDefaultGearTable()
{
    return &BuiltInLootTables::WITHER_SKELETON_GEAR;
}

const SoundEvent* WitherSkeleton::getAmbientSound()
{
    return SoundEvents::WITHER_SKELETON_AMBIENT;
}

const SoundEvent* WitherSkeleton::getHurtSound(const DamageSource*)
{
    return SoundEvents::WITHER_SKELETON_HURT;
}

const SoundEvent* WitherSkeleton::getDeathSound()
{
    return SoundEvents::WITHER_SKELETON_DEATH;
}

const SoundEvent* WitherSkeleton::getStepSound()
{
    return SoundEvents::WITHER_SKELETON_STEP;
}

void WitherSkeleton::populateDefaultEquipmentEnchantments(const DifficultyInstance&)
{
}

boost::shared_ptr<TippableArrow> WitherSkeleton::getArrow(float distanceFactor)
{
    boost::shared_ptr<TippableArrow> arrow = AbstractSkeleton::getArrow(distanceFactor);
    if (arrow)
        arrow->setSecondsOnFire(100);
    return arrow;
}

unsigned long long WitherSkeleton::GetType()
{
    return EntityType::WITHER_SKELETON;
}
