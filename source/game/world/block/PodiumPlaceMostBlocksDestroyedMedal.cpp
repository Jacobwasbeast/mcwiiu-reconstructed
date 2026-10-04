#include "game/world/block/PodiumPlaceMostBlocksDestroyedMedal.hpp"

#include "game/core/GameStats.hpp"
#include "game/core/StatsUID.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/generation/PlayerStats.hpp"

PodiumPlaceMostBlocksDestroyedMedal::PodiumPlaceMostBlocksDestroyedMedal(EPodium podium,
    int name, float target, unsigned int weight, GameMedal::EOperator op)
    : GameMedal(name, target, weight, op, false, false), mPodium(podium)
{
}

float PodiumPlaceMostBlocksDestroyedMedal::GetScore(PlayerStats* playerStats) const
{
    GameStats* gameStats = getGameStats();
    PlayerList* playerList = (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers);
    boost::shared_ptr<ServerPlayer> player =
        playerList->getPlayerFromStatsUID(*(StatsUID*)playerStats);
    boost::shared_ptr<ServerPlayer> firstPlace = gameStats->GetFirstPlace();
    boost::shared_ptr<ServerPlayer> secondPlace = gameStats->GetSecondPlace();
    boost::shared_ptr<ServerPlayer> thirdPlace = gameStats->GetThirdPlace();
    int mostBlocksDestroyed = gameStats->GetMostBlocksDestroyed();
    unsigned int secondMostBlocksDestroyed = gameStats->GetSecondMostBlocksDestroyed();
    unsigned int thirdMostBlocksDestroyed = gameStats->GetThirdMostBlocksDestroyed();
    unsigned int score = 0;

    if (player)
    {
        if (mPodium == PODIUM_FIRST)
        {
            if (player.get() == firstPlace.get())
            {
                score = playerStats->getTotalBlocksDestroyed();
            }
        }
        else if (mPodium == PODIUM_SECOND)
        {
            if ((player.get() != firstPlace.get() ||
                 playerStats->getTotalBlocksDestroyed() != mostBlocksDestroyed) &&
                (player.get() == secondPlace.get() || player.get() == firstPlace.get()))
            {
                unsigned int blocksDestroyed = playerStats->getTotalBlocksDestroyed();

                if (secondMostBlocksDestroyed <= blocksDestroyed)
                {
                    score = blocksDestroyed - secondMostBlocksDestroyed;
                }
            }
        }
        else if (mPodium == PODIUM_THIRD)
        {
            bool eligible = true;

            if (player.get() == firstPlace.get() || player.get() == secondPlace.get())
            {
                unsigned int blocksDestroyed = playerStats->getTotalBlocksDestroyed();

                if (secondMostBlocksDestroyed <= blocksDestroyed)
                {
                    eligible = false;
                }
                else if (player.get() != secondPlace.get() && player.get() != firstPlace.get())
                {
                    eligible = player.get() == thirdPlace.get();
                }
            }
            else
            {
                eligible = player.get() == thirdPlace.get();
            }

            if (eligible)
            {
                unsigned int blocksDestroyed = playerStats->getTotalBlocksDestroyed();

                if (thirdMostBlocksDestroyed <= blocksDestroyed)
                {
                    score = blocksDestroyed - thirdMostBlocksDestroyed;
                }
            }
        }
        else
        {
            score = 0;
        }
    }

    return static_cast<double>(static_cast<float>(static_cast<int>(score)));
}
