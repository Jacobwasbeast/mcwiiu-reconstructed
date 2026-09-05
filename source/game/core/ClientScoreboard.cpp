#include "game/core/ClientScoreboard.hpp"
#include "game/minigame/GameMedal.hpp"

#include <algorithm>
#include <limits>

ClientScoreboard::ClientScoreboard()
    : Scoreboard(), mPlayerStats()
{
}

ClientScoreboard::~ClientScoreboard()
{
    ResetPlayerScores();
}

void ClientScoreboard::SyncPlayerStats(const StatsUID& suid, PlayerStats* stats, int round)
{
    PlayerStats*& current = mPlayerStats[suid];
    if (current == NULL)
    {
        current = new PlayerStats(*stats);
        return;
    }
    current->mRoundStats[round] = stats->mRoundStats[round];
    if (stats->mRoundStats[round].mActive)
    {
        current->mName = stats->mName;
        current->mFavouriteWeapon = stats->mFavouriteWeapon;
        current->mWeaknessWeapon = stats->mWeaknessWeapon;
        current->mMedal = stats->mMedal;
        current->mRoundStats[round].mPerLife = stats->mRoundStats[round].mPerLife;
    }
}

unsigned long long ClientScoreboard::UpdatePlayerStats(const StatsUID& suid, PlayerStats* stats,
                                                        int round, int life,
                                                        unsigned long long changed)
{
    PlayerStats* current = GetPlayerStats(suid);
    if (current != NULL)
    {
        current->updateFromOtherStats(stats, round, life, changed);
        if ((changed & 1) != 0)
        {
            current->setRecordSUID(stats->getRecordSUID());
        }
    }
    return 0;
}

ClientScoreboard::PlayerStats* ClientScoreboard::GetPlayerStats(const StatsUID& suid)
{
    PlayerStatsMap::iterator found = mPlayerStats.find(suid);
    return found == mPlayerStats.end() ? NULL : found->second;
}

Scoreboard::PlayerStats::_RoundStats* ClientScoreboard::GetPlayerRoundStats(const StatsUID& suid,
                                                                             int round)
{
    PlayerStats* stats = GetPlayerStats(suid);
    if (stats == NULL)
    {
        return NULL;
    }
    return &stats->mRoundStats[round];
}

int ClientScoreboard::GetLastBankedCheckpointID(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? -1 : stats->mLastCheckpointId;
}

void ClientScoreboard::RemovePlayerStats(const StatsUID& suid)
{
    PlayerStatsMap::iterator found = mPlayerStats.find(suid);
    if (found != mPlayerStats.end())
    {
        delete found->second;
        mPlayerStats.erase(found);
    }
}

void ClientScoreboard::ResetPlayerScores()
{
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        delete it->second;
    }
    mPlayerStats.clear();
}

int ClientScoreboard::GetActivePlayerCount(EMiniGameId)
{
    int count = 0;
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        if (it->second != NULL && GetPlayerRoundStats(it->first, -1) != NULL
            && GetPlayerRoundStats(it->first, -1)->mActive)
        {
            ++count;
        }
    }
    return count;
}

std::wstring ClientScoreboard::GetDisplayName(const StatsUID& suid)
{
    PlayerStats* stats = GetPlayerStats(suid);
    return stats == NULL ? L"" : stats->mName;
}

int ClientScoreboard::GetMedal(const StatsUID& suid)
{
    PlayerStats* stats = GetPlayerStats(suid);
    return stats == NULL || stats->mMedal == NULL ? 0 : stats->mMedal->GetId();
}

bool ClientScoreboard::IsWinner(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats != NULL && stats->mWinner;
}

bool ClientScoreboard::GetSurvived(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats != NULL && stats->mFinished;
}

int ClientScoreboard::GetKills(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mKills;
}

Scoreboard::Weapon ClientScoreboard::GetFavouriteWeapon(const StatsUID& suid)
{
    PlayerStats* stats = GetPlayerStats(suid);
    return stats == NULL ? Weapon() : stats->mFavouriteWeapon;
}

