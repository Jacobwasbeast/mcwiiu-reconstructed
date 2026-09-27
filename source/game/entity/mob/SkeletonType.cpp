#include "game/entity/mob/SkeletonType.hpp"
#include "game/core/StringIds.hpp"
#include "game/client/audio/SoundEvents.hpp"

namespace
{
    SkeletonType* skeletonTypeById(int id)
    {
        static ResourceLocation skeletonLootTable;
        static ResourceLocation witherSkeletonLootTable;
        static ResourceLocation strayLootTable;
        static SkeletonType skeleton(0, IDS_SKELETON, &skeletonLootTable);
        static SkeletonType witherSkeleton(1, (int)IDS_WITHER, &witherSkeletonLootTable);
        static SkeletonType stray(2, IDS_SKELETON, &strayLootTable);

        switch (id)
        {
        case 0:
            return &skeleton;
        case 1:
            return &witherSkeleton;
        case 2:
            return &stray;
        default:
            return NULL;
        }
    }
}

SkeletonType::SkeletonType(int id, int name, ResourceLocation* defaultLootTable)
    : id(id),
      name(name),
      defaultLootTable(defaultLootTable)
{
}

int SkeletonType::getId() const
{
    return id;
}

SkeletonType* SkeletonType::byId(int id)
{
    return skeletonTypeById(id);
}

int SkeletonType::getName() const
{
    return name;
}

ResourceLocation* SkeletonType::getDefaultLootTable() const
{
    return defaultLootTable;
}

const SoundEvent* SkeletonType::getAmbientSound() const
{
    if (id == 1)
        return SoundEvents::WITHER_SKELETON_AMBIENT;
    if (id == 2)
        return SoundEvents::STRAY_AMBIENT;
    return SoundEvents::SKELETON_AMBIENT;
}

const SoundEvent* SkeletonType::getHurtSound() const
{
    if (id == 1)
        return SoundEvents::WITHER_SKELETON_HURT;
    if (id == 2)
        return SoundEvents::STRAY_HURT;
    return SoundEvents::SKELETON_HURT;
}

const SoundEvent* SkeletonType::getDeathSound() const
{
    if (id == 1)
        return SoundEvents::WITHER_SKELETON_DEATH;
    if (id == 2)
        return SoundEvents::STRAY_DEATH;
    return SoundEvents::SKELETON_DEATH;
}

const SoundEvent* SkeletonType::getStepSound() const
{
    if (id == 1)
        return SoundEvents::WITHER_SKELETON_STEP;
    if (id == 2)
        return SoundEvents::STRAY_STEP;
    return SoundEvents::SKELETON_STEP;
}
