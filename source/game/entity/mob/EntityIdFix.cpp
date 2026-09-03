#include "game/entity/mob/EntityIdFix.hpp"

#include <string>

#include <boost/unordered_map.hpp>

#include "game/data/CompoundTag.hpp"

static boost::unordered_map<std::wstring, std::wstring> sEntityIdMap;

EntityIdFix::~EntityIdFix()
{
}

int EntityIdFix::getVersion()
{
    return 704;
}

CompoundTag* EntityIdFix::fix(CompoundTag* tag)
{
    boost::unordered_map<std::wstring, std::wstring>::const_iterator entry = sEntityIdMap.find(tag->getString(L"id"));

    if (entry != sEntityIdMap.end())
    {
        tag->putString(L"id", entry->second);
    }

    return tag;
}
