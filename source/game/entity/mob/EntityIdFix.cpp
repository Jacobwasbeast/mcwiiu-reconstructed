#include "game/entity/mob/EntityIdFix.hpp"

#include <string>

#include <boost/unordered_map.hpp>

#include "game/data/CompoundTag.hpp"

const boost::unordered_map<std::wstring, std::wstring> EntityIdFix::ID_MAP;

EntityIdFix::~EntityIdFix()
{
}

int EntityIdFix::getVersion()
{
    return 704;
}

CompoundTag* EntityIdFix::fix(CompoundTag* tag)
{
    boost::unordered_map<std::wstring, std::wstring>::const_iterator entry = ID_MAP.find(tag->getString(L"id"));

    if (entry != ID_MAP.end())
    {
        tag->putString(L"id", entry->second);
    }

    return tag;
}
