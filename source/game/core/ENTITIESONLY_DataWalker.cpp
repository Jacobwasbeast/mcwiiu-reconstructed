#include "game/core/ENTITIESONLY_DataWalker.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/ListTag.hpp"

ENTITIESONLY_DataWalker::~ENTITIESONLY_DataWalker()
{
}

CompoundTag* ENTITIESONLY_DataWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (tag->contains(std::wstring(L"Entities"), 9))
    {
        ListTag* entities = tag->getList(std::wstring(L"Entities"), 10);

        for (int entityIndex = 0; entityIndex < entities->size(); entityIndex++)
        {
            CompoundTag* entity = (CompoundTag*)entities->get(entityIndex);
            if (entity->contains(std::wstring(L"Passengers"), 9))
            {
                ListTag* passengers = entity->getList(std::wstring(L"Passengers"), 10);

                for (int passengerIndex = 0; passengerIndex < passengers->size(); passengerIndex++)
                {
                    passengers->set(passengerIndex,
                        fixer->update(DataFixTypes::eDataFixTypes_Entity,
                            passengers->getCompound(passengerIndex), version));
                }
            }

            entities->set(entityIndex,
                fixer->update(DataFixTypes::eDataFixTypes_Entity, entity, version));
        }
    }

    if (tag->contains(std::wstring(L"TileEntities"), 9))
    {
        ListTag* tileEntities = tag->getList(std::wstring(L"TileEntities"), 10);

        for (int tileEntityIndex = 0; tileEntityIndex < tileEntities->size(); tileEntityIndex++)
        {
            tileEntities->set(tileEntityIndex,
                fixer->update(DataFixTypes::eDataFixTypes_BlockEntity,
                    (CompoundTag*)tileEntities->get(tileEntityIndex), version));
        }
    }

    return tag;
}
