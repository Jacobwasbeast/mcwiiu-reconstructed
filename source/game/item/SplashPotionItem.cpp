#include "game/item/SplashPotionItem.hpp"

#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"

SplashPotionItem::~SplashPotionItem()
{
}

unsigned int SplashPotionItem::getPotionType() const
{
    return 1;
}

std::wstring SplashPotionItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    Potion* potion = PotionUtils::getPotion(itemInstance);
    return potion->getName(Potion::SPLASH);
}

void SplashPotionItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance,
                                       boost::shared_ptr<Player> player,
                                       std::vector<HtmlString>* lines,
                                       bool)
{
    PotionUtils::addPotionTooltip(itemInstance, lines, 0.75f, player);
}

InteractionResultHolder SplashPotionItem::use(Level*,
                                              boost::shared_ptr<Player>,
                                              InteractionHand::EInteractionHand)
{
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success,
                                   not_null_ptr<ItemInstance>());
}

unsigned int SplashPotionItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail)
    {
        return 0xffffffffu;
    }
    return 0x3c0788a3u;
}

int SplashPotionItem::getUseDuration(not_null_ptr<ItemInstance>)
{
    return 0;
}

std::wstring SplashPotionItem::getBottleTextureName()
{
    return L"potion_splash";
}
