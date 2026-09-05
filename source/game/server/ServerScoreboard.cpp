#include "game/server/ServerScoreboard.hpp"

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
    boost::unordered_map<StatsUID, Scoreboard::PlayerStats*, boost::hash<StatsUID>,
                         std::equal_to<StatsUID> >::iterator it = mPlayerStats.find(suid);
    if (it != mPlayerStats.end())
    {
        delete it->second;
        mPlayerStats.erase(it);
    }
}

void ServerScoreboard::SyncPlayerScores(std::vector<boost::shared_ptr<ServerPlayer> >* players,
                                        boost::shared_ptr<ServerPlayer> player)
{
    if (players != NULL)
    {
        std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin();
        while (it != players->end())
        {
            ++it;
        }
    }
    player.reset();
}

void ServerScoreboard::ResetPlayerScores()
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

void ServerScoreboard::removePlayerFromTeam(PlayerUID uid, PlayerTeam* pTeam)
{
    Scoreboard::removePlayerFromTeam(uid, pTeam);
}

void ServerScoreboard::addPlayerToTeam(PlayerUID uid, PlayerTeam* pTeam)
{
    Scoreboard::addPlayerToTeam(uid, pTeam);
}

void ServerScoreboard::onTeamAdded(PlayerTeam* pTeam)
{
    Scoreboard::onTeamAdded(pTeam);
}

void ServerScoreboard::onTeamChanged(PlayerTeam* pTeam)
{
    Scoreboard::onTeamChanged(pTeam);
}

void ServerScoreboard::onTeamRemoved(PlayerTeam* pTeam)
{
    Scoreboard::onTeamRemoved(pTeam);
}

void ServerScoreboard::syncTeamChanges(PlayerTeam* pTeam)
{
    Scoreboard::syncTeamChanges(pTeam);
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
