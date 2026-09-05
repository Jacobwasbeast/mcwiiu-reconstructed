#include "game/core/LootTables.hpp"

#include "game/core/BuiltInLootTables.hpp"

LootTables::LootTables()
{
    tables = BuiltInLootTables::getLootTables();
}

LootTable* LootTables::get(ResourceLocation* location)
{
    boost::unordered_map<ResourceLocation*, LootTable*>::iterator found = tables->find(location);
    if (found != tables->end())
    {
        return found->second;
    }

    return NULL;
}
