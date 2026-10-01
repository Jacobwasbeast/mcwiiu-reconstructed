#include "game/core/ClientScoreboard.hpp"
#include "game/core/StringIds.hpp"
#include "game/minigame/GameMedal.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/Player.hpp"

#include <algorithm>
#include <limits>

static const unsigned short NO_VOTES_GIVEN[4] = { 0, 0, 0, 0 };
static const unsigned short NO_VOTES_RECEIVED[4] = { 0, 0, 0, 0 };

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
}

void ClientScoreboard::SyncPlayerStats(const StatsUID& suid, PlayerStats* stats, int round)
{
    if (mPlayerStats.find(suid) != mPlayerStats.end())
    {
        mPlayerStats[suid]->mRoundStats[round] = stats->mRoundStats[round];
        if (stats->mMedal != NULL)
        {
            mPlayerStats[suid]->mWeaknessWeapon = stats->mWeaknessWeapon;
            mPlayerStats[suid]->mFavouriteWeapon = stats->mFavouriteWeapon;
            mPlayerStats[suid]->mMedal = stats->mMedal;
            mPlayerStats[suid]->mName = stats->mName;
        }
    }
    else
    {
        mPlayerStats[suid] = new PlayerStats(*stats);
    }
}

void ClientScoreboard::UpdatePlayerStats(const StatsUID& suid, PlayerStats* stats,
                                         int round, int life,
                                         unsigned long long changed)
{
    PlayerStatsMap::iterator found = mPlayerStats.find(suid);
    if (found != mPlayerStats.end())
    {
        found->second->updateFromOtherStats(stats, round, life, changed);
        if ((changed & 1) != 0)
        {
            *static_cast<PlayerUID*>(mPlayerStats[suid]) = *static_cast<PlayerUID*>(stats);
        }
    }
}

ClientScoreboard::PlayerStats* ClientScoreboard::GetPlayerStats(const StatsUID& suid)
{
    PlayerStatsMap::iterator found = mPlayerStats.find(suid);
    if (found != mPlayerStats.end())
    {
        return found->second;
    }
    return NULL;
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
        miniGameId = Minecraft::GetInstance()->GetMiniGame()->GetId();
    }

    switch (miniGameId)
    {
    case eMiniGameId_Lobby:
    case eMiniGameId_Battle:
    case eMiniGameId_Tumble:
    {
        std::vector<boost::shared_ptr<Player> >* players = &Minecraft::GetInstance()->mLevel->mPlayers;
        if (players != NULL)
        {
            for (unsigned int i = 0; i < players->size(); ++i)
            {
                if ((*players)[i]->isSpectator() == false)
                {
                    ++count;
                }
            }
        }
        break;
    }
    case eMiniGameId_Glide:
        if (!mPlayerStats.empty())
        {
            PlayerStatsMap::iterator it = mPlayerStats.begin();
            for (; it != mPlayerStats.end(); ++it)
            {
                if (GetEndZoneEntered(it->first, -1) == false)
                {
                    ++count;
                }
            }
        }
        break;
    default:
        break;
    }
    return count;
}

std::wstring ClientScoreboard::GetDisplayName(const StatsUID& suid)
{
    std::wstring name = L"";
    PlayerStats* stats = GetPlayerStats(suid);
    if (stats != NULL)
    {
        name = stats->mName;
    }
    return name;
}

GameMedal* ClientScoreboard::GetMedal(const StatsUID& suid)
{
    PlayerStats* stats = GetPlayerStats(suid);
    return stats == NULL ? NULL : const_cast<GameMedal*>(stats->mMedal);
}

bool ClientScoreboard::IsWinner(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    bool winner = false;
    if (stats != NULL)
    {
        winner = stats->mActive;
    }
    return winner;
}

bool ClientScoreboard::GetSurvived(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    bool survived = false;
    if (stats != NULL)
    {
        survived = stats->mFinished;
    }
    return survived;
}

int ClientScoreboard::GetKills(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats == NULL ? 0 : stats->mScore;
}

not_null_ptr<ItemInstance> ClientScoreboard::GetFavouriteWeapon(const StatsUID& suid)
{
    not_null_ptr<ItemInstance> weapon = ItemInstance::EMPTY;
    PlayerStats* stats = GetPlayerStats(suid);
    if (stats != NULL && stats->mFavouriteWeapon.HasItem())
    {
        weapon = stats->mFavouriteWeapon.GetItemInstance();
    }
    return weapon;
}

not_null_ptr<ItemInstance> ClientScoreboard::GetWeaknessWeapon(const StatsUID& suid)
{
    not_null_ptr<ItemInstance> weapon = ItemInstance::EMPTY;
    PlayerStats* stats = GetPlayerStats(suid);
    if (stats != NULL && stats->mWeaknessWeapon.HasItem())
    {
        weapon = stats->mWeaknessWeapon.GetItemInstance();
    }
    return weapon;
}

