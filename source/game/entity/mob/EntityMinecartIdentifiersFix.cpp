#include "game/entity/mob/EntityMinecartIdentifiersFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"

static const std::wstring MINECART_IDS[] =
{
    L"MinecartRideable",
    L"MinecartChest",
    L"MinecartFurnace",
    L"MinecartTNT",
    L"MinecartSpawner",
    L"MinecartHopper",
    L"MinecartCommandBlock"
};

EntityMinecartIdentifiersFix::~EntityMinecartIdentifiersFix()
{
}

int EntityMinecartIdentifiersFix::getVersion()
{
    return 106;
}

CompoundTag* EntityMinecartIdentifiersFix::fix(CompoundTag* tag)
{
    if (!CheckID(tag, L"Minecart"))
    {
        return tag;
    }

    std::wstring id = L"MinecartRideable";

    int type = tag->getInt(L"Type");

    if (type >= 0 && type < static_cast<int>(sizeof(MINECART_IDS) / sizeof(MINECART_IDS[0])))
    {
        id = MINECART_IDS[type];
    }

    tag->putString(L"id", id);

    tag->remove(L"Type");

    return tag;
}
