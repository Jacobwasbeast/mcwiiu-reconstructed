#include "game/item/ElytraItem.hpp"

#include "game/core/BehaviorRegistry.hpp"
#include "game/core/IconRegister.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ArmorDispenseItemBehavior.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/ElytraItem_ItemPropertyFunction.hpp"
#include "game/world/block/DispenserBlock.hpp"

class ElytraItemPropertyFunction : public ElytraItem_ItemPropertyFunction
{
public:
    virtual float call(not_null_ptr<ItemInstance> itemInstance,
                       Level*,
                       boost::shared_ptr<LivingEntity>)
    {
        return ElytraItem::isFlyEnabled(itemInstance) ? 0.25f : 1.0f;
    }
};

ElytraItem::ElytraItem()
    : Item()
{
    mMaxStackSize = 1;
    setMaxDamage(0x1B0);
    addProperty(ResourceLocation(L"broken"), new ElytraItemPropertyFunction());
    DispenserBlock::DISPENSER_REGISTRY.add(this, new ArmorDispenseItemBehavior());
}

ElytraItem::~ElytraItem()
{
}

int ElytraItem::GetArmorType()
{
    return 2;
}

bool ElytraItem::isFlyEnabled(not_null_ptr<ItemInstance> itemInstance)
{
    return itemInstance->getDamageValue() < itemInstance->getMaxDamage() - 1;
}

bool ElytraItem::isValidRepairItem(not_null_ptr<ItemInstance>, not_null_ptr<ItemInstance> repairItem)
{
    return repairItem->getItem() == Items::elytra;
}

InteractionResultHolder ElytraItem::use(Level*,
                                        boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInHand = player->getItemInHand(hand);
    const EquipmentSlot* slot = Mob::getEquipmentSlotForItem(itemInHand);
    not_null_ptr<ItemInstance> equippedItem = player->getItemSlot(slot);

    player->setItemSlot(slot, itemInHand->copy());
    itemInHand->setCount(0);

    if (equippedItem->isEmpty())
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInHand);
    }

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, equippedItem);
}

bool ElytraItem::TestUse(Level*, boost::shared_ptr<Player>, InteractionHand::EInteractionHand)
{
    return true;
}

unsigned int ElytraItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail)
    {
        return 0xFFFFFFFF;
    }

    return 0xAFD35F15;
}

void ElytraItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    mBrokenIcon = iconRegister->registerIcon(L"broken_elytra");
}

TextureAtlasSprite* ElytraItem::getLayerIcon(int auxValue, not_null_ptr<ItemInstance>)
{
    if (auxValue < getMaxDamage() - 1)
    {
        return mIcon;
    }

    return mBrokenIcon;
}

TextureAtlasSprite* ElytraItem::getIcon(not_null_ptr<ItemInstance> itemInstance)
{
    return isFlyEnabled(itemInstance) ? mIcon : mBrokenIcon;
}
