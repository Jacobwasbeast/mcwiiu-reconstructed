#include "game/entity/mob/ZombieHorse.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

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
#include "game/item/ItemId.hpp"

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
    AbstractHorse::addDataWalkers(fixer, EntityType::ZOMBIE_HORSE);
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
        bool horseArmor = itemId >= 417 && itemId <= 419 || itemId == ItemId::LEATHER_HORSE_ARMOR;
        boost::shared_ptr<Player> player(tooltipData.m_pPlayer,
                                         (Player*)tooltipData.m_pPlayer.get());

        if (itemId == ItemId::SPAWN_EGG)
        {
            if (SpawnEggItem::fjGetType(item) == EntityType::ZOMBIE_HORSE)
                return IDS_TOOLTIPS_PLACE;
            if (isLeashedToPlayer(player))
                return IDS_TOOLTIPS_UNLEASH;
            return IDS_NONE;
        }
        if (isLeashedToPlayer(player))
            return IDS_TOOLTIPS_UNLEASH;
        if (itemId == ItemId::LEAD)
        {
            if (!isLeashed())
                return IDS_TOOLTIPS_LEASH;
            return IDS_NONE;
        }
        else if (itemId == ItemId::NAME_TAG)
        {
            if (!player->isSneaking() || !isTamed())
            {
                if (!item->hasCustomHoverName())
                    return IDS_NONE;
                if (hasCustomName() && item->getHoverName() == getCustomName())
                    return IDS_NONE;
                return IDS_TOOLTIPS_NAME;
            }
        }

        if (isTamed())
        {
            if (player->isSneaking()
                || itemId == ItemId::SADDLE && !isSaddled()
                || wearsArmor() && horseArmor)
                return IDS_TOOLTIPS_OPEN;
            if (getPassengers()->empty())
                return IDS_TOOLTIPS_MOUNT;
        }
    }
    return IDS_NONE;
}

bool ZombieHorse::breathesWater()
{
    return true;
}

unsigned long long ZombieHorse::GetType()
{
    return EntityType::ZOMBIE_HORSE;
}
