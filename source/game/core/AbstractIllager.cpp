#include "game/core/AbstractIllager.hpp"

#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"

static const unsigned long long ENTITY_TYPE_ABSTRACT_ILLAGER = 0x000000004D541040ULL;

const EntityDataAccessor<unsigned char>* AbstractIllager::DATA_ILLAGER_FLAGS = NULL;

AbstractIllager::AbstractIllager(Level* level)
    : Monster(level)
{
}

AbstractIllager::~AbstractIllager()
{
}

void AbstractIllager::staticCtor()
{
    DATA_ILLAGER_FLAGS = SynchedEntityData::defineId(ENTITY_TYPE_ABSTRACT_ILLAGER,
                                                     ENTITY_TYPE_MONSTER,
                                                     EntityDataSerializers::BYTE);
}

void AbstractIllager::defineSynchedData()
{
    Mob::defineSynchedData();
    getEntityData()->createDateItem(DATA_ILLAGER_FLAGS, (unsigned char)0);
}

bool AbstractIllager::getIllagerFlag(int flag)
{
    return (getEntityData()->get(DATA_ILLAGER_FLAGS) & flag) != 0;
}

void AbstractIllager::setIllagerFlag(int flag, bool value)
{
    unsigned char flags = getEntityData()->get(DATA_ILLAGER_FLAGS);
    if (value)
        flags = (unsigned char)(flags | flag);
    else
        flags = (unsigned char)(flags & ~flag);
    getEntityData()->set(DATA_ILLAGER_FLAGS, flags, true);
}

EMobType AbstractIllager::getMobType()
{
    return MOB_TYPE_ILLAGER;
}

int AbstractIllager::getArmPose()
{
    return ILLAGER_ARM_POSE_CROSSED;
}
