#include "game/item/LingeringPotionItem.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"

LingeringPotionItem::~LingeringPotionItem()
{
}

unsigned int LingeringPotionItem::getPotionType() const
{
    return 2;
}

std::wstring LingeringPotionItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    Potion* potion = PotionUtils::getPotion(itemInstance);
    return potion->getName(Potion::AMPLIFIED);
}

void LingeringPotionItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance,
                                          boost::shared_ptr<Player> player,
                                          std::vector<HtmlString>* lines,
                                          bool)
{
    PotionUtils::addPotionTooltip(itemInstance, lines, 0.25f, player);
}

InteractionResultHolder LingeringPotionItem::use(Level* level,
                                                  boost::shared_ptr<Player> player,
                                                  InteractionHand::EInteractionHand hand)
{
    return Item::use(level, player, hand);
}

unsigned int LingeringPotionItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail)
    {
        return 0xffffffffu;
    }
    return 0x3c0788a3u;
}

std::wstring LingeringPotionItem::getBottleTextureName()
{
    return L"potion_bottle_lingering";
}
