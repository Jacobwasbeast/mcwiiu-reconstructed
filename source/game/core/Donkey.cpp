#include "game/core/Donkey.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Mule.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Blocks.hpp"

Donkey::Donkey(Level* level)
    : AbstractChestedHorse(level)
{
    fjDerivedCtorCalls();
}

Donkey::~Donkey()
{
}

void Donkey::addDataWalkers(DataFixerUpper* fixer)
{
    AbstractChestedHorse::addDataWalkers(fixer, ENTITY_TYPE_DONKEY);
}

ResourceLocation* Donkey::getDefaultLootTable()
{
    return &BuiltInLootTables::DONKEY;
}

const SoundEvent* Donkey::getAmbientSound()
{
    AbstractHorse::getAmbientSound();
    return SoundEvents::DONKEY_AMBIENT;
}

const SoundEvent* Donkey::getDeathSound()
{
    AbstractHorse::getDeathSound();
    return SoundEvents::DONKEY_DEATH;
}

const SoundEvent* Donkey::getHurtSound(const DamageSource* source)
{
    AbstractHorse::getHurtSound(source);
    return SoundEvents::DONKEY_HURT;
}

bool Donkey::canMate(const boost::shared_ptr<Animal>& other)
{
    if (other.get() == this)
    {
        return false;
    }

    unsigned long long type = other->GetType();
    if (type != ENTITY_TYPE_DONKEY && type != ENTITY_TYPE_HORSE)
    {
        return false;
    }

    return canParent() && boost::static_pointer_cast<AbstractHorse>(other)->canParent();
}

boost::shared_ptr<AgableMob> Donkey::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    boost::shared_ptr<AgableMob> child;
    if (partner->GetType() == ENTITY_TYPE_HORSE)
    {
        child.reset(new Mule(mLevel));
    }
    else
    {
        child.reset(new Donkey(mLevel));
    }
    setOffspringAttributes(partner, boost::static_pointer_cast<AbstractHorse>(child));
    return child;
}

unsigned int Donkey::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bAllowedToInteract)
    {
        return 0xffffffffU;
    }

    not_null_ptr<ItemInstance> item = tooltipData.m_pItemInstance;
    Item* itemType = item->getItem();
    bool horseArmor = itemType == Items::leatherHorseArmor || itemType == Items::apple ||
                      itemType == Items::wheat || itemType == Items::goldenApple ||
                      itemType == Items::sugar || itemType == Items::goldenCarrot;
    bool loveFoodItem = itemType == Items::goldenApple || itemType == Items::goldenCarrot;
    boost::shared_ptr<Player> player(tooltipData.m_pPlayer,
                                     (Player*)tooltipData.m_pPlayer.get());

    if (itemType == Items::spawnEgg)
    {
        if (SpawnEggItem::fjGetType(item) == GetType())
        {
            return 0x4c36cd96U;
        }
        if (isLeashedToPlayer(player))
        {
            return 0x551269b5U;
        }
        return 0xffffffffU;
    }

    if (isLeashedToPlayer(player))
    {
        return 0x551269b5U;
    }

    if (itemType == Items::lead)
    {
        if (!isLeashed())
        {
            return 0xa1b34a96U;
        }
        if (!getPassengers()->empty() || !isTamed())
        {
            return 0xffffffffU;
        }
        return 0x497adcf0U;
    }

    if (itemType == Items::nameTag && (!player->isSneaking() || !isTamed()))
    {
        if (item->hasCustomHoverName() && (!hasCustomName() || item->getHoverName() != getCustomName()))
        {
            return 0x5592c692U;
        }
        if (isTamed())
        {
            if (getPassengers()->empty())
            {
                return 0x497adcf0U;
            }
        }
        return 0xffffffffU;
    }

    if (isBaby())
    {
        if (horseArmor)
        {
            return 0x82f8d90fU;
        }
        return 0xffffffffU;
    }

    if (!isTamed())
    {
        if (!item->isEmpty())
        {
            if (horseArmor && itemType != Items::leatherHorseArmor)
            {
                return 0x82f8d90fU;
            }
        }
        else if (getPassengers()->empty())
        {
            return 0x95cfa728U;
        }
        return 0xffffffffU;
    }

    bool showMountTooltip = player->isSneaking() && (!isVehicle() || hasPassenger(player));
    if (!showMountTooltip && itemType == Items::saddle && !isSaddled())
    {
        showMountTooltip = true;
    }
    if (showMountTooltip)
    {
        return 0x41590ad1U;
    }

    if (itemType == Item::byBlock(Blocks::chest) && !hasChest())
    {
        return 0x7b938f36U;
    }

    if (loveFoodItem && !isInLove() && getPassengers()->empty() && !isBreedCoolingDown())
    {
        return 0xb47a2ab6U;
    }

    if (horseArmor && getHealth() < getMaxHealth())
    {
        return 0x56b3f98dU;
    }

    if (getPassengers()->empty())
    {
        return 0x497adcf0U;
    }
    return 0xffffffffU;
}

bool Donkey::HideUseTooltips()
{
    return true;
}

unsigned long long Donkey::GetType()
{
    return ENTITY_TYPE_DONKEY;
}

void Donkey::create(Level* level)
{
    new Donkey(level);
}
