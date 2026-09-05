#include "game/item/PotionItem.hpp"

#include "game/core/GenericStats.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"

PotionItem::PotionItem()
    : Item(), mIcon(NULL), mLayerIcon(NULL)
{
    setMaxStackSize(1);
}

PotionItem::~PotionItem()
{
}

uint32_t PotionItem::getPotionType() const
{
    return 0;
}

boost::shared_ptr<ItemInstance> PotionItem::getDefaultInstance()
{
    boost::shared_ptr<ItemInstance> itemInstance = Item::getDefaultInstance();
    not_null_ptr<ItemInstance> potionItem;
    static_cast<boost::shared_ptr<ItemInstance>&>(potionItem) = itemInstance;
    PotionUtils::setPotion(potionItem, Potion::byId(0));
    return itemInstance;
}

not_null_ptr<ItemInstance> PotionItem::finishUsingItem(not_null_ptr<ItemInstance> itemInstance,
                                                       Level* level,
                                                       boost::shared_ptr<LivingEntity> entity)
{
    if (entity.get() != NULL)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        if (player.get() != NULL)
        {
            player->awardStat(gStats->get_itemsUsed(Item::VALIDgetId(this)),
                              GenericStats::param_itemsUsed(player, itemInstance));
        }
    }

    if (entity.get() != NULL && !entity->isCreative())
    {
        itemInstance->shrink(1);
    }

    std::vector<MobEffectInstance*>* effects = PotionUtils::getMobEffects(itemInstance);
    if (effects != NULL && entity.get() != NULL)
    {
        std::vector<MobEffectInstance*>::iterator it;
        for (it = effects->begin(); it != effects->end(); ++it)
        {
            MobEffectInstance* effect = *it;
            if (effect != NULL)
            {
                entity->addEffect(new MobEffectInstance(effect->getEffect(),
                                                        effect->getDuration(),
                                                        effect->getAmplifier()), entity);
            }
        }
        delete effects;
    }

    return itemInstance;
}

int PotionItem::getUseDuration(not_null_ptr<ItemInstance>)
{
    return 32;
}

int PotionItem::getUseAnimation(not_null_ptr<const ItemInstance>) const
{
    return 2;
}

InteractionResultHolder PotionItem::use(Level*, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    player->startUsingItem(hand);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

bool PotionItem::TestUse(Level*, boost::shared_ptr<Player>, InteractionHand::EInteractionHand)
{
    return true;
}

std::wstring PotionItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    Potion* potion = PotionUtils::getPotion(itemInstance);
    return potion->getName(Potion::NORMAL);
}

void PotionItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance,
                                 boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool)
{
    PotionUtils::addPotionTooltip(itemInstance, lines, 1.0f, player);
}

bool PotionItem::isFoil(not_null_ptr<ItemInstance> itemInstance)
{
    if (Item::isFoil(itemInstance))
    {
        return true;
    }
    return PotionUtils::isFoil(itemInstance);
}

int PotionItem::getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    Potion* potion = PotionUtils::getPotion(itemInstance);
    return potion->getUseDescriptionId(Potion::NORMAL);
}

TextureAtlasSprite* PotionItem::getIcon(int)
{
    return mIcon;
}

TextureAtlasSprite* PotionItem::getLayerIcon(int auxValue, int layer,
                                             not_null_ptr<ItemInstance> itemInstance)
{
    if (layer != 0)
    {
        return Item::getLayerIcon(auxValue, layer, itemInstance);
    }
    return mLayerIcon;
}

bool PotionItem::hasMultipleSpriteLayers()
{
    return true;
}

void PotionItem::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(getBottleTextureName());
    mLayerIcon = iconRegister->registerIcon(std::wstring());
}

unsigned int PotionItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail)
    {
        return 0xffffffff;
    }
    return 0x32374037;
}

std::wstring PotionItem::getBottleTextureName()
{
    return std::wstring();
}
