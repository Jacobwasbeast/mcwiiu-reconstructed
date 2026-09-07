#include "game/entity/mob/Skeleton.hpp"

#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Creeper.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/ai/RestrictSunGoal.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

static const unsigned long long ENTITY_TYPE_SKELETON = 0x5D540441ULL;
static const unsigned long long ENTITY_TYPE_TIPPABLE_ARROW = 0x8A020002ULL;

Skeleton::Skeleton(Level* level)
    : AbstractSkeleton(level)
{
    Mob::fjDerivedCtorCalls();
}

Skeleton::~Skeleton()
{
}

void Skeleton::create(Level* level)
{
    new Skeleton(level);
}

void Skeleton::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, ENTITY_TYPE_SKELETON);
}

void Skeleton::registerGoals()
{
    AbstractSkeleton::registerGoals();
    mGoalSelector.addGoal(2, new RestrictSunGoal(this), true);
    mGoalSelector.addGoal(4, new AvoidEntityGoal(this, typeid(Creeper), 6.0f, 1.0, 1.2), true);
}

const void* Skeleton::getDefaultLootTable()
{
    return &BuiltInLootTables::SKELETON;
}

const void* Skeleton::getDefaultGearTable()
{
    return &BuiltInLootTables::SKELETON_GEAR;
}

const SoundEvent* Skeleton::getAmbientSound()
{
    return SoundEvents::SKELETON_AMBIENT;
}

const SoundEvent* Skeleton::getHurtSound(const DamageSource*)
{
    return SoundEvents::SKELETON_HURT;
}

const SoundEvent* Skeleton::getDeathSound()
{
    return SoundEvents::SKELETON_DEATH;
}

const SoundEvent* Skeleton::getStepSound()
{
    return SoundEvents::SKELETON_STEP;
}

void Skeleton::die(const DamageSource* source)
{
    AbstractSkeleton::die(source);

    boost::shared_ptr<Entity> attacker = source->getEntity();
    if (attacker && attacker->GetType() == 0x5D540041ULL)
    {
        boost::shared_ptr<Creeper> creeper = boost::static_pointer_cast<Creeper>(attacker);
        if (creeper->canDropMobsSkull())
        {
            creeper->increaseDroppedSkulls();
            spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 0)), 3.0f);
        }
    }
}

boost::shared_ptr<TippableArrow> Skeleton::getArrow(float distanceFactor)
{
    not_null_ptr<ItemInstance> item = getItemSlot(EquipmentSlot::MAINHAND);
    boost::shared_ptr<TippableArrow> arrow = AbstractSkeleton::getArrow(distanceFactor);
    if (item->getItem() == Items::tippedArrow && arrow && arrow->GetType() == ENTITY_TYPE_TIPPABLE_ARROW)
        arrow->setEffectsFromItem(item);
    return arrow;
}

unsigned long long Skeleton::GetType()
{
    return ENTITY_TYPE_SKELETON;
}
