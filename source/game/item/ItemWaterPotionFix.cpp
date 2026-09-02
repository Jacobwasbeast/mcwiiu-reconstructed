#include "game/item/ItemWaterPotionFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"

ItemWaterPotionFix::~ItemWaterPotionFix()
{
}

int ItemWaterPotionFix::getVersion()
{
    return 806;
}

CompoundTag* ItemWaterPotionFix::fix(CompoundTag* tag)
{
    std::wstring id = tag->getString(L"id");

    if (id == L"minecraft_potion" || id == L"minecraft_splash_potion" ||
        id == L"minecraft_lingering_potion" || id == L"minecraft_tipped_arrow")
    {
        CompoundTag* itemTag = tag->getCompound(L"tag");

        if (!itemTag->contains(L"Potion", 8))
        {
            itemTag->putString(L"Potion", L"minecraft_water");
        }

        if (!tag->contains(L"tag", 10))
        {
            tag->put(L"tag", itemTag);
        }
    }

    return tag;
}
