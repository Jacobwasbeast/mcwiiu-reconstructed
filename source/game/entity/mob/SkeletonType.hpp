#ifndef GAME_ENTITY_MOB_SKELETONTYPE_HPP
#define GAME_ENTITY_MOB_SKELETONTYPE_HPP

#include "game/client/audio/SoundEvent.hpp"
#include "game/util/ResourceLocation.hpp"

class SkeletonType
{
public:
    SkeletonType(int id, int name, ResourceLocation* defaultLootTable);

    int getId() const;
    int getName() const;

    ResourceLocation* getDefaultLootTable() const;

    const SoundEvent* getAmbientSound() const;
    const SoundEvent* getHurtSound() const;
    const SoundEvent* getDeathSound() const;
    const SoundEvent* getStepSound() const;

    static SkeletonType* byId(int id);

private:
    int id;
    int name;
    ResourceLocation* defaultLootTable;
};

#endif