Scoreboard::Weapon ClientScoreboard::GetWeaknessWeapon(const StatsUID& suid)
{
    PlayerStats* stats = GetPlayerStats(suid);
    return stats == NULL ? Weapon() : stats->mWeaknessWeapon;
}

std::wstring ClientScoreboard::GetKiller(const StatsUID& suid, int round, int life)
{
    PlayerStats* stats = GetPlayerStats(suid);
    if (stats == NULL)
    {
        return L"";
    }
    return stats->getRecord(eStringArg_KillerName, round, life);
}

std::wstring ClientScoreboard::GetNemesis(const StatsUID& suid)
{
    PlayerStats* stats = GetPlayerStats(suid);
    return stats == NULL ? L"" : stats->mName;
}

Scoreboard::Weapon ClientScoreboard::GetKillerWeapon(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? Weapon() : stats->mKillerWeapon;
}

int ClientScoreboard::GetDamageDealt(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mDamageDealt;
}

int ClientScoreboard::GetDamageReceived(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mDamageTaken;
}

int ClientScoreboard::GetLifeSpan(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mTimeAlive;
}

int ClientScoreboard::GetBestLifeSpan(const StatsUID& suid, int* round)
{
    PlayerStats* player = GetPlayerStats(suid);
    int best = -1;
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        for (; it != player->mRoundStats.end(); ++it)
        {
            if (it->second.mActive && (best < 0 || it->second.mTimeAlive < best))
            {
                best = it->second.mTimeAlive;
                if (round != NULL) *round = it->first;
            }
        }
    }
    return best;
}

int ClientScoreboard::GetBestLifeSpan()
{
    int best = 0;
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        int value = GetBestLifeSpan(it->first, NULL);
        if (value > best) best = value;
    }
    return best;
}

int ClientScoreboard::GetTotalLifeSpan(const StatsUID& suid)
{
    PlayerStats* player = GetPlayerStats(suid);
    int total = 0;
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        for (; it != player->mRoundStats.end(); ++it) total += it->second.mTimeAlive;
    }
    return total;
}

int ClientScoreboard::GetRoundsWon(const StatsUID& suid)
{
    PlayerStats* player = GetPlayerStats(suid);
    int total = 0;
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        for (; it != player->mRoundStats.end(); ++it) if (it->second.mWinner) ++total;
    }
    return total;
}

bool ClientScoreboard::IsWinningOnMatchPoint(const StatsUID& suid)
{
    return GetRoundsWon(suid) == 2;
}

int ClientScoreboard::GetBlocksDestroyed(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mBlocksMined;
}

int ClientScoreboard::GetBlocksPlaced(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mBlocksPlaced;
}

int ClientScoreboard::GetSecondaryTargets(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mCheckpointsPassed;
}

int ClientScoreboard::GetLapsCompleted(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mLapsCompleted;
}

int ClientScoreboard::GetPoints(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mScore;
}

int ClientScoreboard::GetLastPointsAdded(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mScore;
}

int ClientScoreboard::GetBestPoints(const StatsUID& suid, int* round)
{
    PlayerStats* player = GetPlayerStats(suid);
    int best = 0;
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        for (; it != player->mRoundStats.end(); ++it)
        {
            int points = it->second.mScore;
            if (it->second.mWinner && points > best)
            {
                best = points;
                if (round != NULL) *round = it->first;
            }
        }
    }
    return best;
}

int ClientScoreboard::GetBestPoints()
{
    int best = 0;
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it) best = std::max(best, GetBestPoints(it->first, NULL));
    return best;
}

void ClientScoreboard::SetPersonalBest(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    if (stats != NULL) stats->mFinished = true;
}

bool ClientScoreboard::IsPersonalBest(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats != NULL && stats->mFinished;
}

int ClientScoreboard::GetLastCheckpointID(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? -1 : stats->mLastCheckpointId;
}

int ClientScoreboard::GetLastCheckpointSequenceID(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? -1 : stats->mLastCheckpointSequenceId;
}

