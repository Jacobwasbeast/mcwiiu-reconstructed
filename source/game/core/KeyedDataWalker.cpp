#include "game/core/KeyedDataWalker.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/world/block/BlockEntity.hpp"

KeyedDataWalker::KeyedDataWalker(unsigned long long type)
    : mKey()
{
    if ((type & 0x08000000ULL) != 0)
    {
        mKey = EntityIO::getKey(type);
    }
    else if ((type & 0x01000200ULL) == 0x01000200ULL)
    {
        mKey = BlockEntity::getKey(type);
    }
    else
    {
        mKey = ResourceLocation(std::wstring(L""));
    }
}

KeyedDataWalker::~KeyedDataWalker()
{
}

CompoundTag* KeyedDataWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    ResourceLocation location(tag->getString(std::wstring(L"id")));

    if (location.equals(&mKey))
    {
        return doWalk(fixer, tag, version);
    }

    return tag;
}
