#include "game/entity/mob/ItemInstanceEntityTagWalker.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"

ItemInstanceEntityTagWalker::~ItemInstanceEntityTagWalker()
{
}

CompoundTag* ItemInstanceEntityTagWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (!tag->contains(L"tag"))
    {
        return tag;
    }

    CompoundTag* itemTag = tag->getCompound(L"tag");
    if (!itemTag->contains(L"EntityTag", 10))
    {
        return tag;
    }

    CompoundTag* entityTag = itemTag->getCompound(L"EntityTag");
    std::wstring entityId;

    if (CheckID(tag, L"minecraft:armor_stand"))
    {
        entityId = (version < 515) ? L"ArmorStand" : L"minecraft:armor_stand";
    }
    else
    {
        if (!CheckID(tag, L"minecraft:spawn_egg"))
        {
            return tag;
        }

        entityId = entityTag->getString(L"id");
    }

    if (entityId.empty())
    {
        fixer->update(DataFixTypes::eDataFixTypes_Entity, entityTag, version);
    }
    else
    {
        bool hadId = entityTag->contains(L"id", 8);

        entityTag->putString(L"id", entityId);
        fixer->update(DataFixTypes::eDataFixTypes_Entity, entityTag, version);

        if (!hadId)
        {
            entityTag->remove(L"id");
        }
    }

    return tag;
}
