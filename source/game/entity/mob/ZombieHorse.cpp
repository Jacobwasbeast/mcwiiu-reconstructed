#include "game/entity/mob/ZombieHorse.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/SinkInWaterRandomStrollGoal.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/generation/Player.hpp"

ZombieHorse::ZombieHorse(Level* level)
    : AbstractHorse(level)
{
    fjDerivedCtorCalls();
}

ZombieHorse::~ZombieHorse()
{
}

void ZombieHorse::create(Level* level)
{
    new ZombieHorse(level);
}

void ZombieHorse::addDataWalkers(DataFixerUpper* fixer)
{
    AbstractHorse::addDataWalkers(fixer, 0x2d690003ULL);
}

void ZombieHorse::registerAttributes()
{
    AbstractHorse::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(15.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2);
    getAttributes()->getInstance(eATTRIBUTE_ID_HorseJumpStrength)->setBaseValue(generateRandomJumpStrength());
}

void ZombieHorse::registerGoals()
{
    AbstractHorse::registerGoals();
    mGoalSelector.removeGoal(mFloatGoal);
    mGoalSelector.removeGoal(mWaterAvoidingRandomStrollGoal);
    mGoalSelector.addGoal(6, new SinkInWaterRandomStrollGoal(this, 0.7), true);
}

EMobType ZombieHorse::getMobType()
{
    return MOB_TYPE_UNDEAD;
}

boost::shared_ptr<AgableMob> ZombieHorse::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<AgableMob>(new ZombieHorse(mLevel));
}

const SoundEvent* ZombieHorse::getAmbientSound()
{
    AbstractHorse::getAmbientSound();
    return SoundEvents::ZOMBIE_HORSE_AMBIENT;
}

const SoundEvent* ZombieHorse::getDeathSound()
{
    AbstractHorse::getDeathSound();
    return SoundEvents::ZOMBIE_HORSE_DEATH;
}

const SoundEvent* ZombieHorse::getHurtSound(const DamageSource* source)
{
    AbstractHorse::getHurtSound(source);
    return SoundEvents::ZOMBIE_HORSE_HURT;
}

ResourceLocation* ZombieHorse::getDefaultLootTable()
{
    return &BuiltInLootTables::ZOMBIE_HORSE;
}

bool ZombieHorse::mobInteract(const boost::shared_ptr<Player>& player,
                              InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (!item->isEmpty() && item->getItem() == Items::spawnEgg)
        return AbstractHorse::mobInteract(player, hand);
    if (!isTamed())
        return false;
    if (isBaby())
        return AbstractHorse::mobInteract(player, hand);
    if (!player->isSneaking())
    {
        if (isVehicle())
            return AbstractHorse::mobInteract(player, hand);
        if (!item->isEmpty())
        {
            if (!isSaddled() && item->getItem() == Items::saddle)
            {
                openInventory(player);
                return true;
            }
            if (item->interactEnemy(player,
                                    boost::static_pointer_cast<LivingEntity>(shared_from_this()),
                                    hand))
                return true;
        }
        doPlayerRide(player);
    }
    else
    {
        openInventory(player);
    }
    return true;
}

unsigned int ZombieHorse::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_bAllowedToInteract)
    {
        not_null_ptr<ItemInstance> item = tooltipData.m_pItemInstance;
        unsigned int itemId = Item::getId(item->getItem());
        bool horseArmor = itemId >= 417 && itemId <= 419 || itemId == 453;
        boost::shared_ptr<Player> player(tooltipData.m_pPlayer,
                                         (Player*)tooltipData.m_pPlayer.get());

        if (itemId == 383 && SpawnEggItem::fjGetType(item) == GetType())
            return 0x4c36cd96U;
        if (isLeashedToPlayer(player))
            return 0x551269b5U;
        if (itemId == 420)
        {
            if (!isLeashed())
                return 0xa1b34a96U;
        }
        else if (itemId == 421)
        {
            if (!player->isSneaking() || !isTamed())
            {
                if (!item->hasCustomHoverName())
                    return 0xffffffffU;
                if (hasCustomName() && item->getHoverName() == getCustomName())
                    return 0xffffffffU;
                return 0x5592c692U;
            }
        }

        if (isTamed())
        {
            if (player->isSneaking()
                || itemId == 329 && !isSaddled()
                || wearsArmor() && horseArmor)
                return 0x41590ad1U;
            if (getPassengers()->empty())
                return 0x497adcf0U;
        }
    }
    return 0xffffffffU;
}

bool ZombieHorse::breathesWater()
{
    return true;
}

unsigned long long ZombieHorse::GetType()
{
    return 0x000000002D690003ULL;
}
