#include "game/core/GameStats.hpp"

#include "game/core/CombatEntry.hpp"
#include "game/core/System.hpp"
#include "game/util/Math.hpp"

GameStats::GameStats()
    : mPlayerCount(0), mRound(0), mRoundStartTime(0), mRoundEndTime(0), mRoundTime(0), mFarthestCheckpointReached(0)
{
}

GameStats::~GameStats()
{
    mPlayerStats.clear();
}

PlayerStats* GameStats::GetPlayerStats(const StatsUID& uid)
{
    StatsMap::iterator it = mPlayerStats.find(uid);
    return it == mPlayerStats.end() ? 0 : it->second;
}

boost::shared_ptr<ServerPlayer> GameStats::GetFirstPlace()
{
    return boost::shared_ptr<ServerPlayer>();
}

unsigned int GameStats::GetShortestLifespan(unsigned int) const
{
    return 0x7fffffffU;
}

GameStats::StatsMap* GameStats::GetPlayerStats()
{
    return &mPlayerStats;
}

unsigned int GameStats::GetDeathCount(const StatsUID&)
{
    return 0;
}

void GameStats::RecordBlockPresent(const StatsUID&, const BlockPos&, int)
{
}

unsigned int GameStats::GetRoundTime()
{
    if (mRoundStartTime == 0)
        return 0;
    unsigned long long now = static_cast<unsigned long long>(System::processTimeInMilliSecs());
    return static_cast<unsigned int>((now - mRoundStartTime) / 1000);
}

void GameStats::AwardMG01Medals(const std::vector<boost::shared_ptr<ServerPlayer> >*)
{
}
bool GameStats::SORT_MG02_STATS(PlayerStats* first, PlayerStats* second) { return first < second; }
PlayerStats* GameStats::getPlayerFromStatsVec(const std::vector<PlayerStats*>& stats, unsigned int index) { return index < stats.size() ? stats[index] : 0; }
void GameStats::AwardMG02Medals(const std::vector<boost::shared_ptr<ServerPlayer> >*)
{
}
bool GameStats::SORT_MG03_STATS_FALLBACK(PlayerStats* first, PlayerStats* second) { return first < second; }
bool GameStats::SORT_MG03_STATS_POINTS(PlayerStats* first, PlayerStats* second) { return first < second; }
bool GameStats::SORT_MG03_STATS_TIME(PlayerStats* first, PlayerStats* second) { return first < second; }
bool GameStats::SORT_MG03_STATS_POINTS_CURRENT_ROUND(PlayerStats* first, PlayerStats* second) { return first < second; }
bool GameStats::SORT_MG03_STATS_TIME_CURRENT_ROUND(PlayerStats* first, PlayerStats* second) { return first < second; }
void GameStats::AwardMG03Medals(const std::vector<boost::shared_ptr<ServerPlayer> >*)
{
}
void GameStats::AwardMG04Medals(const std::vector<boost::shared_ptr<ServerPlayer> >*)
{
}
void GameStats::CalculateStats(const std::vector<boost::shared_ptr<ServerPlayer> >*, bool)
{
}

void GameStats::AddPlayerStats(const boost::shared_ptr<ServerPlayer>&) { ++mPlayerCount; }
void GameStats::AddPlayers(const std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    if (players != 0)
        for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin(); it != players->end(); ++it)
            AddPlayerStats(*it);
}
void GameStats::RemovePlayerStats(const boost::shared_ptr<ServerPlayer>&) { if (mPlayerCount != 0) --mPlayerCount; }
void GameStats::Clear() { mPlayerStats.clear(); mPlayerCount = 0; mRoundTime = 0; mFarthestCheckpointReached = 0; }
void GameStats::OnRoundStart() { mRoundStartTime = static_cast<unsigned int>(System::processTimeInMilliSecs()); }
void GameStats::OnRoundEnd() { mRoundEndTime = static_cast<unsigned int>(System::processTimeInMilliSecs()); }
void GameStats::OnGameStart() { Clear(); OnRoundStart(); }
void GameStats::OnGameEnd() { mRoundEndTime = static_cast<unsigned int>(System::processTimeInMilliSecs()); }

