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
    AbstractChestedHorse::addDataWalkers(fixer, 0x2d698001ULL);
}

const void* Donkey::getDefaultLootTable()
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
    if (type != 0x2d69800100000000ULL && type != 0x2d69000100000000ULL)
    {
        return false;
    }

    return canParent() && boost::static_pointer_cast<AbstractHorse>(other)->canParent();
}

boost::shared_ptr<AgableMob> Donkey::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    boost::shared_ptr<AgableMob> child;
    if (partner->GetType() == 0x2d69000100000000ULL)
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
    unsigned int itemId = Item::getId(item->getItem());
    bool horseArmor = itemId == 170 || itemId == 260 || itemId == 296 ||
                      itemId == 322 || itemId == 353 || itemId == 396;
    boost::shared_ptr<Player> player(tooltipData.m_pPlayer,
                                     (Player*)tooltipData.m_pPlayer.get());

    if (itemId == 383 && SpawnEggItem::fjGetType(item) == GetType())
    {
        return 0x4c36cd96U;
    }
    if (isLeashedToPlayer(player))
    {
        return 0x551269b5U;
    }
    if (itemId == 420 && !isLeashed())
    {
        return 0xa1b34a96U;
    }
    if (itemId == 421 && (!player->isSneaking() || !isTamed()))
    {
        if (!item->hasCustomHoverName())
        {
            return 0xffffffffU;
        }
        if (hasCustomName() && item->getHoverName() == getCustomName())
        {
            return 0xffffffffU;
        }
        return 0x5592c692U;
    }

    if (isTamed())
    {
        if (player->isSneaking() || itemId == 329 && !isSaddled() || horseArmor)
        {
            return 0x41590ad1U;
        }
        if (getPassengers()->empty())
        {
            return 0x497adcf0U;
        }
    }
    return 0xffffffffU;
}

bool Donkey::HideUseTooltips()
{
    return true;
}

unsigned long long Donkey::GetType()
{
    return 0x2d69800100000000ULL;
}

void Donkey::create(Level* level)
{
    new Donkey(level);
}
