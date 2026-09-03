#include "game/world/LEVELDATA_DataWalker.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/ListTag.hpp"

LEVELDATA_DataWalker::~LEVELDATA_DataWalker()
{
}

CompoundTag* LEVELDATA_DataWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (tag->contains(L"Level", 10))
    {
        CompoundTag* level = tag->getCompound(L"Level");

        if (level->contains(L"Entities", 9))
        {
            ListTag* entities = level->getList(L"Entities", 10);

            for (int i = 0; i < entities->size(); i++)
            {
                CompoundTag* entity = (CompoundTag*)entities->get(i);

                if (entity->contains(L"Passengers", 9))
                {
                    ListTag* passengers = entity->getList(L"Passengers", 10);

                    for (int j = 0; j < passengers->size(); j++)
                    {
                        passengers->set(j,
                            fixer->update(DataFixTypes::eDataFixTypes_Entity,
                                passengers->getCompound(j), version));
                    }
                }

                entities->set(i,
                    fixer->update(DataFixTypes::eDataFixTypes_Entity, entity, version));
            }
        }

        if (level->contains(L"TileEntities", 9))
        {
            ListTag* tileEntities = level->getList(L"TileEntities", 10);

            for (int i = 0; i < tileEntities->size(); i++)
            {
                tileEntities->set(i,
                    fixer->update(DataFixTypes::eDataFixTypes_BlockEntity,
                        (CompoundTag*)tileEntities->get(i), version));
            }
        }
    }

    return tag;
}
