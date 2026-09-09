#include "game/entity/mob/EntityRedundantChanceTagsFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"

EntityRedundantChanceTagsFix::~EntityRedundantChanceTagsFix()
{
}

int EntityRedundantChanceTagsFix::getVersion()
{
    return 113;
}

CompoundTag* EntityRedundantChanceTagsFix::fix(CompoundTag* tag)
{
    if (tag->contains(L"HandDropChances", 9))
    {
        ListTag* handDropChances = tag->getList(L"HandDropChances", 5);

        if (handDropChances->size() == 2 &&
            handDropChances->getFloat(0) == 0.0f &&
            handDropChances->getFloat(1) == 0.0f)
        {
            tag->remove(L"HandDropChances");
        }
    }

    if (tag->contains(L"ArmorDropChances", 9))
    {
        ListTag* armorDropChances = tag->getList(L"ArmorDropChances", 5);

        if (armorDropChances->size() == 4 &&
            armorDropChances->getFloat(0) == 0.0f &&
            armorDropChances->getFloat(1) == 0.0f &&
            armorDropChances->getFloat(2) == 0.0f &&
            armorDropChances->getFloat(3) == 0.0f)
        {
            tag->remove(L"ArmorDropChances");
        }
    }

    return tag;
}