std::wstring ClientScoreboard::GetKiller(const StatsUID& suid, int round, int life)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    if (stats != NULL)
    {
        if (life == -1)
        {
            life = Mth::clamp(stats->mCheckpointsPassed - 1, 0, 10);
        }
        if (static_cast<unsigned int>(life) < stats->mPerLife.size())
        {
            std::wstring killer = stats->mPerLife[life].mKillerName;
            if (!killer.empty() && killer.compare(GetDisplayName(suid)) != 0)
            {
                return killer;
            }
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

not_null_ptr<ItemInstance> ClientScoreboard::GetKillerWeapon(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    if (stats != NULL && stats->mKillerWeapon.HasItem())
    {
        return stats->mKillerWeapon.GetItemInstance();
    }
    return ItemInstance::EMPTY;
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
    unsigned int best = 0xffffffffu;
    PlayerStats* player = GetPlayerStats(suid);
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        for (; it != player->mRoundStats.end(); ++it)
        {
            unsigned int lifeSpan = it->second.mPlacement;
            if (it->second.mWinner && lifeSpan < best)
            {
                best = lifeSpan;
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
        PlayerStats* player = it->second;
        if (player == NULL)
        {
            continue;
        }
        PlayerStats::RoundStatsMap::iterator round = player->mRoundStats.begin();
        for (; round != player->mRoundStats.end(); ++round)
        {
            unsigned int lifeSpan = round->second.mPlacement;
            if (round->second.mWinner && lifeSpan < best)
            {
                best = lifeSpan;
            }
        }
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
    int total = 0;
    PlayerStats* player = GetPlayerStats(suid);
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        for (; it != player->mRoundStats.end(); ++it) total += it->second.mActive;
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
    int points;
    if (stats != NULL)
    {
        points = stats->mTotalTime;
        if (points == 0)
        {
            return stats->mTimePlayed;
        }
    }
    else
    {
        points = 0;
    }
    return points;
}

int ClientScoreboard::GetBestPoints(const StatsUID& suid, int* round)
{
    unsigned int best = 0;
    PlayerStats* player = GetPlayerStats(suid);
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        for (; it != player->mRoundStats.end(); ++it)
        {
            unsigned int points = it->second.mItemsCollected + it->second.mBlocksMined;
            if (it->second.mWinner && points > best)
            {
                best = points;
                if (round != NULL) *round = it->first;
            }
        }
    }
    return static_cast<int>(best);
}

int ClientScoreboard::GetBestPoints()
{
    unsigned int best = 0;
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        PlayerStats* player = it->second;
        if (player == NULL)
        {
            continue;
        }
        PlayerStats::RoundStatsMap::iterator round = player->mRoundStats.begin();
        for (; round != player->mRoundStats.end(); ++round)
        {
            unsigned int points = round->second.mItemsCollected + round->second.mBlocksMined;
            if (round->second.mWinner && points > best)
            {
                best = points;
            }
        }
    }
    return static_cast<int>(best);
}

void ClientScoreboard::SetPersonalBest(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    if (stats != NULL) stats->mQuit = true;
}

bool ClientScoreboard::IsPersonalBest(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    return stats != NULL && stats->mQuit;
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
    if (stats != NULL)
    {
        return stats->mFallDistance;
    }
    return 0.0;
}

bool ClientScoreboard::GetEndZoneEntered(const StatsUID& suid, int round)
{
    PlayerStats::_RoundStats* stats = GetPlayerRoundStats(suid, round);
    bool entered = false;
    if (stats != NULL)
    {
        entered = stats->mWinner;
    }
    return entered;
}

int ClientScoreboard::GetTotalBlocksDestroyed(const StatsUID& suid)
{
    int total = 0;
    PlayerStats* player = GetPlayerStats(suid);
    if (player != NULL)
    {
        PlayerStats::RoundStatsMap::iterator it = player->mRoundStats.begin();
        while (it != player->mRoundStats.end())
        {
            total += it->second.mDamageTaken;
            ++it;
        }
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
            total += it->second.mArrowsHit;
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
            total += it->second.mBlocksPlaced;
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
        if (it->second != NULL && static_cast<unsigned int>(it->second->mRoundStats[round].mPlacement) > longest)
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
    const double activeOrder = 100.0;
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
            entry.order = stats->mActive ? activeOrder : stats->mBestTime;
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
    if (round == -1)
    {
        round = mCurrentRound;
    }
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        if (it->second != NULL && it->second->mRoundStats[round].mActive)
        {
            return true;
        }
    }
    return false;
}

std::wstring ClientScoreboard::GetWinnerName(int round)
{
    if (round == -1)
    {
        round = mCurrentRound;
    }
    std::wstring name = L"";
    PlayerStatsMap::iterator it = mPlayerStats.begin();
    for (; it != mPlayerStats.end(); ++it)
    {
        if (it->second != NULL && it->second->mRoundStats[round].mActive)
        {
            name = it->second->mName;
            break;
        }
    }
    return name;
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
        PlayerStats* player = it->second;
        if (player == NULL)
        {
            continue;
        }
        PlayerStats::RoundStatsMap::iterator rit = player->mRoundStats.begin();
        for (; rit != player->mRoundStats.end(); ++rit)
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
