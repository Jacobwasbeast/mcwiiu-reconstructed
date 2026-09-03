#include "game/entity/mob/EntityZombieSplitFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/Tag.hpp"

EntityZombieSplitFix::~EntityZombieSplitFix()
{
}

int EntityZombieSplitFix::getVersion()
{
    return 702;
}

CompoundTag* EntityZombieSplitFix::fix(CompoundTag* tag)
{
    if (!CheckID(tag, L"Zombie"))
    {
        return tag;
    }

    unsigned int zombieType = (unsigned int)tag->getInt(L"ZombieType");

    if (zombieType >= 1 && zombieType <= 5)
    {
        tag->putString(L"id", L"ZombieVillager");
        tag->putInt(L"Profession", zombieType - 1);
    }
    else if (zombieType == 6)
    {
        tag->putString(L"id", L"Husk");
    }

    Tag* zombieTypeTag = tag->get(L"ZombieType");
    delete zombieTypeTag;

    tag->remove(L"ZombieType");

    return tag;
}
