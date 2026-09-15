#include "game/item/GoldenAppleItem.hpp"

#include "game/core/GenericStats.hpp"
#include "game/core/Rarity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

GoldenAppleItem::GoldenAppleItem(int nutrition, float saturationModifier, bool isMeat)
    : FoodItem(nutrition, saturationModifier, isMeat)
{
    setStackedByData(true);
}

GoldenAppleItem::~GoldenAppleItem()
{
}

bool GoldenAppleItem::isFoil(not_null_ptr<ItemInstance> itemInstance)
{
    const bool enchanted = Item::isFoil(itemInstance);
    const int auxValue = itemInstance->getAuxValue();
    return enchanted || auxValue > 0;
}

const Rarity* GoldenAppleItem::getRarity(not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->getAuxValue() == 0)
    {
        return Rarity::RARE;
    }
    return Rarity::EPIC;
}

int GoldenAppleItem::getUseDescriptionId(int auxValue)
{
    if (auxValue != 0)
    {
        return (int)0x5cf7965c;
    }
    return (int)0x00cd42e1;
}

int GoldenAppleItem::getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return getUseDescriptionId(itemInstance->getAuxValue());
}

void GoldenAppleItem::addEatEffect(not_null_ptr<ItemInstance> itemInstance,
                                   Level* level,
                                   boost::shared_ptr<Player> player)
{
    if (!level->mIsClientSide)
    {
        if (itemInstance->getAuxValue() < 1)
        {
            player->addEffect(new MobEffectInstance(MobEffects::REGENERATION, 100, 1),
                              boost::shared_ptr<Entity>());
            player->addEffect(new MobEffectInstance(MobEffects::ABSORPTION, 2400, 0),
                              boost::shared_ptr<Entity>());
        }
        else
        {
            player->awardStat(GenericStats::overpowered(), GenericStats::param_overpowered());
            player->addEffect(new MobEffectInstance(MobEffects::REGENERATION, 600, 4),
                              boost::shared_ptr<Entity>());
            player->addEffect(new MobEffectInstance(MobEffects::DAMAGE_RESISTANCE, 6000, 0),
                              boost::shared_ptr<Entity>());
            player->addEffect(new MobEffectInstance(MobEffects::FIRE_RESISTANCE, 6000, 0),
                              boost::shared_ptr<Entity>());
            player->addEffect(new MobEffectInstance(MobEffects::ABSORPTION, 2400, 3),
                              boost::shared_ptr<Entity>());
        }
    }
}
