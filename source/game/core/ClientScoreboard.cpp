#include "game/core/ClientScoreboard.hpp"
#include "game/core/StringIds.hpp"
#include "game/minigame/GameMedal.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/Player.hpp"

#include <algorithm>
#include <limits>

static const unsigned short NO_VOTES_GIVEN[4] = { 0, 0, 0, 0 };
static const unsigned short NO_VOTES_RECEIVED[4] = { 0, 0, 0, 0 };
static const double ACTIVE_POSITION_ORDER = 100.0;

static int getEvidencedTrapsSetOff(const Scoreboard::PlayerStats::_RoundStats& stats)
{
    return stats.mMobKills;
}

static int getEvidencedJumps(const Scoreboard::PlayerStats::_RoundStats& stats)
{
    return stats.mCheckpointsPassed;
}

static bool selectGlideWinner(const ClientScoreboard* scoreboard, bool usePoints,
                              StatsUID* winner, int* winnerRound)
{
    bool found = false;
    unsigned int best = usePoints ? 0u : 0xffffffffu;
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
            const unsigned int value = usePoints
                ? static_cast<unsigned int>(stats.mScore + stats.mPlacement)
                : static_cast<unsigned int>(stats.mTimeAlive);
            if ((usePoints && best < value) || (!usePoints && value < best))
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
            current->setRecordSUID(stats->getRecordSUID());
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
        round = mCurrentRound;
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
    return stats == NULL ? -1 : stats->mLastCheckpointSequenceId;
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
            if (*it != NULL && !(*it)->isSpectator())
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
    return L10N::GetString(IDS_MG_YOURSELF);
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
    std::wstring best = L10N::GetString(IDS_MG_YOURSELF);
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
    return stats == NULL ? 0 : stats->mKills;
}

int ClientScoreboard::GetDamageReceived(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mDeaths;
}

int ClientScoreboard::GetLifeSpan(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    if (stats != NULL)
    {
        return stats->mPlacement;
    }
    return Minecraft::InMiniGame(eMiniGameId_Glide, true) ? -1 : 0;
}

int ClientScoreboard::GetBestLifeSpan(const StatsUID& suid, int* round)
{
    PlayerStats* player = GetPlayerStats(suid);
    unsigned int best = 0xffffffffu;
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        for (; it != player->mRoundStats.end(); ++it)
        {
            if (it->second.mWinner && static_cast<unsigned int>(it->second.mPlacement) < best)
            {
                best = static_cast<unsigned int>(it->second.mPlacement);
                if (round != NULL) *round = it->first;
            }
        }
    }
    return static_cast<int>(best);
}

int ClientScoreboard::GetBestLifeSpan()
{
    unsigned int best = 0xffffffffu;
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        int value = GetBestLifeSpan(it->first, NULL);
        if (static_cast<unsigned int>(value) < best) best = static_cast<unsigned int>(value);
    }
    return static_cast<int>(best);
}

int ClientScoreboard::GetTotalLifeSpan(const StatsUID& suid)
{
    PlayerStats* player = GetPlayerStats(suid);
    int total = 0;
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        for (; it != player->mRoundStats.end(); ++it) total += it->second.mPlacement;
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
        for (; it != player->mRoundStats.end(); ++it) if (it->second.mActive) ++total;
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
    return stats == NULL ? 0 : stats->mDamageTaken;
}

int ClientScoreboard::GetBlocksPlaced(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mDamageDealt;
}

int ClientScoreboard::GetSecondaryTargets(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mMobKills;
}

int ClientScoreboard::GetLapsCompleted(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mEndZoneEntered;
}

int ClientScoreboard::GetPoints(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mItemsCollected + stats->mBlocksMined;
}

int ClientScoreboard::GetLastPointsAdded(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    if (stats == NULL)
    {
        return 0;
    }
    int points = stats->mTotalTime;
    if (points == 0)
    {
        points = stats->mTimePlayed;
    }
    return points;
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
            int points = it->second.mItemsCollected + it->second.mBlocksMined;
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
    return stats == NULL ? -1 : stats->mLapsCompleted;
}

