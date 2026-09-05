#include "game/world/generation/PlayerSelector.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/core/GameMode.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/Player.hpp"

bool PlayerSelector::apply(boost::shared_ptr<Player> player) const
{
    if (mEntity->distanceToSqr(player) < 2500.0)
    {
        GameMode* gameMode = (GameMode*)boost::static_pointer_cast<ServerPlayer>(player)->mGameMode;
        if (gameMode->isSurvival())
        {
            return true;
        }
    }

    return false;
}

PlayerSelector::~PlayerSelector()
{
}
