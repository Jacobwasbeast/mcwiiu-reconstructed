#include "game/core/UniqueTypeWalker.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/DataFixers.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/EntityIO.hpp"

UniqueTypeWalker::UniqueTypeWalker(const ResourceLocation& key)
    : mKey()
{
    mKey = key;
}

UniqueTypeWalker::~UniqueTypeWalker()
{
}

CompoundTag* UniqueTypeWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (mKey.toString() != tag->getString(L"id"))
    {
        return tag;
    }

    ResourceLocation villagerKey = EntityIO::getKey(0x000000000D600001ULL);
    if (mKey.toString() == villagerKey.toString())
    {
        if (tag->contains(L"Offers", 10))
        {
            CompoundTag* offers = tag->getCompound(L"Offers");
            if (offers->contains(L"Recipes", 9))
            {
                ListTag* recipes = offers->getList(L"Recipes", 10);
                for (int i = 0; i < recipes->size(); ++i)
                {
                    CompoundTag* recipe = recipes->getCompound(i);
                    DataFixers::updateItem(fixer, recipe, version, L"buy");
                    DataFixers::updateItem(fixer, recipe, version, L"buyB");
                    DataFixers::updateItem(fixer, recipe, version, L"sell");
                    recipes->set(i, recipe);
                }
            }
        }
    }

    if (mKey.toString() == L"minecraft_mob_spawner")
    {
        if (tag->contains(L"SpawnPotentials", 9))
        {
            ListTag* potentials = tag->getList(L"SpawnPotentials", 10);
            for (int i = 0; i < potentials->size(); ++i)
            {
                CompoundTag* potential = potentials->getCompound(i);
                potential->put(L"Entity",
                    fixer->update(DataFixTypes::eDataFixTypes_Entity,
                        potential->getCompound(L"Entity"), version));
            }
        }

        tag->put(L"SpawnData",
            fixer->update(DataFixTypes::eDataFixTypes_Entity,
                tag->getCompound(L"SpawnData"), version));
    }

    return tag;
}
