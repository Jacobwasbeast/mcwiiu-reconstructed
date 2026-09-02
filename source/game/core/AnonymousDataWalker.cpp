#include "game/core/AnonymousDataWalker.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/ListTag.hpp"

AnonymousDataWalker::~AnonymousDataWalker()
{
}

CompoundTag* AnonymousDataWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (tag->contains(L"entities", 9))
    {
        ListTag* entitiesTag = tag->getList(L"entities", 10);

        for (int i = 0; i < entitiesTag->size(); ++i)
        {
            CompoundTag* entityTag = (CompoundTag*)entitiesTag->get(i);

            if (entityTag->contains(L"nbt", 10))
            {
                entityTag->put(L"nbt", fixer->update(DataFixTypes::eDataFixTypes_Entity, entityTag->getCompound(L"nbt"), version));
            }
        }
    }

    if (tag->contains(L"blocks", 9))
    {
        ListTag* blocksTag = tag->getList(L"blocks", 10);

        for (int i = 0; i < blocksTag->size(); ++i)
        {
            CompoundTag* blockTag = (CompoundTag*)blocksTag->get(i);

            if (blockTag->contains(L"nbt", 10))
            {
                blockTag->put(L"nbt", fixer->update(DataFixTypes::eDataFixTypes_BlockEntity, blockTag->getCompound(L"nbt"), version));
            }
        }
    }

    return tag;
}
