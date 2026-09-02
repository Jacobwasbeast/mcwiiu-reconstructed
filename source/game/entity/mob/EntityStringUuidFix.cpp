#include "game/entity/mob/EntityStringUuidFix.hpp"

#include "game/data/CompoundTag.hpp"

EntityStringUuidFix::~EntityStringUuidFix()
{
}

int EntityStringUuidFix::getVersion()
{
    return 108;
}

CompoundTag* EntityStringUuidFix::fix(CompoundTag* tag)
{
    return tag;
}
