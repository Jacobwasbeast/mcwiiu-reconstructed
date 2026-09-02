#include "game/entity/mob/SkeletonType.hpp"

SkeletonType::SkeletonType(int id, int name, ResourceLocation* defaultLootTable)
    : id(id),
      name(name),
      defaultLootTable(defaultLootTable)
{
}
