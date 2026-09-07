#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"

bool canSeeSkyFunction(const boost::shared_ptr<Entity>& entity)
{
    if (!entity || !entity->isAlive())
    {
        return false;
    }

    const BlockPos position = entity->getCommandSenderBlockPosition();
    return entity->mLevel->canSeeSky(position);
}
