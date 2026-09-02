#include "game/item/ItemBannerColorFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"

ItemBannerColorFix::~ItemBannerColorFix()
{
}

int ItemBannerColorFix::getVersion()
{
    return 804;
}

CompoundTag* ItemBannerColorFix::fix(CompoundTag* tag)
{
    if (tag->getString(L"id") == L"minecraft:banner" && tag->contains(L"tag", 10))
    {
        CompoundTag* itemTag = tag->getCompound(L"tag");

        if (itemTag->contains(L"BlockEntityTag", 10))
        {
            CompoundTag* blockEntityTag = itemTag->getCompound(L"BlockEntityTag");

            if (blockEntityTag->contains(L"Base", 99))
            {
                tag->putShort(L"Damage", (short)(blockEntityTag->getShort(L"Base") & 0xF));

                if (itemTag->contains(L"display", 10))
                {
                    CompoundTag* displayTag = itemTag->getCompound(L"display");

                    if (displayTag->contains(L"Lore", 9))
                    {
                        ListTag* loreTag = displayTag->getList(L"Lore", 8);

                        if (loreTag->size() == 1 && loreTag->getString(0) == L"(+NBT)")
                        {
                            return tag;
                        }
                    }
                }

                blockEntityTag->remove(L"Base");

                if (blockEntityTag->isEmpty())
                {
                    itemTag->remove(L"BlockEntityTag");
                }

                if (itemTag->isEmpty())
                {
                    tag->remove(L"tag");
                }
            }
        }
    }

    return tag;
}
