#include "game/core/Mule.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/generation/Player.hpp"

Mule::Mule(Level* level)
    : AbstractChestedHorse(level)
{
    fjDerivedCtorCalls();
}

Mule::~Mule()
{
}

void Mule::create(Level* level)
{
    new Mule(level);
}

void Mule::addDataWalkers(DataFixerUpper* fixer)
{
    AbstractChestedHorse::addDataWalkers(fixer, 0x2d698002ULL);
}

ResourceLocation* Mule::getDefaultLootTable()
{
    return &BuiltInLootTables::MULE;
}

const SoundEvent* Mule::getAmbientSound()
{
    AbstractHorse::getAmbientSound();
    return SoundEvents::MULE_AMBIENT;
}

const SoundEvent* Mule::getDeathSound()
{
    AbstractHorse::getDeathSound();
    return SoundEvents::MULE_DEATH;
}

const SoundEvent* Mule::getHurtSound(const DamageSource* source)
{
    AbstractHorse::getHurtSound(source);
    return SoundEvents::MULE_HURT;
}

void Mule::playChestEquipsSound()
{
    playSound(SoundEvents::DONKEY_CHEST, 1.0f,
              1.0f + (getRandom()->nextFloat() - getRandom()->nextFloat()) * 0.2f);
}

unsigned int Mule::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bAllowedToInteract)
        return 0xffffffffU;

    not_null_ptr<ItemInstance> item = tooltipData.m_pItemInstance;
    unsigned int itemId = Item::getId(item->getItem());
    bool food = itemId == 170 || itemId == 260 || itemId == 296
             || itemId == 322 || itemId == 353 || itemId == 396;
    boost::shared_ptr<Player> player(tooltipData.m_pPlayer,
                                     (Player*)tooltipData.m_pPlayer.get());

    if (itemId == 383)
    {
        if (SpawnEggItem::fjGetType(item) != GetType() && isLeashedToPlayer(player))
            return 0x551269b5U;
        return 0xffffffffU;
    }

    if (isLeashedToPlayer(player))
        return 0x551269b5U;

    if (itemId == 420 && !isLeashed())
        return 0xa1b34a96U;

    if (itemId == 421 && (!player->isSneaking() || !isTamed()))
    {
        if (!item->hasCustomHoverName())
            return 0xffffffffU;
        if (hasCustomName() && item->getHoverName() == getCustomName())
            return 0xffffffffU;
        return 0x5592c692U;
    }

    if (isBaby())
        return food ? 0x82f8d90fU : 0xffffffffU;

    if (!isTamed())
    {
        if (food)
            return 0x82f8d90fU;
        if (item->isEmpty() && getPassengers()->empty())
            return 0x95cfa728U;
        return 0xffffffffU;
    }

    if (player->isSneaking())
        return 0x41590ad1U;
    if (itemId == 54 && !hasChest())
        return 0x7b938f36U;
    if (itemId == 329 && !isSaddled())
        return 0x41590ad1U;
    if (food && itemId != 170)
        return 0x82f8d90fU;
    if (getPassengers()->empty())
        return 0x497adcf0U;

    return 0xffffffffU;
}

bool Mule::HideUseTooltips()
{
    return true;
}

unsigned long long Mule::GetType()
{
    return 0x000000002D698002ULL;
}
