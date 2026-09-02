#include "game/entity/ai/NoCreativeOrSpectatorSelector.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"

bool NoCreativeOrSpectatorSelector::apply(boost::shared_ptr<Entity> entity) const
{
    if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER
        && boost::static_pointer_cast<Player>(entity)->isSpectator()
        && boost::static_pointer_cast<Player>(entity)->isCreative())
    {
        return false;
    }

    return true;
}

NoCreativeOrSpectatorSelector::~NoCreativeOrSpectatorSelector()
{
}
