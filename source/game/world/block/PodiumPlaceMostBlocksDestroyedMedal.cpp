#include "game/world/block/PodiumPlaceMostBlocksDestroyedMedal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/StatsUID.hpp"
#include "game/world/generation/PlayerStats.hpp"

class GameStats;
class PlayerList;
class MinecraftServer;
class ServerPlayer;

extern MinecraftServer* gMinecraftServer;

class GameStats
{
public:
    boost::shared_ptr<ServerPlayer> GetFirstPlace();
    boost::shared_ptr<ServerPlayer> GetSecondPlace();
    boost::shared_ptr<ServerPlayer> GetThirdPlace();
    int GetMostBlocksDestroyed();
    int GetSecondMostBlocksDestroyed();
    int GetThirdMostBlocksDestroyed();
};

class PlayerList
{
public:
    boost::shared_ptr<ServerPlayer> getPlayerFromStatsUID(const StatsUID& statsUID);
};

class MinecraftServer
{
public:
    PlayerList* getPlayers();
};

PodiumPlaceMostBlocksDestroyedMedal::PodiumPlaceMostBlocksDestroyedMedal(EPodium podium,
    int name, float target, unsigned int weight, GameMedal::EOperator op)
    : GameMedal(name, target, weight, op, false, false), mPodium(podium)
{
}

PodiumPlaceMostBlocksDestroyedMedal::~PodiumPlaceMostBlocksDestroyedMedal()
{
}

double PodiumPlaceMostBlocksDestroyedMedal::GetScore(PlayerStats* playerStats) const
{
    GameStats* gameStats = getGameStats();
    boost::shared_ptr<ServerPlayer> player =
        gMinecraftServer->getPlayers()->getPlayerFromStatsUID(*(StatsUID*)playerStats);
    boost::shared_ptr<ServerPlayer> firstPlace = gameStats->GetFirstPlace();
    boost::shared_ptr<ServerPlayer> secondPlace = gameStats->GetSecondPlace();
    boost::shared_ptr<ServerPlayer> thirdPlace = gameStats->GetThirdPlace();
    int mostBlocksDestroyed = gameStats->GetMostBlocksDestroyed();
    int secondMostBlocksDestroyed = gameStats->GetSecondMostBlocksDestroyed();
    int thirdMostBlocksDestroyed = gameStats->GetThirdMostBlocksDestroyed();
    int blocksDestroyed = 0;

    if (player)
    {
        blocksDestroyed = playerStats->getTotalBlocksDestroyed();

        if (mPodium == PODIUM_FIRST)
        {
            if (player.get() == firstPlace.get())
            {
                return static_cast<double>(static_cast<float>(blocksDestroyed));
            }
        }
        else if (mPodium == PODIUM_SECOND)
        {
            if ((player.get() == secondPlace.get() || player.get() == firstPlace.get()) &&
                blocksDestroyed >= secondMostBlocksDestroyed &&
                (player.get() != firstPlace.get() || blocksDestroyed != mostBlocksDestroyed))
            {
                return static_cast<double>(static_cast<float>(blocksDestroyed - secondMostBlocksDestroyed));
            }
        }
        else if (mPodium == PODIUM_THIRD)
        {
            bool placed = player.get() == firstPlace.get() ||
                player.get() == secondPlace.get() || player.get() == thirdPlace.get();

            if (placed && blocksDestroyed >= thirdMostBlocksDestroyed &&
                ((player.get() != firstPlace.get() && player.get() != secondPlace.get()) ||
                 blocksDestroyed < secondMostBlocksDestroyed))
            {
                return static_cast<double>(static_cast<float>(blocksDestroyed - thirdMostBlocksDestroyed));
            }
        }
    }

    return 0.0;
}