int GameStats::GetDamageTakenThisRound(const StatsUID&) { return 0; }
unsigned int GameStats::GetKillsWithBowThisRound(const StatsUID&) { return 0; }
unsigned int GameStats::GetMaxSnowballHitsOnAnyPlayer(const StatsUID&) { return 0; }
void GameStats::RecordDamage(boost::shared_ptr<Entity>, CombatEntry*, boost::shared_ptr<ClientboundChatPacket>) { }
unsigned int GameStats::GetPlayerFatalDamage(const StatsUID&) { return 0; }
void GameStats::RecordDeath(boost::shared_ptr<Entity>) { }
void GameStats::RecordJump(const StatsUID&) { }
void GameStats::RecordItem(const StatsUID&, not_null_ptr<ItemInstance>) { }
void GameStats::RecordContainer(const StatsUID&, int, int, int, int) { }
void GameStats::RecordWalk(const StatsUID&, int, bool, bool) { }
void GameStats::RecordSwim(const StatsUID&, int, bool) { }
void GameStats::RecordClimb(const StatsUID&, int) { }
void GameStats::RecordGainPoints(const StatsUID&, int) { }
void GameStats::ResetTempPoints(const StatsUID&) { }
unsigned int GameStats::RecordSecondaryTargetReached(const StatsUID&, const std::wstring&, int, GlideRingGenerator::eGlideRingSize) { return 0; }
void GameStats::TakeTimestamp(const StatsUID&, eTimeSpentCategory, bool) { }
void GameStats::RecordUseItem(const StatsUID&, not_null_ptr<ItemInstance>) { }
void GameStats::RecordTimeOnLayer(const StatsUID&, int, bool) { }
void GameStats::RecordSnowballDirectHit(const StatsUID&) { }
void GameStats::RecordHitBySnowball(const StatsUID&) { }
void GameStats::RecordLanding(const StatsUID&) { }
void GameStats::RecordBlockPlaced(const StatsUID&, const BlockPos&, int) { }
void GameStats::RecordBlockDestroyed(const StatsUID&, const BlockPos&, int) { }
unsigned int GameStats::RecordLastCheckpoint(const StatsUID&, int checkpoint, int, bool)
{
    if (static_cast<unsigned int>(checkpoint) > mFarthestCheckpointReached)
        mFarthestCheckpointReached = checkpoint;
    return static_cast<unsigned int>(checkpoint);
}
void GameStats::RecordLapCompleted(const StatsUID&) { }
unsigned int GameStats::GetLapsCompleted(const StatsUID&) { return 0; }
unsigned int GameStats::GetLastCheckpointID(const StatsUID&) { return 0; }
unsigned int GameStats::GetLastCheckpointSequenceID(const StatsUID&) { return 0; }
void GameStats::RecordThermalEntered(const StatsUID&) { }
void GameStats::RecordProgress(const StatsUID&, double, double, double, double) { }
void GameStats::RecordSpawnedAtCheckpoint(const StatsUID&, int) { }
void GameStats::RecordTimeSpentInPosition(bool) { }
unsigned int GameStats::RecordEndZoneEntered(const StatsUID&) { return 0; }
unsigned char GameStats::GetEndZoneEntered(const StatsUID&) { return 0; }
boost::shared_ptr<ServerPlayer> GameStats::GetLeadingPlayer() { return boost::shared_ptr<ServerPlayer>(); }
boost::shared_ptr<ServerPlayer> GameStats::GetTrailingPlayer(const boost::shared_ptr<ServerPlayer>& player) { return player; }
void GameStats::increasePlayerRoundStartTimeMs(long long milliseconds) { mRoundStartTime += static_cast<unsigned int>(milliseconds); }
long long GameStats::GetStartTime() { return mRoundStartTime; }
boost::shared_ptr<ServerPlayer> GameStats::GetRoundWinner() { return boost::shared_ptr<ServerPlayer>(); }
boost::shared_ptr<ServerPlayer> GameStats::GetSecondPlace() { return boost::shared_ptr<ServerPlayer>(); }
boost::shared_ptr<ServerPlayer> GameStats::GetThirdPlace() { return boost::shared_ptr<ServerPlayer>(); }
std::wstring GameStats::GetWinningThemeChoice() const { return std::wstring(); }
unsigned int GameStats::GetRoundsWon(const StatsUID&) { return 0; }
unsigned int GameStats::GetMostBlocksDestroyed() { return 0; }
unsigned int GameStats::GetSecondMostBlocksDestroyed() { return 0; }
unsigned int GameStats::GetThirdMostBlocksDestroyed() { return 0; }
unsigned int GameStats::GetFastestPlayerTime() { return GetShortestLifespan(mRound); }
unsigned int GameStats::GetHighestScoringPlayerScore() { return 0; }
unsigned int GameStats::GetFarthestCheckpointReached() { return mFarthestCheckpointReached; }