int ClientScoreboard::GetDeaths(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mCheckpointsPassed;
}

double ClientScoreboard::GetPercentageOfMapTraversed(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0.0 : stats->mFallDistance;
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
        for (; it != player->mRoundStats.end(); ++it) total += it->second.mDamageTaken;
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
    if (round == 0xffffffffu)
        round = static_cast<unsigned int>(mCurrentRound);
    unsigned int longest = 0;
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        if (it->second != NULL && longest < static_cast<unsigned int>(it->second->mRoundStats[round].mPlacement))
            longest = static_cast<unsigned int>(it->second->mRoundStats[round].mPlacement);
    }
    return static_cast<int>(longest);
}

void ClientScoreboard::GetPlayersSortedByPoints(std::list<std::pair<std::wstring, int> >& players, int round)
{
    if (round == -1)
    {
        round = mCurrentRound;
    }
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        std::pair<std::wstring, int> entry(GetDisplayName(it->first), GetPoints(it->first, round));
        std::list<std::pair<std::wstring, int> >::iterator insert = players.begin();
        while (insert != players.end() && insert->second >= entry.second)
        {
            ++insert;
        }
        players.insert(insert, entry);
    }
}

namespace
{
struct PositionEntry
{
    std::wstring name;
    int placement;
    double order;
    unsigned int tie;
};
}

void ClientScoreboard::GetPlayersSortedByPosition(std::list<std::pair<std::wstring, int> >& players, int round)
{
    if (round == -1)
    {
        round = mCurrentRound;
    }
    std::list<PositionEntry> entries;
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        PlayerStats::_RoundStats* stats = GetPlayerRoundStats(it->first, round);
        if (stats != NULL)
        {
            PositionEntry entry;
            entry.name = GetDisplayName(it->first);
            entry.placement = stats->mActive ? stats->mPlacement : 0;
            entry.order = stats->mActive ? ACTIVE_POSITION_ORDER : stats->mBestTime;
            entry.tie = stats->mActive ? (unsigned int)stats->mPlacement : 0;
            std::list<PositionEntry>::iterator insert = entries.begin();
            while (insert != entries.end() &&
                   (insert->order < entry.order ||
                    (insert->order == entry.order && insert->tie <= entry.tie)))
            {
                ++insert;
            }
            entries.insert(insert, entry);
        }
    }
    for (std::list<PositionEntry>::const_iterator entry = entries.begin();
         entry != entries.end(); ++entry)
    {
        players.push_back(std::make_pair(entry->name, entry->placement));
    }
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
    {
        return suid == StatsUID::EMPTY_SUID();
    }
    StatsUID winner = StatsUID::EMPTY_SUID();
    int bestWins = 0;
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        if (it->second == NULL)
            continue;
        int wins = 0;
        PlayerStats::RoundStatsMap::iterator round = it->second->mRoundStats.begin();
        for (; round != it->second->mRoundStats.end(); ++round)
        {
            if (round->second.mActive)
            {
                ++wins;
            }
        }
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
            if (rit->second.mWinner) return true;
        }
    }
    return false;
}

std::wstring ClientScoreboard::GetGlideOverallWinner(bool usePoints)
{
    std::wstring result = L"clock";
    unsigned int best = usePoints ? 0u : 0xffffffffu;
    PlayerStatsMap::iterator player = mPlayerStats.begin();
    for (; player != mPlayerStats.end(); ++player)
    {
        if (player->second == NULL)
            continue;
        PlayerStats::RoundStatsMap::iterator round = player->second->mRoundStats.begin();
        for (; round != player->second->mRoundStats.end(); ++round)
        {
            const PlayerStats::_RoundStats& stats = round->second;
            if (!stats.mWinner)
                continue;
            const unsigned int value = usePoints
                ? static_cast<unsigned int>(stats.mItemsCollected + stats.mBlocksMined)
                : static_cast<unsigned int>(stats.mPlacement);
            if ((usePoints && best < value) || (!usePoints && value < best))
            {
                best = value;
                result = player->second->mName;
            }
        }
    }
    return result;
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
