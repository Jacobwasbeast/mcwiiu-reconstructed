#include "game/world/generation/PlayerStats.hpp"
#include <algorithm>

Scoreboard::Weapon::Weapon() : mItemId(-1), mPotionId(-1) {}
PlayerStats::RoundStats::BlockInteractionInfo::BlockInteractionInfo() : mCount(0), placed(), mined(), used() {}
PlayerStats::RoundStats::PerLifeStats::PerLifeStats() : trackers(), values() {}

PlayerStats::RoundStats::RoundStats(const StatsUID&) : trackers(), won(false), reachedEnd(false), round(0), deaths(0), kills(0), points(0), jumpCount(0), distance(), mDistance(), mDamageDealt(0), containers(), itemsUsedList(), weapon(), itemsUsed(0), potions(0), checkpointTimes(), splashPotions(0), reachedEndFlag(false), thermals(0), secondaryTargetsFlag(false), secondaryTargets(0), lastPointsAdded(0), mTimeSpent(), topSpeed(0.0), medals(), medalNames(), mBestTime(0.0), mCurrentTime(0.0), mTotalTime(0.0), itemCounts(), blocksDestroyed(), blocksPlaced(), blocksUsed(), mReserved(), lifeStats()
{
}

PlayerStats::RoundStats::~RoundStats()
{
    std::vector<UntypedStatTracker*>::iterator i;
    for (i = trackers.begin(); i != trackers.end(); ++i) delete *i;
}

PlayerStats::RoundStats::PerLifeStats* PlayerStats::RoundStats::getLifeStats(int life)
{
    while ((int)lifeStats.size() <= life) lifeStats.push_back(PerLifeStats());
    return &lifeStats.at((unsigned int)life);
}

int PlayerStats::RoundStats::getNumberLifeStats() const { return (int)lifeStats.size(); }

void PlayerStats::RoundStats::MedalTimestamp(eTimeSpentCategory category, bool start, long long timestamp)
{
    if (category < 0 || category >= 8) return;
    if (start) {
        if (mTimeSpent[category & 1] == 0 && mTimeSpent[(category & 1) + 1] == 0) {
            mTimeSpent[category & 1] = (int)(timestamp >> 32);
            mTimeSpent[(category & 1) + 1] = (int)timestamp;
        }
    } else {
        if (mTimeSpent[category & 1] != 0 || mTimeSpent[(category & 1) + 1] != 0) {
            mTimeSpent[category & 1] = 0;
            mTimeSpent[(category & 1) + 1] = 0;
        }
    }
}

PlayerStats::PlayerStats(const StatsUID& statsUID) : mStatsUID(statsUID), mMedals(), mTrackers(), mItems(), mRoundStats() {}
PlayerStats::~PlayerStats()
{
    boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) delete i->second;
    std::vector<UntypedStatTracker*>::iterator j;
    for (j = mTrackers.begin(); j != mTrackers.end(); ++j) delete *j;
}

PlayerStats::RoundStats* PlayerStats::getRoundStats(unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator i = mRoundStats.find((int)round);
    if (i != mRoundStats.end()) return i->second;
    RoundStats* stats = new RoundStats(mStatsUID);
    stats->round = (int)round;
    mRoundStats[(int)round] = stats;
    return stats;
}

void PlayerStats::updateTrackedStats(int round, const DamageEvent* damageEvent)
{
    std::vector<UntypedStatTracker*>::iterator i;
    for (i = mTrackers.begin(); i != mTrackers.end(); ++i) (*i)->update(damageEvent);
    RoundStats* stats = getRoundStats((unsigned int)round);
    for (i = stats->trackers.begin(); i != stats->trackers.end(); ++i) (*i)->update(damageEvent);
}

int PlayerStats::getTotalRounds()
{
    if (mRoundStats.empty()) return 1;
    int highest = 0;
    boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) if (highest < i->first) highest = i->first;
    return highest + 1;
}

double PlayerStats::getDamageDealt(int round) { return (double)getRoundStats((unsigned int)round)->mDistance[0]; }
double PlayerStats::getDamageTaken(int round) { return (double)getRoundStats((unsigned int)round)->mDistance[1]; }

int PlayerStats::getTotalDistanceMoved(unsigned int category)
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) total += i->second->mDistance[category % 6];
    return total;
}

double PlayerStats::getAllDamageTaken() { return getDamageTaken(0); }

