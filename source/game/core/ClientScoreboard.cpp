#include "game/core/ClientScoreboard.hpp"
#include "game/minigame/GameMedal.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/Player.hpp"

#include <algorithm>
#include <cstring>
#include <limits>

static const unsigned short NO_VOTES_GIVEN[4] = { 0, 0, 0, 0 };
static const unsigned short NO_VOTES_RECEIVED[4] = { 0, 0, 0, 0 };

static int getEvidencedTrapsSetOff(const Scoreboard::PlayerStats::_RoundStats& stats)
{
    return *reinterpret_cast<const int*>(reinterpret_cast<const unsigned char*>(&stats) + 0x44);
}

static int getEvidencedJumps(const Scoreboard::PlayerStats::_RoundStats& stats)
{
    return *reinterpret_cast<const int*>(reinterpret_cast<const unsigned char*>(&stats) + 0x48);
}

static int getEvidencedLastPointsAdded(const Scoreboard::PlayerStats::_RoundStats& stats)
{
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&stats);
    const int secondary = *reinterpret_cast<const int*>(bytes + 0xa4);
    return secondary != 0 ? secondary : *reinterpret_cast<const int*>(bytes + 0xa0);
}

static int getCurrentRound(const ClientScoreboard* scoreboard)
{
    return *reinterpret_cast<const int*>(reinterpret_cast<const unsigned char*>(scoreboard) + 0x30);
}

static bool selectGlideWinner(const ClientScoreboard* scoreboard, bool usePoints,
                              StatsUID* winner, int* winnerRound)
{
    bool found = false;
    int best = usePoints ? 0 : -1;
    ClientScoreboard::PlayerStatsMap::const_iterator player = scoreboard->mPlayerStats.begin();
    for (; player != scoreboard->mPlayerStats.end(); ++player)
    {
        if (player->second == NULL)
        {
            continue;
        }
        Scoreboard::PlayerStats::RoundStatsMap::const_iterator round = player->second->mRoundStats.begin();
        for (; round != player->second->mRoundStats.end(); ++round)
        {
            const Scoreboard::PlayerStats::_RoundStats& stats = round->second;
            if (!stats.mEndZoneEntered)
            {
                continue;
            }
            const int value = usePoints ? stats.mScore + stats.mPlacement : stats.mTimeAlive;
            if ((usePoints && value > best) || (!usePoints && value < best))
            {
                found = true;
                best = value;
                if (winner != NULL) *winner = player->first;
                if (winnerRound != NULL) *winnerRound = round->first;
            }
        }
    }
    return found;
}

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

void ClientScoreboard::UpdatePlayerStats(const StatsUID& suid, PlayerStats* stats,
                                         int round, int life,
                                         unsigned long long changed)
{
    PlayerStats* current = GetPlayerStats(suid);
    if (current != NULL)
    {
        current->updateFromOtherStats(stats, round, life, changed);
        if ((changed & 1) != 0)
        {
            std::memcpy(current, stats, sizeof(unsigned int) * 5);
        }
    }
}

ClientScoreboard::PlayerStats* ClientScoreboard::GetPlayerStats(const StatsUID& suid)
{
    PlayerStatsMap::iterator found = mPlayerStats.find(suid);
    return found == mPlayerStats.end() ? NULL : found->second;
}

Scoreboard::PlayerStats::_RoundStats* ClientScoreboard::GetPlayerRoundStats(const StatsUID& suid,
                                                                             int round)
{
    if (round == -1)
    {
        round = getCurrentRound(this);
    }
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

int ClientScoreboard::GetActivePlayerCount(EMiniGameId miniGameId)
{
    int count = 0;
    if (miniGameId == eMiniGameId_Any)
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        if (miniGame != NULL)
        {
            miniGameId = miniGame->GetId();
        }
    }

    if (miniGameId > eMiniGameId_Glide)
    {
        return 0;
    }

    if (miniGameId < eMiniGameId_Glide)
    {
        MultiPlayerLevel* level = Minecraft::GetInstance()->getLevel(0);
        if (level == NULL)
        {
            return 0;
        }
        std::vector<boost::shared_ptr<Player> >::iterator it = level->mPlayers.begin();
        for (; it != level->mPlayers.end(); ++it)
        {
            if (*it != NULL && !(*it)->isRemoved())
            {
                ++count;
            }
        }
        return count;
    }

    if (miniGameId == eMiniGameId_Glide)
    {
        PlayerStatsMap::iterator it = mPlayerStats.begin();
        for (; it != mPlayerStats.end(); ++it)
        {
            if (GetEndZoneEntered(it->first, -1) == false)
            {
                ++count;
            }
        }
        return count;
    }
    return count;
}

std::wstring ClientScoreboard::GetDisplayName(const StatsUID& suid)
{
    PlayerStats* stats = GetPlayerStats(suid);
    return stats == NULL ? L"" : stats->mName;
}

GameMedal* ClientScoreboard::GetMedal(const StatsUID& suid)
{
    PlayerStats* stats = GetPlayerStats(suid);
    return stats == NULL ? NULL : const_cast<GameMedal*>(stats->mMedal);
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
    if (stats != NULL)
    {
        std::wstring killer = stats->getRecord(eStringArg_KillerName, round, life);
        if (!killer.empty() && killer != GetDisplayName(suid))
        {
            return killer;
        }
    }
    return L10N::GetString(0xfa198c24);
}

