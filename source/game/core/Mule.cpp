#include "game/core/Mule.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemId.hpp"
#include "game/world/block/BlockId.hpp"

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
    AbstractChestedHorse::addDataWalkers(fixer, EntityType::MULE);
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
        return IDS_NONE;

    not_null_ptr<ItemInstance> item = tooltipData.m_pItemInstance;
    unsigned int itemId = Item::getId(item->getItem());
    bool food = itemId == BlockId::HAY_BLOCK || itemId == ItemId::APPLE || itemId == ItemId::WHEAT
             || itemId == ItemId::GOLDEN_APPLE || itemId == ItemId::SUGAR || itemId == ItemId::GOLDEN_CARROT;
    boost::shared_ptr<Player> player(tooltipData.m_pPlayer,
                                     (Player*)tooltipData.m_pPlayer.get());

    if (itemId == ItemId::SPAWN_EGG)
    {
        if (SpawnEggItem::fjGetType(item) != GetType() && isLeashedToPlayer(player))
            return IDS_TOOLTIPS_UNLEASH;
        return IDS_NONE;
    }

    if (isLeashedToPlayer(player))
        return IDS_TOOLTIPS_UNLEASH;

    if (itemId == ItemId::LEAD && !isLeashed())
        return IDS_TOOLTIPS_LEASH;

    if (itemId == ItemId::NAME_TAG && (!player->isSneaking() || !isTamed()))
    {
        if (!item->hasCustomHoverName())
            return IDS_NONE;
        if (hasCustomName() && item->getHoverName() == getCustomName())
            return IDS_NONE;
        return IDS_TOOLTIPS_NAME;
    }

    if (isBaby())
        return food ? IDS_TOOLTIPS_FEED : IDS_NONE;

    if (!isTamed())
    {
        if (food)
            return IDS_TOOLTIPS_FEED;
        if (item->isEmpty() && getPassengers()->empty())
            return IDS_TOOLTIPS_TAME;
        return IDS_NONE;
    }

    if (player->isSneaking())
        return IDS_TOOLTIPS_OPEN;
    if (itemId == BlockId::CHEST && !hasChest())
        return IDS_TOOLTIPS_ATTACH;
    if (itemId == ItemId::SADDLE && !isSaddled())
        return IDS_TOOLTIPS_OPEN;
    if (food && itemId != BlockId::HAY_BLOCK)
        return IDS_TOOLTIPS_FEED;
    if (getPassengers()->empty())
        return IDS_TOOLTIPS_MOUNT;

    return IDS_NONE;
}

bool Mule::HideUseTooltips()
{
    return true;
}

unsigned long long Mule::GetType()
{
    return EntityType::MULE;
}
