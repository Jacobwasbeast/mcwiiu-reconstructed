#include "game/item/ItemTotemRenameFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"

ItemTotemRenameFix::~ItemTotemRenameFix()
{
}

int ItemTotemRenameFix::getVersion()
{
    return 820;
}

CompoundTag* ItemTotemRenameFix::fix(CompoundTag* tag)
{
    if (tag->getString(L"id") == L"minecraft:totem")
    {
        tag->putString(L"id", L"minecraft:totem_of_undying");
    }

    return tag;
}
