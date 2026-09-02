#include "game/world/LEVELDATA_DataWalker.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/ListTag.hpp"

LEVELDATA_DataWalker::~LEVELDATA_DataWalker()
{
}

CompoundTag* LEVELDATA_DataWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (tag->contains(std::wstring(L"Level"), 10))
    {
        CompoundTag* level = tag->getCompound(std::wstring(L"Level"));

        if (level->contains(std::wstring(L"Entities"), 9))
        {
            ListTag* entities = level->getList(std::wstring(L"Entities"), 10);

            for (int i = 0; i < entities->size(); i++)
            {
                if (((CompoundTag*)entities->get(i))->contains(std::wstring(L"Passengers"), 9))
                {
                    ListTag* passengers =
                        ((CompoundTag*)entities->get(i))->getList(std::wstring(L"Passengers"), 10);

                    for (int j = 0; j < passengers->size(); j++)
                    {
                        passengers->set(j,
                            fixer->update(DataFixTypes::eDataFixTypes_Entity,
                                passengers->getCompound(j), version));
                    }
                }

                entities->set(i,
                    fixer->update(DataFixTypes::eDataFixTypes_Entity,
                        (CompoundTag*)entities->get(i), version));
            }
        }

        if (level->contains(std::wstring(L"TileEntities"), 9))
        {
            ListTag* tileEntities = level->getList(std::wstring(L"TileEntities"), 10);

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
