#include "game/core/Cow.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/GenericStats.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowParentGoal.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"

Cow::Cow(Level* level, bool callDerivedCtor)
    : Animal(level)
{
    if (callDerivedCtor)
        fjDerivedCtorCalls();
    setSize(0.9f, 1.4f);
}

Cow::~Cow() = default;

void Cow::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x2d6a0000ULL);
}

void Cow::registerGoals()
{
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new PanicGoal(this, 2.0), true);
    mGoalSelector.addGoal(2, new BreedGoal(this, 1.0), true);
    mGoalSelector.addGoal(4, new FollowParentGoal(this, 1.25), true);
    mGoalSelector.addGoal(5, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 6.0f), true);
    mGoalSelector.addGoal(7, new RandomLookAroundGoal(this), true);
}

void Cow::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(10.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2);
}

const SoundEvent* Cow::getAmbientSound()
{
    return SoundEvents::COW_AMBIENT;
}

const SoundEvent* Cow::getHurtSound(const DamageSource*)
{
    return SoundEvents::COW_HURT;
}

const SoundEvent* Cow::getDeathSound()
{
    return SoundEvents::COW_DEATH;
}

void Cow::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::COW_STEP, 0.15f, 1.0f);
}

float Cow::getSoundVolume()
{
    return 0.4f;
}

ResourceLocation* Cow::getDefaultLootTable()
{
    return &BuiltInLootTables::COW;
}

bool Cow::mobInteract(const boost::shared_ptr<Player>& player,
                      InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (itemInstance->getItem() != Items::bucket || isBaby())
        return Animal::mobInteract(player, hand);

    player->awardStat(gStats->get_cowsMilked(), GenericStats::param_cowsMilked());
    playSound(SoundEvents::COW_MILK, 1.0f, 1.0f);
    itemInstance->shrink(1);

    if (!itemInstance->isEmpty())
    {
        not_null_ptr<ItemInstance> milkBucket(new ItemInstance(Items::milkBucket));
        if (!player->mInventory->add(milkBucket))
            player->drop(not_null_ptr<ItemInstance>(new ItemInstance(Items::milkBucket, 1, 0)), false);
    }
    else
    {
        player->setItemInHand(hand, not_null_ptr<ItemInstance>(new ItemInstance(Items::milkBucket)));
    }

    return true;
}

boost::shared_ptr<AgableMob> Cow::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<AgableMob>(new Cow(mLevel, true));
}

double Cow::getEyeHeight()
{
    return isBaby() ? (double)mBbHeight : 1.3;
}

unsigned int Cow::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_pItemInstance->getItem() == Items::bucket && !isBaby())
        return 0x735a2f66U;
    return Animal::GetInteractTooltip(tooltipData);
}

unsigned long long Cow::GetType()
{
    return 0x000000002d6a0000ULL;
}

void Cow::create(Level* level)
{
    new Cow(level, true);
}
