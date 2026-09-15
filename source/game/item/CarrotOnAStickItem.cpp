#include "game/item/CarrotOnAStickItem.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GenericStats.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/core/Pig.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"

CarrotOnAStickItem::CarrotOnAStickItem()
    : Item()
{
    setMaxStackSize(1);
    setMaxDamage(0x19);
}

CarrotOnAStickItem::~CarrotOnAStickItem()
{
}

bool CarrotOnAStickItem::isHandEquipped()
{
    return true;
}

bool CarrotOnAStickItem::isMirroredArt()
{
    return true;
}

bool CarrotOnAStickItem::TestUse(Level*, boost::shared_ptr<Player> player,
                                  InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    boost::shared_ptr<Pig> pig = player->isPassenger()
        ? boost::dynamic_pointer_cast<Pig>(player->getVehicle())
        : boost::shared_ptr<Pig>();

    if (pig)
    {
        if (itemInstance->getMaxDamage() - itemInstance->getAuxValue() > 6 && pig->testboost())
        {
            return true;
        }
    }

    return false;
}

InteractionResultHolder CarrotOnAStickItem::use(Level* level,
                                                boost::shared_ptr<Player> player,
                                                InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (level->mIsClientSide)
    {
        return InteractionResultHolder(
            TestUse(level, player, hand)
                ? InteractionResult::eInteractionResult_Success
                : InteractionResult::eInteractionResult_Pass,
            itemInstance);
    }

    boost::shared_ptr<Pig> pig = player->isPassenger()
        ? boost::dynamic_pointer_cast<Pig>(player->getVehicle())
        : boost::shared_ptr<Pig>();

    if (pig)
    {
        if (itemInstance->getMaxDamage() - itemInstance->getAuxValue() > 6 && pig->boost())
        {
            itemInstance->hurtAndBreak(7, player);

            if (!itemInstance->isEmpty())
            {
                return InteractionResultHolder(InteractionResult::eInteractionResult_Success,
                                               itemInstance);
            }

            not_null_ptr<ItemInstance> replacement(new ItemInstance(Items::fishingRod));
            replacement->setTag(itemInstance->getTag());
            return InteractionResultHolder(InteractionResult::eInteractionResult_Success,
                                           replacement);
        }
    }

    player->awardStat(gStats->get_itemsUsed(VALIDgetId(this)),
                      GenericStats::param_itemsUsed(player, itemInstance));
    return InteractionResultHolder(InteractionResult::eInteractionResult_Pass,
                                   itemInstance);
}

unsigned int CarrotOnAStickItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Success)
    {
        return 0xffffffffu;
    }
    return 0x39cf7c20u;
}
