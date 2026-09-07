#include "game/entity/mob/ShoulderRidingEntity.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/world/generation/Player.hpp"

ShoulderRidingEntity::ShoulderRidingEntity(Level* level)
    : TamableAnimal(level), mShoulderTick(0)
{
}

ShoulderRidingEntity::~ShoulderRidingEntity()
{
}

bool ShoulderRidingEntity::setEntityOnShoulder(boost::shared_ptr<Player> player)
{
    CompoundTag* tag = new CompoundTag();
    tag->putString(L"id", getEncodeId());
    saveWithoutId(tag);

    if (player->setEntityOnShoulder(tag) == 0)
    {
        return false;
    }

    remove();
    return true;
}

void ShoulderRidingEntity::tick()
{
    ++mShoulderTick;
    Mob::tick();
}

bool ShoulderRidingEntity::canSitOnShoulder()
{
    return mShoulderTick > 100;
}
