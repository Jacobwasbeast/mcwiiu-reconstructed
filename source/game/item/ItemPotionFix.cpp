#include "game/item/ItemPotionFix.hpp"
#include <string>
#include "game/data/CompoundTag.hpp"
#include "game/item/Item.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
namespace { std::wstring potionNames[128]; }
ItemPotionFix::~ItemPotionFix() {}
int ItemPotionFix::getVersion() { return 102; }
CompoundTag* ItemPotionFix::fix(CompoundTag* tag)
{
    if (CheckID(tag, L"minecraft:potion"))
    {
        CompoundTag* itemTag = tag->getCompound(L"tag");
        short damage = tag->getShort(L"Damage");
        if (!itemTag->contains(L"Potion", 8))
        {
            itemTag->putString(L"Potion", potionNames[damage & 0x7f]);
            tag->put(L"tag", itemTag);
            if ((damage & 0x4000) != 0)
                tag->putString(L"id", L"minecraft:splash_potion");
        }
        if (damage != 0)
            tag->putShort(L"Damage", 0);
    }
    return tag;
}
void ItemPotionFix::fix(not_null_ptr<ItemInstance> item)
{
    Item* itemType = item->getItem();
    if (itemType != Item::byId(373) && itemType != Item::byId(438) && itemType != Item::byId(441) && itemType != Item::byId(440))
        return;
    int auxValue = item->getAuxValue();
    PotionUtils::setPotion(item, Potion::byName(potionNames[auxValue & 0x7f]));
    if (auxValue != 0)
        item->setAuxValue(0);
}
uint64_t ItemPotionFix::staticCtor()
{
    const wchar_t* names[128] = {
L"minecraft:water",L"minecraft:regeneration",L"minecraft:swiftness",L"minecraft:fire_resistance",L"minecraft:poison",L"minecraft:healing",L"minecraft:night_vision",L"",L"minecraft:weakness",L"minecraft:strength",L"minecraft:slowness",L"minecraft:leaping",L"minecraft:harming",L"minecraft:water_breathing",L"minecraft:invisibility",L"",
L"minecraft:awkward",L"minecraft:regeneration",L"minecraft:swiftness",L"minecraft:fire_resistance",L"minecraft:poison",L"minecraft:healing",L"minecraft:night_vision",L"",L"minecraft:weakness",L"minecraft:strength",L"minecraft:slowness",L"minecraft:leaping",L"minecraft:harming",L"minecraft:water_breathing",L"minecraft:invisibility",L"",
L"minecraft:thick",L"minecraft:strong_regeneration",L"minecraft:strong_swiftness",L"minecraft:fire_resistance",L"minecraft:strong_poison",L"minecraft:strong_healing",L"minecraft:night_vision",L"",L"minecraft:weakness",L"minecraft:strong_strength",L"minecraft:slowness",L"minecraft:strong_leaping",L"minecraft:strong_harming",L"minecraft:water_breathing",L"minecraft:invisibility",L"",
L"minecraft:strong_regeneration",L"minecraft:strong_swiftness",L"minecraft:fire_resistance",L"minecraft:strong_poison",L"minecraft:strong_healing",L"minecraft:night_vision",L"",L"minecraft:weakness",L"minecraft:strong_strength",L"minecraft:slowness",L"minecraft:strong_leaping",L"minecraft:strong_harming",L"minecraft:water_breathing",L"minecraft:invisibility",L"",L"",
L"minecraft:mundane",L"minecraft:long_regeneration",L"minecraft:long_swiftness",L"minecraft:long_fire_resistance",L"minecraft:long_poison",L"minecraft:healing",L"minecraft:long_night_vision",L"",L"minecraft:long_weakness",L"minecraft:long_strength",L"minecraft:long_slowness",L"minecraft:long_leaping",L"minecraft:harming",L"minecraft:long_water_breathing",L"minecraft:long_invisibility",L"",
L"minecraft:awkward",L"minecraft:long_regeneration",L"minecraft:long_swiftness",L"minecraft:long_fire_resistance",L"minecraft:long_poison",L"minecraft:healing",L"minecraft:long_night_vision",L"",L"minecraft:long_weakness",L"minecraft:long_strength",L"minecraft:long_slowness",L"minecraft:long_leaping",L"minecraft:harming",L"minecraft:long_water_breathing",L"minecraft:long_invisibility",L"",
L"minecraft:thick",L"minecraft:regeneration",L"minecraft:swiftness",L"minecraft:long_fire_resistance",L"minecraft:poison",L"minecraft:strong_healing",L"minecraft:long_night_vision",L"",L"minecraft:long_weakness",L"minecraft:strength",L"minecraft:long_slowness",L"minecraft:leaping",L"minecraft:strong_harming",L"minecraft:long_water_breathing",L"minecraft:long_invisibility",L"",
L"minecraft:regeneration",L"minecraft:swiftness",L"minecraft:long_fire_resistance",L"minecraft:poison",L"minecraft:strong_healing",L"minecraft:long_night_vision",L"",L"minecraft:long_weakness",L"minecraft:strength",L"minecraft:long_slowness",L"minecraft:leaping",L"minecraft:strong_harming",L"minecraft:long_water_breathing",L"minecraft:long_invisibility",L"",L""};
    for (int i = 0; i < 128; ++i) potionNames[i] = names[i];
    return 0;
}
