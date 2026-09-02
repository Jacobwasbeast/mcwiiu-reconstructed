#include "game/world/block/BlockEntityMobSpawnerEntityIdentifiersFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"

BlockEntityMobSpawnerEntityIdentifiersFix::~BlockEntityMobSpawnerEntityIdentifiersFix()
{
}

int BlockEntityMobSpawnerEntityIdentifiersFix::getVersion()
{
    return 107;
}

CompoundTag* BlockEntityMobSpawnerEntityIdentifiersFix::fix(CompoundTag* tag)
{
    if (CheckID(tag, L"MobSpawner"))
    {
        if (tag->contains(L"EntityId", 8))
        {
            std::wstring entityId = tag->getString(L"EntityId");
            CompoundTag* spawnData = tag->getCompound(L"SpawnData");

            spawnData->putString(L"id", entityId.empty() ? std::wstring(L"Pig") : entityId);

            tag->put(L"SpawnData", spawnData);
            tag->remove(L"EntityId");
        }

        if (tag->contains(L"SpawnPotentials", 9))
        {
            ListTag* spawnPotentials = tag->getList(L"SpawnPotentials", 10);

            for (int i = 0; i < spawnPotentials->size(); i++)
            {
                CompoundTag* spawnPotential = spawnPotentials->getCompound(i);

                if (spawnPotential->contains(L"Type", 8))
                {
                    CompoundTag* properties = spawnPotential->getCompound(L"Properties");

                    properties->putString(L"id", spawnPotential->getString(L"Type"));

                    spawnPotential->put(L"Entity", properties);
                    spawnPotential->remove(L"Type");
                    spawnPotential->remove(L"Properties");
                }
            }
        }
    }

    return tag;
}
