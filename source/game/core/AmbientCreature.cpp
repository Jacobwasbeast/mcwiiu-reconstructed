#include "game/core/AmbientCreature.hpp"

AmbientCreature::AmbientCreature(Level* level)
    : Mob(level)
{
}

AmbientCreature::~AmbientCreature()
{
}

bool AmbientCreature::canBeLeashed(const boost::shared_ptr<Player>&)
{
    return false;
}
