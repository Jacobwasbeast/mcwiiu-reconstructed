#include "game/server/ServerScoreboard.hpp"

#include "game/core/GameStats.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/network/ClientboundSetPlayerTeamPacket.hpp"
#include "game/network/ScoreboardPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"

#include <vector>

ServerScoreboard::ServerScoreboard(MinecraftServer* server)
    : Scoreboard(), mServer(server), mPlayerStats()
{
}

ServerScoreboard::~ServerScoreboard()
{
    boost::unordered_map<StatsUID, Scoreboard::PlayerStats*, boost::hash<StatsUID>,
                         std::equal_to<StatsUID> >::iterator it = mPlayerStats.begin();
    while (it != mPlayerStats.end())
    {
        delete it->second;
        ++it;
    }
    mPlayerStats.clear();
}

Scoreboard::PlayerStats* ServerScoreboard::GetPlayerStats(const StatsUID& suid, int round)
{
    boost::unordered_map<StatsUID, Scoreboard::PlayerStats*, boost::hash<StatsUID>,
                         std::equal_to<StatsUID> >::iterator it = mPlayerStats.find(suid);
    if (it == mPlayerStats.end())
    {
        Scoreboard::PlayerStats* stats = new Scoreboard::PlayerStats();
        stats->setRecordSUID(suid);
        mPlayerStats.insert(std::make_pair(suid, stats));
        return stats;
    }
    return it->second;
}

void ServerScoreboard::OnPlayerStatsChanged(const StatsUID& suid, int round)
{
    Scoreboard::PlayerStats* stats = GetPlayerStats(suid, round);
    if (stats != NULL)
    {
        stats->getRecord(Scoreboard::eStringArg_PlayerName, round, 0);
    }
}

void ServerScoreboard::OnPlayerStatsRemoved(const StatsUID& suid, const std::wstring& name)
{
    boost::shared_ptr<Packet> packet(new ScoreboardPacket(ScoreboardPacket::REMOVE_PLAYER_STATS,
        new ScoreboardPacket::RemovePlayerStatsData(suid)));
    mServer->getPlayers()->broadcastAll(packet);

    Scoreboard::PlayerStats*& stats = mPlayerStats[suid];
    if (stats != NULL)
    {
        delete stats;
    }
    mPlayerStats.erase(suid);
}

void ServerScoreboard::SyncPlayerScores(std::vector<boost::shared_ptr<ServerPlayer> >* players,
                                        boost::shared_ptr<ServerPlayer> player)
{
    if (players == NULL || player == NULL || player->mConnection == NULL || mServer == NULL ||
        mServer->mMasterGameMode == NULL)
    {
        return;
    }

    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin();
    for (; it != players->end(); ++it)
    {
        if (*it == NULL)
        {
            continue;
        }

        StatsUID uid = (*it)->getSUID();
        ::PlayerStats* gameStats = mServer->mMasterGameMode->mGameStats.GetPlayerStats(uid);
        if (gameStats == NULL)
        {
            continue;
        }

        int rounds = gameStats->getTotalRounds();
        for (int round = 0; round < rounds; ++round)
        {
            ::PlayerStats::RoundStats* roundStats = gameStats->getRoundStats(round);
            if (roundStats == NULL)
            {
                continue;
            }

            int lives = roundStats->getNumberLifeStats();
            for (int life = 0; life < lives; ++life)
            {
                Scoreboard::PlayerStats* stats = GetPlayerStats(uid, round);
                ScoreboardPacket::AddPlayerStatsData* data =
                    new ScoreboardPacket::AddPlayerStatsData(stats, round, life);
                boost::shared_ptr<Packet> packet(
                    new ScoreboardPacket(ScoreboardPacket::ADD_PLAYER_STATS, data));
                player->mConnection->send(packet);
            }
        }
    }
}

void ServerScoreboard::ResetPlayerScores()
{
    boost::shared_ptr<Packet> packet(
        new ScoreboardPacket(ScoreboardPacket::CLEAR_PLAYER_STATS, new ScoreboardPacket::Data()));
    mServer->getPlayers()->broadcastAll(packet);

    boost::unordered_map<StatsUID, Scoreboard::PlayerStats*, boost::hash<StatsUID>,
                         std::equal_to<StatsUID> >::iterator it = mPlayerStats.begin();
    while (it != mPlayerStats.end())
    {
        delete it->second;
        ++it;
    }
    mPlayerStats.clear();
}

void ServerScoreboard::removePlayerFromTeam(PlayerUID uid, PlayerTeam* pTeam)
{
    Scoreboard::removePlayerFromTeam(uid, pTeam);

    std::vector<PlayerUID> players;
    players.push_back(uid);
    boost::shared_ptr<Packet> packet(new ClientboundSetPlayerTeamPacket(pTeam, &players, 4));
    mServer->getPlayers()->broadcastAll(packet);
}

void ServerScoreboard::addPlayerToTeam(PlayerUID uid, PlayerTeam* pTeam)
{
    if (getPlayersTeam(uid) != NULL)
    {
        Scoreboard::removePlayerFromTeam(uid, pTeam);
    }
    Scoreboard::addPlayerToTeam(uid, pTeam);

    std::vector<PlayerUID> players;
    players.push_back(uid);
    boost::shared_ptr<Packet> packet(new ClientboundSetPlayerTeamPacket(pTeam, &players, 3));
    mServer->getPlayers()->broadcastAll(packet);
}

void ServerScoreboard::onTeamAdded(PlayerTeam* pTeam)
{
    Scoreboard::onTeamAdded(pTeam);
    boost::shared_ptr<Packet> packet(new ClientboundSetPlayerTeamPacket(pTeam, 0));
    mServer->getPlayers()->broadcastAll(packet);
}

void ServerScoreboard::onTeamChanged(PlayerTeam* pTeam)
{
    Scoreboard::onTeamChanged(pTeam);
}

void ServerScoreboard::onTeamRemoved(PlayerTeam* pTeam)
{
    Scoreboard::onTeamRemoved(pTeam);
    boost::shared_ptr<Packet> packet(new ClientboundSetPlayerTeamPacket(pTeam, 1));
    mServer->getPlayers()->broadcastAll(packet);
}

void ServerScoreboard::syncTeamChanges(PlayerTeam* pTeam)
{
    boost::shared_ptr<Packet> packet(new ClientboundSetPlayerTeamPacket(pTeam, 2));
    mServer->getPlayers()->broadcastAll(packet);
}

int ServerScoreboard::GetLastCheckpointID(const StatsUID& suid, int round)
{
    Scoreboard::PlayerStats* stats = GetPlayerStats(suid, round);
    return stats == NULL ? 0 : stats->getRecord(Scoreboard::eIntArg_LastCheckpointId, round);
}

int ServerScoreboard::GetLapsCompleted(const StatsUID& suid, int round)
{
    Scoreboard::PlayerStats* stats = GetPlayerStats(suid, round);
    return stats == NULL ? 0 : stats->getRecord(Scoreboard::eIntArg_LapsCompleted, round);
}

bool ServerScoreboard::GetEndZoneEntered(const StatsUID& suid, int round)
{
    Scoreboard::PlayerStats* stats = GetPlayerStats(suid, round);
    return stats != NULL && stats->getRecord(Scoreboard::eIntArg_EndZoneEntered, round) != 0;
}

int ServerScoreboard::GetLastCheckpointSequenceID(const StatsUID& suid, int round)
{
    Scoreboard::PlayerStats* stats = GetPlayerStats(suid, round);
    return stats == NULL ? 0 : stats->getRecord(Scoreboard::eIntArg_LastCheckpointSequenceId, round);
}
