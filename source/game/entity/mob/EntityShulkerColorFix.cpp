#include "game/entity/mob/EntityShulkerColorFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"

EntityShulkerColorFix::~EntityShulkerColorFix()
{
}

int EntityShulkerColorFix::getVersion()
{
    return 510;
}

CompoundTag* EntityShulkerColorFix::fix(CompoundTag* tag)
{
    if (tag->getString(L"id") == L"minecraft:shulker")
    {
        if (!tag->contains(L"Color", 99))
        {
            tag->putByte(L"Color", 10);
        }
    }

    return tag;
}
