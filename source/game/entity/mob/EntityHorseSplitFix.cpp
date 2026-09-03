#include "game/entity/mob/EntityHorseSplitFix.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/Tag.hpp"

EntityHorseSplitFix::~EntityHorseSplitFix()
{
}

int EntityHorseSplitFix::getVersion()
{
    return 703;
}

CompoundTag* EntityHorseSplitFix::fix(CompoundTag* tag)
{
    if (!CheckID(tag, L"EntityHorse"))
    {
        return tag;
    }

    int horseType = tag->getInt(L"Type");

    switch (horseType)
    {
    case 1:
        tag->putString(L"id", L"Donkey");
        break;

    case 2:
        tag->putString(L"id", L"Mule");
        break;

    case 3:
        tag->putString(L"id", L"ZombieHorse");
        break;

    case 4:
        tag->putString(L"id", L"SkeletonHorse");
        break;

    default:
        tag->putString(L"id", L"Horse");
        break;
    }

    Tag* typeTag = tag->get(L"Type");
    delete typeTag;

    tag->remove(L"Type");

    return tag;
}