std::wstring ClientScoreboard::GetNemesis(const StatsUID& suid)
{
    ClientScoreboard* scoreboard = Minecraft::GetInstance()->getLevel(0)->GetClientScoreboard();
    boost::unordered_map<std::wstring, int> killCounts(11);
    int roundCount = (int)Minecraft::GetInstance()->GetMiniGame()->GetRoundCount();
    int livesPerRound = (int)Minecraft::GetInstance()->GetMiniGame()->GetLivesPerRound();
    for (int round = 0; round < roundCount; ++round)
    {
        for (int life = 0; life < livesPerRound; ++life)
        {
            std::wstring killer = scoreboard->GetKiller(suid, round, life);
            if (!killer.empty())
            {
                ++killCounts[killer];
            }
        }
    }
    std::wstring best = L10N::GetString(0xfa198c24);
    int bestCount = 0;
    boost::unordered_map<std::wstring, int>::iterator it = killCounts.begin();
    for (; it != killCounts.end(); ++it)
    {
        if (bestCount < it->second)
        {
            bestCount = it->second;
            if (it->first != GetDisplayName(suid))
            {
                best = it->first;
            }
        }
    }
    return best;
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
    if (stats != NULL)
    {
        return stats->mTimeAlive;
    }
    return Minecraft::InMiniGame(eMiniGameId_Glide, true) ? -1 : 0;
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
            if (it->second.mActive && it->second.mTimeAlive < best)
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
    int best = -1;
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        int value = GetBestLifeSpan(it->first, NULL);
        if (value < best) best = value;
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
    return stats == NULL ? 0 : stats->mScore + stats->mPlacement;
}

int ClientScoreboard::GetLastPointsAdded(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    if (stats == NULL)
    {
        return 0;
    }
    return getEvidencedLastPointsAdded(*stats);
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
            int points = it->second.mScore + it->second.mPlacement;
            if (it->second.mActive && points > best)
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
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        for (; it != player->mRoundStats.end(); ++it) total += it->second.mBlocksMined;
    }
    return total;
}

int ClientScoreboard::GetTotalDistanceTravelled(const StatsUID& suid)
{
    PlayerStats* player = GetPlayerStats(suid);
    int total = 0;
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        for (; it != player->mRoundStats.end(); ++it)
            total += (int)it->second.mDistanceTravelled;
    }
    return total;
}

int ClientScoreboard::GetTotalTrapsSetOff(const StatsUID& suid)
{
    PlayerStats* player = GetPlayerStats(suid);
    int total = 0;
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        for (; it != player->mRoundStats.end(); ++it)
            total += getEvidencedTrapsSetOff(it->second);
    }
    return total;
}

int ClientScoreboard::GetTotalJumps(const StatsUID& suid)
{
    PlayerStats* player = GetPlayerStats(suid);
    int total = 0;
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        for (; it != player->mRoundStats.end(); ++it)
            total += getEvidencedJumps(it->second);
    }
    return total;
}

const unsigned short* ClientScoreboard::GetVotesGiven(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? NO_VOTES_GIVEN : &stats->mDeathsMelee;
}

const unsigned short* ClientScoreboard::GetVotesReceived(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? NO_VOTES_RECEIVED : &stats->mKillsMelee;
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
    if (mPlayerStats.empty())
        return suid == StatsUID::EMPTY_SUID();

    StatsUID winner = StatsUID::EMPTY_SUID();
    int bestWins = 0;
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        if (it->second == NULL || it->second->mRoundStats.empty())
            continue;

        const int wins = GetRoundsWon(it->first);
        if (wins < bestWins)
            continue;
        if (wins <= bestWins)
        {
            winner = StatsUID::EMPTY_SUID();
            continue;
        }
        winner = it->first;
        bestWins = wins;
    }
    return suid == winner;
}

bool ClientScoreboard::GlideHasOverallWinner()
{
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        if (it->second == NULL)
        {
            continue;
        }
        PlayerStats::RoundStatsMap::iterator rit = it->second->mRoundStats.begin();
        for (; rit != it->second->mRoundStats.end(); ++rit)
        {
            if (rit->second.mEndZoneEntered != 0) return true;
        }
    }
    return false;
}

std::wstring ClientScoreboard::GetGlideOverallWinner(bool usePoints)
{
    StatsUID winner = StatsUID::EMPTY_SUID();
    selectGlideWinner(this, usePoints, &winner, NULL);
    return winner == StatsUID::EMPTY_SUID() ? L"" : GetDisplayName(winner);
}

bool ClientScoreboard::IsOverallGlideWinner(const StatsUID& suid, bool usePoints)
{
    int round = -1;
    const int value = usePoints ? GetBestPoints(suid, &round) : GetBestLifeSpan(suid, &round);
    const int global = usePoints ? GetBestPoints() : GetBestLifeSpan();
    if (value != global)
    {
        return false;
    }
    return GetEndZoneEntered(suid, round);
}
