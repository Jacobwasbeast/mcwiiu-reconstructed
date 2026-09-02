#include "game/entity/ai/PushableBySelector.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

PushableBySelector::PushableBySelector(const boost::shared_ptr<Entity>& pusher)
    : pusher(pusher), team(pusher->getTeam())
{
}

bool PushableBySelector::apply(boost::shared_ptr<Entity> entity) const
{
    if (!entity->isPushable(pusher))
    {
        return false;
    }

    if (pusher->mLevel->mIsClientSide
        && !((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER
            && boost::static_pointer_cast<Player>(entity)->isLocalPlayer()))
    {
        return false;
    }

    if (pusher->mLevel->mIsClientSide
        && pusher->GetType() == ENTITY_TYPE_LOCAL_PLAYER
        && entity->GetType() == ENTITY_TYPE_LOCAL_PLAYER)
    {
        return false;
    }

    return true;
}

PushableBySelector::~PushableBySelector()
{
}