int PlayerStats::getTotalJumpCount()
{
    int total = 0; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) total += i->second->jumpCount;
    return total;
}
int PlayerStats::getTotalDeaths()
{
    int total = 0; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) total += i->second->deaths;
    return total;
}
int PlayerStats::getDeaths(unsigned int round) { boost::unordered_map<int, RoundStats*>::iterator i = mRoundStats.find((int)round); return i == mRoundStats.end() ? 0 : i->second->deaths; }
int PlayerStats::getDeathsInAnyRound(int deaths)
{
    boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) if (i->second->deaths == deaths) return deaths;
    return 0;
}
int PlayerStats::getTotalKills()
{
    int total = 0; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) total += i->second->kills;
    return total;
}
void PlayerStats::getAllItems(std::map<int, int>& items) const { items = mItems; }
int PlayerStats::getTotalWalkDistance() { return getTotalDistanceMoved(0); }
int PlayerStats::getTotalSprintDistance() { return getTotalDistanceMoved(1); }
int PlayerStats::getTotalSneakDistance() { return getTotalDistanceMoved(2); }
int PlayerStats::getTotalSwimDistance() { return getTotalDistanceMoved(3); }
int PlayerStats::getTotalUnderwaterDistance() { return getTotalDistanceMoved(4); }
int PlayerStats::getTotalClimbDistance() { return getTotalDistanceMoved(5); }
void PlayerStats::getAllItemsUsed(std::vector<Item*>& items) const
{
    items.clear(); boost::unordered_map<int, RoundStats*>::const_iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) items.insert(items.end(), i->second->itemsUsedList.begin(), i->second->itemsUsedList.end());
}
int PlayerStats::getTotalRoundsWon()
{
    int total = 0; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) if (i->second->won) ++total;
    return total;
}
unsigned char PlayerStats::getRoundWon(unsigned int round) { boost::unordered_map<int, RoundStats*>::iterator i = mRoundStats.find((int)round); return i != mRoundStats.end() && i->second->won; }
int PlayerStats::getTotalBlocksDestroyed()
{
    int total = 0; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) total += (int)i->second->blocksDestroyed.placed.size();
    return total;
}
int PlayerStats::getTotalSplashPotionsUsed()
{
    int total = 0; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) total += i->second->splashPotions;
    return total;
}
int PlayerStats::getTotalPotionsUsed()
{
    int total = 0; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) total += i->second->potions;
    return total;
}
int PlayerStats::getTotalLifeSpan()
{
    int total = 0; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) total += i->second->mTimeSpent[0];
    return total;
}
int PlayerStats::getLifeSpan(unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator i = mRoundStats.find((int)round);
    return i == mRoundStats.end() ? 0 : i->second->mTimeSpent[0];
}
int PlayerStats::getBestLifeSpan(bool highest, bool reachedEndOnly)
{
    int result = highest ? 0 : 0x7fffffff; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) {
        if (!reachedEndOnly || i->second->reachedEnd) result = highest ? std::max(result, i->second->mTimeSpent[0]) : std::min(result, i->second->mTimeSpent[0]);
    }
    return result;
}
int PlayerStats::getTotalPoints()
{
    int total = 0; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) total += i->second->points;
    return total;
}
int PlayerStats::getPoints(unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator i = mRoundStats.find((int)round);
    return i == mRoundStats.end() ? 0 : i->second->points;
}
int PlayerStats::getBestPoints(bool highest, bool reachedEndOnly)
{
    int result = highest ? 0 : 0x7fffffff; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) {
        if (!reachedEndOnly || i->second->reachedEnd) result = highest ? std::max(result, i->second->points) : std::min(result, i->second->points);
    }
    return result;
}
unsigned char PlayerStats::getReachedEnd(unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator i = mRoundStats.find((int)round);
    return i != mRoundStats.end() && i->second->reachedEnd;
}
int PlayerStats::getTotalThermalsEntered()
{
    int total = 0; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) total += i->second->thermals;
    return total;
}
bool PlayerStats::hasSpawnedAtCheckpoint(int checkpoint, unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator i = mRoundStats.find((int)round);
    return i != mRoundStats.end() && i->second->itemCounts.find(checkpoint) != i->second->itemCounts.end();
}
bool PlayerStats::hasSpawnedAtAllCheckpointsInOneRound() { return hasSpawnedAtAllCheckpoints(); }
double PlayerStats::getTopSpeed()
{
    double result = 0.0; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) result = std::max(result, i->second->topSpeed);
    return result;
}
int PlayerStats::getImprovement() { return 0; }
bool PlayerStats::hasRoundStats(unsigned int round) const { return mRoundStats.find((int)round) != mRoundStats.end(); }
int PlayerStats::getTotalSecondaryTargets()
{
    int total = 0; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) total += i->second->secondaryTargets;
    return total;
}
void PlayerStats::getAllContainers(std::vector<ContainerInfo*>& containers) const
{
    containers.clear(); boost::unordered_map<int, RoundStats*>::const_iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) {
        containers.insert(containers.end(), i->second->containers.begin(), i->second->containers.end());
    }
}
int PlayerStats::GetLastPointsAdded(unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator i = mRoundStats.find((int)round);
    return i == mRoundStats.end() ? 0 : i->second->lastPointsAdded;
}
int PlayerStats::getTotalRoundsReachedEnd()
{
    int total = 0; boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) if (i->second->reachedEnd) ++total;
    return total;
}
bool PlayerStats::hasSpawnedAtCheckpoint(int checkpoint)
{
    boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) {
        if (i->second->itemCounts.find(checkpoint) != i->second->itemCounts.end()) return true;
    }
    return false;
}
bool PlayerStats::hasSpawnedAtAllCheckpoints()
{
    if (mRoundStats.empty()) return false;
    boost::unordered_map<int, RoundStats*>::iterator i;
    for (i = mRoundStats.begin(); i != mRoundStats.end(); ++i) {
        if (i->second->itemCounts.empty()) return false;
    }
    return true;
}
