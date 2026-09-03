#include "game/world/generation/EntityZombieVillagerTypeFix.hpp"

#include "game/data/CompoundTag.hpp"

EntityZombieVillagerTypeFix::~EntityZombieVillagerTypeFix()
{
}

int EntityZombieVillagerTypeFix::getVersion()
{
    return 502;
}

CompoundTag* EntityZombieVillagerTypeFix::fix(CompoundTag* tag)
{
    return tag;
}

int EntityZombieVillagerTypeFix::getVillagerProfession(int profession)
{
    return profession >= 0 && profession < 6 ? profession : -1;
}