int ClientScoreboard::GetDeaths(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mDeaths;
}

double ClientScoreboard::GetPercentageOfMapTraversed(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0.0 : stats->mDistanceTravelled;
}

bool ClientScoreboard::GetEndZoneEntered(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats != NULL && stats->mEndZoneEntered != 0;
}

int ClientScoreboard::GetTotalBlocksDestroyed(const StatsUID& suid)
{
    PlayerStats* player = GetPlayerStats(suid);
    int total = 0;
    if (player != NULL) for (PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin(); it != player->mRoundStats.end(); ++it) total += it->second.mBlocksMined;
    return total;
}

int ClientScoreboard::GetTotalDistanceTravelled(const StatsUID& suid)
{
    PlayerStats* player = GetPlayerStats(suid);
    int total = 0;
    if (player != NULL) for (PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin(); it != player->mRoundStats.end(); ++it) total += (int)it->second.mDistanceTravelled;
    return total;
}

int ClientScoreboard::GetTotalTrapsSetOff(const StatsUID& suid)
{
    return GetTotalBlocksDestroyed(suid);
}

int ClientScoreboard::GetTotalJumps(const StatsUID& suid)
{
    return GetTotalLifeSpan(suid);
}

int ClientScoreboard::GetVotesGiven(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mKillsOther;
}

int ClientScoreboard::GetVotesReceived(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mDeathsOther;
}

int ClientScoreboard::GetRoundLength(unsigned int round)
{
    int longest = 0;
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        PlayerStats::_RoundStats* stats = GetPlayerRoundStats(it->first, (int)round);
        if (stats != NULL) longest = std::max(longest, stats->mTimePlayed);
    }
    return longest;
}

void ClientScoreboard::GetPlayersSortedByPoints(std::list<std::pair<std::wstring, int> >& players, int round)
{
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it) players.push_back(std::make_pair(GetDisplayName(it->first), GetPoints(it->first, round)));
    players.sort();
}

void ClientScoreboard::GetPlayersSortedByPosition(std::list<std::pair<std::wstring, int> >& players, int round)
{
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        PlayerStats::_RoundStats* stats = GetPlayerRoundStats(it->first, round);
        if (stats != NULL) players.push_back(std::make_pair(GetDisplayName(it->first), stats->mPlacement));
    }
    players.sort();
}

bool ClientScoreboard::HasStats()
{
    return !mPlayerStats.empty();
}

bool ClientScoreboard::HasStats(const StatsUID& suid)
{
    return HasStats() && GetPlayerStats(suid) != NULL;
}

bool ClientScoreboard::HasWinner(int round)
{
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it) if (IsWinner(it->first, round)) return true;
    return false;
}

std::wstring ClientScoreboard::GetWinnerName(int round)
{
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it) if (IsWinner(it->first, round)) return GetDisplayName(it->first);
    return L"";
}

bool ClientScoreboard::IsOverallWinner(const StatsUID& suid)
{
    return GetRoundsWon(suid) > 0;
}

bool ClientScoreboard::GlideHasOverallWinner()
{
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it) if (GetEndZoneEntered(it->first, -1)) return true;
    return false;
}

std::wstring ClientScoreboard::GetGlideOverallWinner(bool usePoints)
{
    StatsUID winner = StatsUID::EMPTY_SUID();
    int best = usePoints ? -1 : std::numeric_limits<int>::max();
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        int value = usePoints ? GetBestPoints(it->first, NULL) : GetBestLifeSpan(it->first, NULL);
        if (GetEndZoneEntered(it->first, -1) && ((usePoints && value > best) || (!usePoints && value < best)))
        {
            winner = it->first;
            best = value;
        }
    }
    return winner == StatsUID::EMPTY_SUID() ? L"" : GetDisplayName(winner);
}

bool ClientScoreboard::IsOverallGlideWinner(const StatsUID& suid, bool usePoints)
{
    return GetGlideOverallWinner(usePoints) == GetDisplayName(suid) && GetEndZoneEntered(suid, -1);
}
