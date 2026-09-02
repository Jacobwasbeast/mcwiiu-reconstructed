#include "game/entity/ai/NoSpectatorsSelector.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"

bool NoSpectatorsSelector::apply(boost::shared_ptr<Entity> entity) const
{
    if ((entity->GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER)
    {
        return true;
    }

    return !boost::static_pointer_cast<Player>(entity)->isSpectator();
}

NoSpectatorsSelector::~NoSpectatorsSelector()
{
}
