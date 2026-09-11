#include "game/core/GameStats.hpp"

#include <algorithm>
#include <climits>
#include <cmath>
#include <map>

#include <boost/shared_ptr.hpp>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CombatEntry.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/DamageEvent.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/Random.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/core/StatTracker.hpp"
#include "game/core/System.hpp"
#include "game/core/Voteable.hpp"
#include "game/core/Voteables.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/minigame/CombinationDefinitionRuleDefinition.hpp"
#include "game/minigame/GameMedal.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/server/ServerScoreboard.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Math.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/LastPlayerFatalDamageStatTracker.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/generation/PlayerStats.hpp"

extern std::vector<const GameMedal*> gBattleMedals;
extern std::vector<const GameMedal*> gTumbleMedals;
extern std::vector<const GameMedal*> gGlideMedals;
extern std::vector<const GameMedal*> gBuildMedals;

GameStats::GameStats()
    : mPlayerCount(0), mRoundStartTime(0), mCurrentRound(0), mRoundEndTime(0), mRoundTime(0), mRoundWinner(), mFirstPlace(), mSecondPlace(), mThirdPlace(), mFarthestCheckpointReached(0), mLandingRecorded(false), mWinningThemeChoice()
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
    return mFirstPlace;
}

unsigned int GameStats::GetShortestLifespan(unsigned int round) const
{
    if (mPlayerStats.empty())
        return 0x7fffffffU;

    unsigned int shortest = 0x7fffffffU;
    for (StatsMap::const_iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second == 0)
            continue;
        const unsigned int lifespan = static_cast<unsigned int>(it->second->getLifeSpan(round));
        if (lifespan < shortest)
            shortest = lifespan;
    }
    return shortest;
}

GameStats::StatsMap* GameStats::GetPlayerStats()
{
    return &mPlayerStats;
}

unsigned int GameStats::GetDeathCount(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return 0;
    return static_cast<unsigned int>(stats->getRoundStats(mCurrentRound)->mDeaths);
}

void GameStats::RecordBlockPresent(const StatsUID& uid, const BlockPos& pos, int state)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;

    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    PlayerStats::RoundStats::BlockInteractionInfo& info = round->mBlocksUsed;
    ++info.mCount;
    ++info.mItems[state];
    ++info.mPlayers[pos.y - round->mLayer];
    const double elapsed = static_cast<double>(System::processTimeInMilliSecs() - mRoundStartTime);
    ++info.mRounds[static_cast<int>(elapsed / 10000.0)];
}

unsigned int GameStats::GetRoundTime()
{
    if (mRoundStartTime == 0)
        return 0;
    unsigned long long now = static_cast<unsigned long long>(System::processTimeInMilliSecs());
    return static_cast<unsigned int>((now - mRoundStartTime) / 1000);
}

void GameStats::AwardMG01Medals(const std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    if (players == 0)
        return;

    PlayerStats* best = 0;
    bool tied = false;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
         it != players->end(); ++it) {
        if (!*it)
            continue;
        PlayerStats* stats = GetPlayerStats((*it)->getSUID());
        if (stats == 0 || !stats->hasRoundStats(mCurrentRound))
            continue;
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        if (round == 0)
            continue;
        round->mActive = true;
        if (best == 0 || round->mKills > best->getRoundStats(mCurrentRound)->mKills) {
            best = stats;
            tied = false;
        } else if (round->mKills == best->getRoundStats(mCurrentRound)->mKills) {
            tied = true;
        }
    }
    if (best != 0 && !tied)
        best->getRoundStats(mCurrentRound)->mWon = true;
}
bool GameStats::SORT_MG02_STATS(PlayerStats* first, PlayerStats* second)
{
    if (second->getTotalRoundsWon() < first->getTotalRoundsWon())
        return true;
    if (first->getTotalRoundsWon() == second->getTotalRoundsWon()
        && second->getTotalBlocksDestroyed() < first->getTotalBlocksDestroyed())
        return true;
    return false;
}

namespace
{
struct ContainerInfoLayout
{
    int mX;
    int mY;
    int mZ;
    int mType;
};

unsigned int mg03SortCurrentRound()
{
    if (gMinecraftServer == 0 || gMinecraftServer->mMasterGameMode == 0)
        return 0;
    return *reinterpret_cast<unsigned int*>(reinterpret_cast<char*>(gMinecraftServer->mMasterGameMode) + 0x178);
}

CommonMasterGameMode* masterGameMode()
{
    if (gMinecraftServer == 0)
        return 0;
    return reinterpret_cast<CommonMasterGameMode*>(gMinecraftServer->mMasterGameMode);
}

PlayerList* serverPlayers()
{
    if (gMinecraftServer == 0)
        return 0;
    return gMinecraftServer->getPlayers();
}

DamageEvent* getFatalDamageEvent(PlayerStats::RoundStats* round)
{
    if (round == 0 || round->mLastPlayerFatalDamage == 0)
        return 0;
    return *reinterpret_cast<DamageEvent**>(
        reinterpret_cast<char*>(round->mLastPlayerFatalDamage) + 0x18);
}

ContainerInfo* allocateContainerInfo(int type, int x, int y, int z)
{
    ContainerInfoLayout* info = new ContainerInfoLayout();
    info->mX = x;
    info->mY = y;
    info->mZ = z;
    info->mType = type;
    return reinterpret_cast<ContainerInfo*>(info);
}

struct DamageEventLayout
{
    StatsUID mVictim;
    StatsUID mAttacker;
    boost::shared_ptr<Entity> mAttackerEntity;
    float mDistance;
    bool mFatal;
    Scoreboard::EDamageSource mDamageSource;
    float mDamage;
    Scoreboard::Weapon mWeapon;
    bool mInWater;
    float mFallDistance;
    int mFallCount;
    int mTime;
};

DamageEvent* asDamageEvent(DamageEventLayout* layout)
{
    return reinterpret_cast<DamageEvent*>(layout);
}

Scoreboard::Weapon weaponFromCombatEntry(CombatEntry* entry)
{
    const DamageSource* source = entry->getSource();
    Scoreboard::EDamageSource damageSource = Scoreboard::GetDamageSource(source);
    if (damageSource == Scoreboard::DAMAGE_SOURCE_MAGIC)
    {
        Potion* potion = Scoreboard::GetPotionByDamageSource(damageSource);
        return Scoreboard::Weapon(Item::getId(Items::potion), potion);
    }

    boost::shared_ptr<LivingEntity> living =
        boost::dynamic_pointer_cast<LivingEntity>(source->getEntity());
    if (living)
    {
        not_null_ptr<ItemInstance> item = living->getMainHandItem();
        if (item.get() != 0 && item->getItem() != 0)
            return Scoreboard::Weapon(Item::getId(item->getItem()));
    }

    return Scoreboard::Weapon();
}

boost::shared_ptr<Entity> resolveAttackerFromPacket(
    boost::shared_ptr<ClientboundChatPacket> packet, PlayerList* players)
{
    if (!packet || !players)
        return boost::shared_ptr<Entity>();

    if (packet->getMessage() == ClientboundChatPacket::eChatPacketMessage_Text)
        return boost::shared_ptr<Entity>();

    const std::wstring& killerName = packet->getMobCustomName(0);
    if (killerName.empty())
        return boost::shared_ptr<Entity>();

    boost::shared_ptr<ServerPlayer> named = players->getPlayerByName(killerName);
    return named;
}

int countChestRules(GameRuleDefinition* rule)
{
    int total = 0;
    CombinationDefinitionRuleDefinition* combination =
        dynamic_cast<CombinationDefinitionRuleDefinition*>(rule);
    if (combination == 0)
        return 0;

    std::vector<GameRuleDefinition*> children;
    combination->getChildren(&children);
    for (std::vector<GameRuleDefinition*>::iterator it = children.begin(); it != children.end(); ++it)
    {
        if (*it != 0
            && dynamic_cast<CombinationDefinitionRuleDefinition*>(*it) != 0)
        {
            std::vector<GameRuleDefinition*> nested;
            (*it)->getChildren(&nested);
            total += static_cast<int>(nested.size());
        }
    }
    return total;
}
}

boost::shared_ptr<ServerPlayer> GameStats::getPlayerFromStatsVec(const std::vector<PlayerStats*>& stats, unsigned int index)
{
    if (stats.empty() || index >= stats.size())
        return boost::shared_ptr<ServerPlayer>();

    PlayerStats* playerStats = stats[index];
    if (playerStats == 0 || gMinecraftServer == 0 || gMinecraftServer->getPlayers() == 0)
        return boost::shared_ptr<ServerPlayer>();

    return gMinecraftServer->getPlayers()->getPlayerFromStatsUID(*reinterpret_cast<StatsUID*>(playerStats));
}

void GameStats::AwardMG02Medals(const std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    mRoundWinner.reset();

    if (players == 0)
        return;

    const unsigned int livesPerRound = CommonMasterGameMode::GetMiniGame()->GetLivesPerRound();
    int bestDeaths = INT_MAX;
    std::vector<boost::shared_ptr<ServerPlayer> > activePlayers;
    std::vector<boost::shared_ptr<ServerPlayer> > lowestDeathPlayers;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
         it != players->end(); ++it)
    {
        if (!*it)
            continue;

        PlayerStats* stats = GetPlayerStats((*it)->getSUID());
        if (stats == 0)
            continue;

        int remainingLives = INT_MAX;
        if (livesPerRound > 0)
            remainingLives = static_cast<int>(livesPerRound) - static_cast<int>(stats->getDeaths(mCurrentRound));

        if ((!(*it)->isAlive() && remainingLives <= 0) || (*it)->isSpectator())
            continue;

        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        activePlayers.push_back(*it);

        if (round->mDeaths <= bestDeaths)
        {
            if (round->mDeaths < bestDeaths)
            {
                bestDeaths = round->mDeaths;
                lowestDeathPlayers.clear();
            }
            lowestDeathPlayers.push_back(*it);
        }

        if (round->mRoundTime == 0)
            round->mRoundTime = static_cast<int>(GetRoundTime()) - round->mDeaths * 4;
    }

    if (activePlayers.size() == 1)
        mRoundWinner = activePlayers.front();
    else if (lowestDeathPlayers.size() == 1)
        mRoundWinner = lowestDeathPlayers.front();

    if (mRoundWinner)
    {
        PlayerStats* winnerStats = GetPlayerStats(mRoundWinner->getSUID());
        if (winnerStats != 0)
        {
            PlayerStats::RoundStats* winnerRound = winnerStats->getRoundStats(mCurrentRound);
            winnerRound->mActive = true;
            if (!mLandingRecorded)
                winnerRound->mWon = true;
            else
            {
                mRoundWinner.reset();
                mLandingRecorded = false;
            }
        }
    }

    std::vector<PlayerStats*> statsVec;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
         it != players->end(); ++it)
    {
        if (!*it)
            continue;
        PlayerStats* stats = GetPlayerStats((*it)->getSUID());
        if (stats != 0)
            statsVec.push_back(stats);
    }

    if (statsVec.empty())
        return;

    std::sort(statsVec.begin(), statsVec.end(), SORT_MG02_STATS);
    mFirstPlace = getPlayerFromStatsVec(statsVec, 0);
    mSecondPlace = getPlayerFromStatsVec(statsVec, 1);
    mThirdPlace = getPlayerFromStatsVec(statsVec, 2);
}

bool GameStats::SORT_MG03_STATS_FALLBACK(PlayerStats* first, PlayerStats* second)
{
    if (first->getBestLifeSpan(0, 0) != second->getBestLifeSpan(0, 0))
        return first->getBestLifeSpan(0, 0) < second->getBestLifeSpan(0, 0);
    if (first->getBestPoints(1, 0) != second->getBestPoints(1, 0))
        return second->getBestPoints(1, 0) < first->getBestPoints(1, 0);
    if (first->getTotalRoundsWon() != second->getTotalRoundsWon())
        return second->getTotalRoundsWon() < first->getTotalRoundsWon();
    if (first->getTotalDeaths() != second->getTotalDeaths())
        return first->getTotalDeaths() < second->getTotalDeaths();
    return first < second;
}

bool GameStats::SORT_MG03_STATS_POINTS(PlayerStats* first, PlayerStats* second)
{
    if (first->getBestPoints(1, 1) != second->getBestPoints(1, 1))
        return second->getBestPoints(1, 1) < first->getBestPoints(1, 1);
    return SORT_MG03_STATS_FALLBACK(first, second);
}

bool GameStats::SORT_MG03_STATS_TIME(PlayerStats* first, PlayerStats* second)
{
    if (first->getBestLifeSpan(0, 1) != second->getBestLifeSpan(0, 1))
        return first->getBestLifeSpan(0, 1) < second->getBestLifeSpan(0, 1);
    return SORT_MG03_STATS_FALLBACK(first, second);
}

bool GameStats::SORT_MG03_STATS_POINTS_CURRENT_ROUND(PlayerStats* first, PlayerStats* second)
{
    const unsigned int round = mg03SortCurrentRound();
    if (first->getReachedEnd(round) != second->getReachedEnd(round))
        return first->getReachedEnd(round) != 0;
    if (first->getPoints(round) != second->getPoints(round))
        return second->getPoints(round) < first->getPoints(round);
    return SORT_MG03_STATS_FALLBACK(first, second);
}

bool GameStats::SORT_MG03_STATS_TIME_CURRENT_ROUND(PlayerStats* first, PlayerStats* second)
{
    const unsigned int round = mg03SortCurrentRound();
    if (first->getReachedEnd(round) != second->getReachedEnd(round))
        return first->getReachedEnd(round) != 0;
    if (first->getLifeSpan(round) != second->getLifeSpan(round))
        return first->getLifeSpan(round) < second->getLifeSpan(round);
    return SORT_MG03_STATS_FALLBACK(first, second);
}

void GameStats::AwardMG03Medals(const std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    mRoundWinner.reset();

    if (players == 0)
        return;

    std::vector<PlayerStats*> statsVec;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
         it != players->end(); ++it)
    {
        if (!*it)
            continue;
        PlayerStats* stats = GetPlayerStats((*it)->getSUID());
        if (stats != 0)
            statsVec.push_back(stats);
    }

    if (statsVec.empty())
        return;

    const unsigned int variant = Minecraft::GetInstance()->GetMiniGame()->GetGameVariant();
    if (variant == 1)
        std::sort(statsVec.begin(), statsVec.end(), SORT_MG03_STATS_POINTS);
    else
        std::sort(statsVec.begin(), statsVec.end(), SORT_MG03_STATS_TIME);

    mFirstPlace = getPlayerFromStatsVec(statsVec, 0);
    mSecondPlace = getPlayerFromStatsVec(statsVec, 1);
    mThirdPlace = getPlayerFromStatsVec(statsVec, 2);
    mRoundWinner = mFirstPlace;

    PlayerStats::RoundStats* winnerRound = statsVec[0]->getRoundStats(mCurrentRound);
    if (winnerRound != 0)
        winnerRound->mActive = true;

    if (variant == 1)
        std::sort(statsVec.begin(), statsVec.end(), SORT_MG03_STATS_POINTS_CURRENT_ROUND);
    else
        std::sort(statsVec.begin(), statsVec.end(), SORT_MG03_STATS_TIME_CURRENT_ROUND);

    const int playerCount = static_cast<int>(statsVec.size());
    for (int i = 0; i < playerCount; ++i)
    {
        PlayerStats::RoundStats* round = statsVec[i]->getRoundStats(mCurrentRound);
        if (round != 0)
        {
            round->mCheckpointPadding[0] = i;
            round->mCheckpointPadding[1] = playerCount;
        }
    }
}
void GameStats::AwardMG04Medals(const std::vector<boost::shared_ptr<ServerPlayer> >*)
{
    mRoundWinner.reset();
    mFirstPlace.reset();
    mSecondPlace.reset();
    mThirdPlace.reset();
}
void GameStats::CalculateStats(const std::vector<boost::shared_ptr<ServerPlayer> >* players, bool newRound)
{
    if (players == 0)
        return;

    const long long now = System::processTimeInMilliSecs();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
         it != players->end(); ++it)
    {
        if (!*it)
            continue;
        PlayerStats* stats = GetPlayerStats((*it)->getSUID());
        if (stats == 0)
            continue;
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        for (int category = 0; category < 6; ++category)
            round->MedalTimestamp(static_cast<eTimeSpentCategory>(category), false, now);
    }

    CommonMasterGameMode* mode = masterGameMode();
    if (Minecraft::InMiniGame(eMiniGameId_Build, true) && mode != 0)
    {
        Voteables* voteables = *reinterpret_cast<Voteables**>(reinterpret_cast<char*>(mode) + 0xC0);
        Voteable* buildVoteable = voteables != 0 ? voteables->getVoteable(2) : 0;
        if (buildVoteable != 0)
        {
            for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
                 it != players->end(); ++it)
            {
                if (!*it)
                    continue;
                PlayerStats* stats = GetPlayerStats((*it)->getSUID());
                if (stats == 0)
                    continue;
                PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
                unsigned short* votesFrom = reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(round) + 0x24C);
                unsigned short* votesTo = reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(round) + 0x254);
                votesFrom[0] = 0;
                votesFrom[1] = 0;
                votesTo[0] = 0;
                votesTo[1] = 0;

                const int votePlayerId = (*it)->getId();
                std::map<int, int> fromVotes = buildVoteable->getVotesFromPlayer(votePlayerId);
                for (std::map<int, int>::iterator vote = fromVotes.begin(); vote != fromVotes.end(); ++vote)
                    ++votesFrom[vote->first];
                std::map<int, int> toVotes = buildVoteable->getVotesToVoteable(votePlayerId);
                for (std::map<int, int>::iterator vote = toVotes.begin(); vote != toVotes.end(); ++vote)
                    ++votesTo[vote->first];
            }
        }

        if (gMinecraftServer != 0 && gMinecraftServer->getLevel(0) != 0)
        {
            ServerLevel* level = gMinecraftServer->getLevel(0);
            ServerScoreboard* scoreboard = level->GetServerScoreboard();
            if (scoreboard != 0)
            {
                std::vector<PlayerTeam*>* teams = scoreboard->getPlayerTeams();
                if (teams != 0)
                {
                    for (std::vector<PlayerTeam*>::iterator teamIt = teams->begin();
                         teamIt != teams->end(); ++teamIt)
                    {
                        AABB* area = mode->GetTeamArea(*teamIt);
                        if (area == 0)
                            continue;
                        for (int x = static_cast<int>(area->mMinX); x < static_cast<int>(area->mMaxX); ++x)
                        {
                            for (int y = static_cast<int>(area->mMinY); y < static_cast<int>(area->mMaxY); ++y)
                            {
                                for (int z = static_cast<int>(area->mMinZ); z < static_cast<int>(area->mMaxZ); ++z)
                                {
                                    BlockPos pos(x, y, z);
                                    const int state = Block::getIdWithData(level->getBlockState(pos));
                                    if (state == 0)
                                        continue;
                                    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
                                         it != players->end(); ++it)
                                    {
                                        if (!*it)
                                            continue;
                                        RecordBlockPresent((*it)->getSUID(), pos, state);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (Minecraft::InMiniGame(eMiniGameId_Build, true) && mode != 0)
    {
        Voteables* voteables = *reinterpret_cast<Voteables**>(reinterpret_cast<char*>(mode) + 0xC0);
        Voteable* themeVoteable = voteables != 0 ? voteables->getVoteable(3) : 0;
        if (themeVoteable != 0)
        {
            int winningVoteTotal = -1;
            themeVoteable->getWinningVote(winningVoteTotal);
            mWinningThemeChoice = mode->getThemeWordEnglish(
                *reinterpret_cast<int*>(reinterpret_cast<char*>(mode) + 0xBC));

            for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
                 it != players->end(); ++it)
            {
                if (!*it)
                    continue;
                PlayerStats* stats = GetPlayerStats((*it)->getSUID());
                if (stats == 0)
                    continue;
                int onlyVote = -1;
                themeVoteable->getOnlyVote((*it)->getId(), &onlyVote);
                stats->getRoundStats(mCurrentRound)->mWinningTheme = mode->getThemeWordEnglish(onlyVote);
            }
        }
    }

    if (!newRound)
        mRoundEndTime = static_cast<unsigned int>(now);

    std::vector<const GameMedal*> medalPool;
    const unsigned int miniGameId = Minecraft::GetInstance()->GetMiniGame()->GetId();
    if (miniGameId == 1U)
    {
        AwardMG01Medals(players);
        medalPool = gBattleMedals;
    }
    else if (miniGameId == 2U)
    {
        AwardMG02Medals(players);
        medalPool = gTumbleMedals;
    }
    else if (miniGameId == 3U)
    {
        AwardMG03Medals(players);
        medalPool = gGlideMedals;
    }
    else if (miniGameId == 4U)
    {
        AwardMG04Medals(players);
        medalPool = gBuildMedals;
    }

    if (Minecraft::InMiniGame(eMiniGameId_Build, true))
    {
        const int roundSeconds = static_cast<int>(std::floor(static_cast<double>(now - mRoundStartTime) / 1000.0));
        for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
             it != players->end(); ++it)
        {
            if (!*it)
                continue;
            PlayerStats* stats = GetPlayerStats((*it)->getSUID());
            if (stats != 0)
                stats->getRoundStats(mCurrentRound)->mRoundTime = roundSeconds;
        }
    }

    if (mRoundWinner)
    {
        PlayerStats* winnerStats = GetPlayerStats(mRoundWinner->getSUID());
        if (winnerStats != 0 && mode != 0)
        {
            const double y = mRoundWinner->mY;
            winnerStats->getRoundStats(mCurrentRound)->mLayer = mode->GetLayer(y);
        }
    }

    if (Minecraft::InMiniGame(eMiniGameId_Tumble, true) && mRoundWinner)
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        PlayerStats* winnerStats = GetPlayerStats(mRoundWinner->getSUID());
        if (winnerStats != 0 && miniGame->HasFeatureSnowballAwards())
        {
            PlayerStats::RoundStats* winnerRound = winnerStats->getRoundStats(mCurrentRound);
            if (newRound)
            {
                unsigned int consecutiveWins = 0;
                int round = static_cast<int>(mCurrentRound);
                while (round >= 0 && winnerStats->getRoundStats(static_cast<unsigned int>(round))->mWon)
                {
                    ++consecutiveWins;
                    --round;
                }

                unsigned int bestRivalWins = 0;
                for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
                     it != players->end(); ++it)
                {
                    if (!*it || *it == mRoundWinner)
                        continue;
                    PlayerStats* stats = GetPlayerStats((*it)->getSUID());
                    if (stats != 0)
                        bestRivalWins = std::max(bestRivalWins,
                                                   static_cast<unsigned int>(stats->getTotalRoundsWon()));
                }

                if (consecutiveWins > 2U && bestRivalWins > 1U)
                {
                    mRoundWinner->awardStat(GenericStats::backFromTheDead(),
                                            GenericStats::param_backFromTheDead());
                }
            }

            if (winnerRound->mLayer == 0)
                mRoundWinner->awardStat(GenericStats::mountainTop(), GenericStats::param_mountainTop());
            else if (winnerRound->mLayer == 2)
                mRoundWinner->awardStat(GenericStats::willLikeNoOther(),
                                        GenericStats::param_willLikeNoOther());
            if (winnerRound->mItemsUsed.empty())
                mRoundWinner->awardStat(GenericStats::equipYourFists(),
                                        GenericStats::param_equipYourFists());
        }
    }

    if (newRound)
    {
        Random random;
        for (std::vector<const GameMedal*>::const_iterator medalIt = medalPool.begin();
             medalIt != medalPool.end(); ++medalIt)
        {
            const GameMedal* medal = *medalIt;
            if (medal == 0)
                continue;

            if (medal->BestOnly())
            {
                const int bestIndex = medal->FindBest(players, this);
                if (bestIndex < 0)
                    continue;
                boost::shared_ptr<ServerPlayer> best = players->at(static_cast<std::size_t>(bestIndex));
                PlayerStats* stats = GetPlayerStats(best->getSUID());
                if (stats != 0)
                    stats->mMedals.push_back(medal);
                continue;
            }

            for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
                 it != players->end(); ++it)
            {
                if (!*it)
                    continue;
                if (!medal->CanAward((*it)->getSUID(), this))
                    continue;
                PlayerStats* stats = GetPlayerStats((*it)->getSUID());
                if (stats != 0)
                    stats->mMedals.push_back(medal);
            }
        }

        for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
             it != players->end(); ++it)
        {
            if (!*it)
                continue;
            PlayerStats* stats = GetPlayerStats((*it)->getSUID());
            if (stats == 0)
                continue;

            int totalWeight = 0;
            for (std::vector<const GameMedal*>::iterator medalIt = stats->mMedals.begin();
                 medalIt != stats->mMedals.end(); ++medalIt)
                totalWeight += static_cast<int>((*medalIt)->GetWeight());
            stats->mLastMedal = totalWeight;

            while (!stats->mMedals.empty() && stats->mWinningMedal == 0)
            {
                int pick = random.nextInt(totalWeight);
                for (std::vector<const GameMedal*>::iterator medalIt = stats->mMedals.begin();
                     medalIt != stats->mMedals.end(); ++medalIt)
                {
                    pick -= static_cast<int>((*medalIt)->GetWeight());
                    if (pick >= 0)
                        continue;
                    const GameMedal* chosen = *medalIt;
                    stats->mWinningMedal = reinterpret_cast<int>(chosen);
                    if (!chosen->CanMultiAward())
                    {
                        medalIt = stats->mMedals.erase(medalIt);
                        for (std::vector<const GameMedal*>::iterator poolIt = medalPool.begin();
                             poolIt != medalPool.end(); ++poolIt)
                        {
                            if (*poolIt == chosen)
                            {
                                medalPool.erase(poolIt);
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
         it != players->end(); ++it)
    {
        if (!*it)
            continue;
        PlayerStats* stats = GetPlayerStats((*it)->getSUID());
        if (stats == 0)
            continue;
        if ((*it)->getLevel() == 0)
            continue;
        ServerScoreboard* scoreboard = (*it)->getLevel()->GetServerScoreboard();
        if (scoreboard != 0)
            scoreboard->OnPlayerStatsChanged((*it)->getSUID(), static_cast<int>(GetDeathCount((*it)->getSUID())));
    }
}

void GameStats::AddPlayerStats(const boost::shared_ptr<ServerPlayer>& player)
{
    if (!player)
        return;

    const StatsUID uid = player->getSUID();
    if (mPlayerStats.find(uid) != mPlayerStats.end())
        return;

    PlayerStats* stats = new PlayerStats(uid);
    stats->getRoundStats(mCurrentRound);
    stats->mName.assign(player->getDisplayName());
    mPlayerStats[uid] = stats;
}
void GameStats::AddPlayers(const std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    if (players != 0)
        for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin(); it != players->end(); ++it)
            AddPlayerStats(*it);
}
void GameStats::RemovePlayerStats(const boost::shared_ptr<ServerPlayer>&) { if (mPlayerCount != 0) --mPlayerCount; }
void GameStats::Clear() { mPlayerStats.clear(); mPlayerCount = 0; mRoundTime = 0; mFarthestCheckpointReached = 0; mLandingRecorded = false; }
void GameStats::OnRoundStart() { mRoundStartTime = static_cast<unsigned int>(System::processTimeInMilliSecs()); mLandingRecorded = false; }
void GameStats::OnRoundEnd() { mRoundEndTime = static_cast<unsigned int>(System::processTimeInMilliSecs()); }
void GameStats::OnGameStart() { Clear(); OnRoundStart(); }
void GameStats::OnGameEnd() { mRoundEndTime = static_cast<unsigned int>(System::processTimeInMilliSecs()); }

int GameStats::GetDamageTakenThisRound(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : static_cast<int>(stats->getRoundStats(mCurrentRound)->mDamageReceived->getValue());
}
unsigned int GameStats::GetKillsWithBowThisRound(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : static_cast<unsigned int>(stats->getRoundStats(mCurrentRound)->mKillsWithBow->getValue());
}
unsigned int GameStats::GetMaxSnowballHitsOnAnyPlayer(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : stats->getRoundStats(mCurrentRound)->mMaxSnowballHits->getValue();
}
void GameStats::RecordDamage(boost::shared_ptr<Entity> entity, CombatEntry* entry,
                             boost::shared_ptr<ClientboundChatPacket> packet)
{
    if (!entity || !entry)
        return;

    boost::shared_ptr<Entity> attacker = entry->getSource()->getEntity();
    if (attacker == 0 && packet)
        attacker = resolveAttackerFromPacket(packet, serverPlayers());

    DamageEventLayout layout = {
        StatsUID::EMPTY_SUID(),
        StatsUID::EMPTY_SUID(),
        attacker,
        0.0f,
        entry->getHealthAfterDamage() <= 0.0f,
        Scoreboard::GetDamageSource(entry->getSource()),
        entry->getDamage(),
        weaponFromCombatEntry(entry),
        false,
        entry->getFallDistance(),
        0,
        entry->getTime(),
    };

    if (Entity::instanceof(entity, ENTITY_TYPE_SERVER_PLAYER))
    {
        boost::shared_ptr<ServerPlayer> victim = boost::dynamic_pointer_cast<ServerPlayer>(entity);
        if (victim)
        {
            layout.mVictim = victim->getSUID();
            layout.mDistance = attacker ? static_cast<float>(entity->distanceTo(attacker)) : 0.0f;
            if (attacker)
                layout.mInWater = attacker->isInWater();

            PlayerStats* stats = GetPlayerStats(victim->getSUID());
            if (stats != 0)
                stats->updateTrackedStats(static_cast<int>(mCurrentRound), asDamageEvent(&layout));
        }
    }

    if (attacker && attacker.get() != entity.get()
        && Entity::instanceof(attacker, ENTITY_TYPE_SERVER_PLAYER))
    {
        boost::shared_ptr<ServerPlayer> attackerPlayer = boost::dynamic_pointer_cast<ServerPlayer>(attacker);
        if (attackerPlayer)
        {
            layout.mAttacker = attackerPlayer->getSUID();
            PlayerStats* stats = GetPlayerStats(attackerPlayer->getSUID());
            if (stats != 0)
                stats->updateTrackedStats(static_cast<int>(mCurrentRound), asDamageEvent(&layout));
        }
    }

    if (!Minecraft::InMiniGame(eMiniGameId_Battle, true) || !layout.mFatal)
        return;

    boost::shared_ptr<ServerPlayer> victimPlayer = boost::dynamic_pointer_cast<ServerPlayer>(entity);
    boost::shared_ptr<ServerPlayer> attackerPlayer = boost::dynamic_pointer_cast<ServerPlayer>(attacker);
    if (victimPlayer && attackerPlayer && victimPlayer->mFoodData.isStarving()
        && victimPlayer.get() != attackerPlayer.get())
    {
        attackerPlayer->awardStat(GenericStats::youWontLikeMeWhenImHungry(),
                                  GenericStats::param_youWontLikeMeWhenImHungry());
    }

    if (!CGameNetworkManager::sInstance.IsInPublicJoinableGame())
        return;

    if (victimPlayer)
    {
        const unsigned int damageTaken = GetDamageTakenThisRound(victimPlayer->getSUID());
        if (damageTaken > 99U
            && static_cast<float>(damageTaken) - layout.mDamage < 100.0f)
        {
            victimPlayer->awardStat(GenericStats::tisButAScratch(),
                                    GenericStats::param_tisButAScratch());
        }

        PlayerStats* victimStats = GetPlayerStats(victimPlayer->getSUID());
        if (victimStats != 0 && victimStats->mDeadshot != 0
            && static_cast<StatTracker<bool>*>(victimStats->mDeadshot)->getValue())
        {
            victimPlayer->awardStat(GenericStats::deadshot(), GenericStats::param_deadshot());
        }
    }

    if (attackerPlayer)
    {
        if (GetKillsWithBowThisRound(attackerPlayer->getSUID()) == 2U
            && Minecraft::InMiniGame(eMiniGameId_Battle, true))
        {
            attackerPlayer->awardStat(GenericStats::cupid(), GenericStats::param_cupid());
        }

        if (GetMaxSnowballHitsOnAnyPlayer(attackerPlayer->getSUID()) == 25U
            && Minecraft::InMiniGame(eMiniGameId_Tumble, true))
        {
            attackerPlayer->awardStat(GenericStats::tenaciousBall(),
                                      GenericStats::param_tenaciousBall(25));
        }
    }
}
unsigned int GameStats::GetPlayerFatalDamage(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == 0)
        return 0U;
    DamageEvent* event = getFatalDamageEvent(stats->getRoundStats(mCurrentRound));
    return reinterpret_cast<unsigned int>(event);
}
void GameStats::RecordDeath(boost::shared_ptr<Entity> entity)
{
    boost::shared_ptr<ServerPlayer> player = boost::dynamic_pointer_cast<ServerPlayer>(entity);
    if (!player)
        return;

    const StatsUID uid = player->getSUID();
    const unsigned int deathCount = GetDeathCount(uid);
    ++mPlayerCount;

    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == 0)
        return;

    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    round->getLifeStats(static_cast<int>(deathCount));
    DamageEvent* fatalDamage = getFatalDamageEvent(round);

    const MiniGameDef* miniGame = CommonMasterGameMode::GetMiniGame();
    if (miniGame->GetId() != static_cast<unsigned int>(eMiniGameId_Glide)
        && miniGame->GetLivesPerRound() == mPlayerCount)
    {
        const double elapsedMs = static_cast<double>(System::processTimeInMilliSecs() - mRoundStartTime);
        round->mRoundTime = static_cast<int>(std::floor(elapsedMs / 1000.0)) - static_cast<int>(mCurrentRound) * 4;
    }

    ++round->mDeaths;

    CommonMasterGameMode* mode = masterGameMode();
    if (mode != 0 && mode->GetLayerCount() > 1)
    {
        if (player->mOnGround)
            round->mLayer = -1;
        else
            round->mLayer = mode->GetLayer(player->mY);
    }

    round->mLives = mPlayerCount;

    if (fatalDamage != 0 && fatalDamage->mAttackerEntity)
    {
        boost::shared_ptr<ServerPlayer> killer =
            boost::dynamic_pointer_cast<ServerPlayer>(fatalDamage->mAttackerEntity);
        if (killer && killer.get() != player.get())
        {
            round->mWeapon = fatalDamage->mWeapon;
            round->mMedalNames.push_back(killer->getDisplayName());

            PlayerStats* killerStats = GetPlayerStats(killer->getSUID());
            if (killerStats != 0)
            {
                ++killerStats->getRoundStats(mCurrentRound)->mKills;
                if (Minecraft::InMiniGame(eMiniGameId_Tumble, true)
                    && killerStats->getTotalKills() == 3)
                {
                    killer->awardStat(GenericStats::suckMySnowballs(),
                                      GenericStats::param_suckMySnowballs(3));
                }
            }
        }
        else if (killer)
        {
            round->mWeapon = fatalDamage->mWeapon;
        }
    }

    if (player->getLevel() != 0)
    {
        ServerScoreboard* scoreboard = player->getLevel()->GetServerScoreboard();
        if (scoreboard != 0)
            scoreboard->OnPlayerStatsChanged(uid, static_cast<int>(deathCount));
    }
}
void GameStats::RecordJump(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) ++stats->getRoundStats(mCurrentRound)->mJumpCount;
}
void GameStats::RecordItem(const StatsUID& uid, not_null_ptr<ItemInstance> item)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;

    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    ItemInstance* instance = item.get();
    if (instance == NULL)
        return;

    Item* itemType = instance->getItem();
    const int itemId = Item::VALIDgetId(itemType);
    std::map<int, int>::iterator it = round->mItems.find(itemId);
    if (it != round->mItems.end())
        it->second += instance->getCount();
    else
        round->mItems.insert(std::make_pair(itemId, instance->getCount()));
}
void GameStats::RecordContainer(const StatsUID& uid, int type, int x, int y, int z)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == 0)
        return;

    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    for (std::vector<ContainerInfo*>::iterator it = round->mContainers.begin();
         it != round->mContainers.end(); ++it)
    {
        ContainerInfoLayout* existing = reinterpret_cast<ContainerInfoLayout*>(*it);
        if (existing != 0 && existing->mX == x && existing->mY == y && existing->mZ == z)
            return;
    }

    round->mContainers.push_back(allocateContainerInfo(type, x, y, z));

    int requiredChests = 0;
    LevelRuleset* rules = g_pApp->getGameRuleDefinitions();
    if (rules != 0)
    {
        GameRuleDefinition* rule = rules->getRuleByType(static_cast<ConsoleGameRules::EGameRuleType>(0x1e));
        if (rule != 0)
            requiredChests = countChestRules(rule);
    }

    if (requiredChests == 0 || requiredChests != static_cast<int>(round->mContainers.size()))
        return;

    PlayerList* players = serverPlayers();
    if (players == 0)
        return;

    boost::shared_ptr<ServerPlayer> player = players->getPlayerFromStatsUID(uid);
    if (!player)
        return;

    player->awardStat(GenericStats::mine(), GenericStats::param_mine());
}
void GameStats::RecordWalk(const StatsUID& uid, int distance, bool sprinting, bool sneaking) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL) return;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    if (sprinting) round->mSprintDistance += distance;
    else if (sneaking) round->mSneakDistance += distance;
    else round->mWalkDistance += distance;
}
void GameStats::RecordSwim(const StatsUID& uid, int distance, bool underwater) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL) return;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    if (underwater) round->mUnderwaterDistance += distance;
    else round->mSwimDistance += distance;
}
void GameStats::RecordClimb(const StatsUID& uid, int distance) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) stats->getRoundStats(mCurrentRound)->mClimbDistance += distance;
}
void GameStats::RecordGainPoints(const StatsUID& uid, int points) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) stats->getRoundStats(mCurrentRound)->mBonusPoints += points;
}
void GameStats::ResetTempPoints(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    round->mLastPointsAdded = 0;
    round->mBonusPoints = 0;
    round->mBlocksUsed = PlayerStats::RoundStats::BlockInteractionInfo();
}
unsigned int GameStats::RecordSecondaryTargetReached(const StatsUID& uid, const std::wstring& target, int points,
                                                     GlideRingGenerator::eGlideRingSize size)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return 0;

    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    std::vector<std::wstring>::const_iterator it;
    for (it = round->mMedals.begin(); it != round->mMedals.end(); ++it)
        if (*it == target)
            return 0;
    for (it = round->mMedalNames.begin(); it != round->mMedalNames.end(); ++it)
        if (*it == target)
            return 0;

    ++round->mSecondaryTargets;
    round->mMedalNames.push_back(target);
    round->mBonusPoints += points;
    round->mLastPointsAdded = points;
    ++round->mTimeSpent[static_cast<int>(size)];
    return 1;
}
void GameStats::TakeTimestamp(const StatsUID& uid, eTimeSpentCategory category, bool start) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) stats->getRoundStats(mCurrentRound)->MedalTimestamp(category, start, System::processTimeInMilliSecs());
}
void GameStats::RecordUseItem(const StatsUID& uid, not_null_ptr<ItemInstance> item)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL || item.get() == NULL)
        return;

    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    Item* itemType = item->getItem();
    if (itemType == NULL)
        return;

    round->mItemsUsed.push_back(itemType);
    if (itemType == Items::potion)
        ++round->mPotionsUsed;
    else if (itemType == Items::splashPotion)
        ++round->mSplashPotionsUsed;
}
void GameStats::RecordTimeOnLayer(const StatsUID& uid, int layer, bool start)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL || layer == -1)
        return;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    while (round->mLayerTime.size() <= static_cast<std::size_t>(layer))
        round->mLayerTime.push_back(0);
    ++round->mLayerTime.at(static_cast<std::size_t>(layer));
    if (start)
        ++round->mLayerTimeCounter;
}
void GameStats::RecordSnowballDirectHit(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        ++stats->mSnowballDirectHits;
}
void GameStats::RecordHitBySnowball(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        ++stats->mHitsFromProjectile;
}
void GameStats::RecordLanding(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL || mLandingRecorded)
        return;
    mLandingRecorded = true;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    round->mReachedEnd = true;
}
void GameStats::RecordBlockPlaced(const StatsUID& uid, const BlockPos&, int state) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) {
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        ++round->mBlocksPlaced;
        ++round->mBlocksPlacedInfo.mCount;
        ++round->mBlocksPlacedInfo.mItems[state];
    }
}
void GameStats::RecordBlockDestroyed(const StatsUID& uid, const BlockPos&, int state) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) {
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        ++round->mBlocksDestroyed.mCount;
        ++round->mBlocksDestroyed.mItems[state];
    }
}
unsigned int GameStats::RecordLastCheckpoint(const StatsUID&, int checkpoint, int, bool)
{
    if (static_cast<unsigned int>(checkpoint) > mFarthestCheckpointReached)
        mFarthestCheckpointReached = checkpoint;
    return static_cast<unsigned int>(checkpoint);
}
void GameStats::RecordLapCompleted(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
    {
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        ++round->mLapsCompleted;
    }
}
unsigned int GameStats::GetLapsCompleted(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return 0;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    return static_cast<unsigned int>(round->mLapsCompleted);
}
unsigned int GameStats::GetLastCheckpointID(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : static_cast<unsigned int>(stats->getRoundStats(mCurrentRound)->mLastCheckpointId);
}
unsigned int GameStats::GetLastCheckpointSequenceID(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : static_cast<unsigned int>(stats->getRoundStats(mCurrentRound)->mLastCheckpointSequenceId);
}
void GameStats::RecordThermalEntered(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) ++stats->getRoundStats(mCurrentRound)->mThermalsEntered[0];
}
void GameStats::RecordProgress(const StatsUID& uid, double x, double y, double z, double progress) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) {
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        round->mProgressStats[0] = static_cast<int>(x);
        round->mProgressStats[1] = static_cast<int>(y);
        round->mProgressStats[2] = static_cast<int>(z);
        round->mCurrentTime = progress;
    }
}
void GameStats::RecordSpawnedAtCheckpoint(const StatsUID& uid, int checkpoint) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) stats->getRoundStats(mCurrentRound)->mSpawnedCheckpoints.insert(checkpoint);
}
void GameStats::RecordTimeSpentInPosition(bool inside)
{
    StatsUID highestUid = StatsUID::EMPTY_SUID();
    StatsUID lowestUid = StatsUID::EMPTY_SUID();
    double highestValue = 0.0;
    double lowestValue = 1.7976931348623157e308;

    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second == 0)
            continue;

        PlayerStats::RoundStats* round = it->second->getRoundStats(mCurrentRound);
        if (!inside)
        {
            const double value = round->mTopSpeed;
            if (value <= highestValue)
                continue;
            highestValue = value;
            highestUid = it->first;
            continue;
        }

        const double value = static_cast<double>(
            *reinterpret_cast<int*>(reinterpret_cast<char*>(round) + 0xC8)
            + *reinterpret_cast<int*>(reinterpret_cast<char*>(round) + 0xCC)
            - 4503599627370496.0);
        if (value > highestValue)
        {
            highestValue = value;
            highestUid = it->first;
        }
        else if (value < lowestValue)
        {
            lowestValue = value;
            lowestUid = it->first;
        }
    }

    const long long now = System::processTimeInMilliSecs();
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second == 0)
            continue;
        PlayerStats::RoundStats* round = it->second->getRoundStats(mCurrentRound);
        round->MedalTimestamp(0, it->first == highestUid, now);
        round->MedalTimestamp(1, it->first == lowestUid, now);
    }
}
unsigned int GameStats::RecordEndZoneEntered(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return 0;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    if (round->mReachedEnd)
        return 0;
    round->mReachedEnd = true;
    round->mLifeSpan = static_cast<int>(System::processTimeInMilliSecs() - mRoundStartTime);
    return 1;
}
unsigned char GameStats::GetEndZoneEntered(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : static_cast<unsigned char>(stats->getRoundStats(mCurrentRound)->mReachedEnd);
}
boost::shared_ptr<ServerPlayer> GameStats::GetLeadingPlayer()
{
    PlayerList* players = serverPlayers();
    if (players == 0)
        return boost::shared_ptr<ServerPlayer>();

    std::vector<boost::shared_ptr<ServerPlayer> >* playerList = players->getPlayers();
    if (playerList == 0)
        return boost::shared_ptr<ServerPlayer>();

    double bestProgress = -1.0;
    boost::shared_ptr<ServerPlayer> bestPlayer;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = playerList->begin();
         it != playerList->end(); ++it)
    {
        if (!*it)
            continue;

        PlayerStats* stats = GetPlayerStats((*it)->getSUID());
        if (stats == 0)
            continue;

        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        if (round == 0)
            continue;

        const double progress = round->mCurrentTime;
        if (bestProgress < progress || !bestPlayer)
        {
            bestPlayer = *it;
            bestProgress = progress;
        }
        else if (bestPlayer->isSpectator() && !(*it)->isSpectator())
        {
            bestPlayer = *it;
            bestProgress = progress;
        }
    }

    return bestPlayer;
}
boost::shared_ptr<ServerPlayer> GameStats::GetTrailingPlayer(const boost::shared_ptr<ServerPlayer>& player) { return player; }
void GameStats::increasePlayerRoundStartTimeMs(long long milliseconds)
{
    const uint32_t addLow = static_cast<uint32_t>(milliseconds);
    const uint32_t addHigh = static_cast<uint32_t>(static_cast<uint64_t>(milliseconds) >> 32);

    char* base = reinterpret_cast<char*>(this);
    const uint32_t startLow = *reinterpret_cast<uint32_t*>(base + 0x34);
    *reinterpret_cast<uint32_t*>(base + 0x34) = startLow + addLow;
    *reinterpret_cast<uint32_t*>(base + 0x30) =
        *reinterpret_cast<uint32_t*>(base + 0x30) + addHigh
        + static_cast<uint32_t>(startLow + addLow < startLow);

    const int32_t endHigh = *reinterpret_cast<int32_t*>(base + 0x38);
    const uint32_t endLow = *reinterpret_cast<uint32_t*>(base + 0x3c);
    if (endHigh < 0)
        return;
    if (endHigh < 1 && endLow == 0)
        return;

    *reinterpret_cast<uint32_t*>(base + 0x3c) = endLow + addLow;
    *reinterpret_cast<int32_t*>(base + 0x38) =
        endHigh + static_cast<int32_t>(addHigh)
        + static_cast<int32_t>(endLow + addLow < endLow);
}
long long GameStats::GetStartTime() { return mRoundStartTime; }
boost::shared_ptr<ServerPlayer> GameStats::GetRoundWinner() { return mRoundWinner; }
boost::shared_ptr<ServerPlayer> GameStats::GetSecondPlace() { return mSecondPlace; }
boost::shared_ptr<ServerPlayer> GameStats::GetThirdPlace() { return mThirdPlace; }
std::wstring GameStats::GetWinningThemeChoice() const { return mWinningThemeChoice; }
unsigned int GameStats::GetRoundsWon(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : static_cast<unsigned int>(stats->getTotalRoundsWon());
}
unsigned int GameStats::GetMostBlocksDestroyed()
{
    unsigned int result = 0;
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second != NULL)
            result = std::max(result, static_cast<unsigned int>(it->second->getTotalBlocksDestroyed()));
    }
    return result;
}
unsigned int GameStats::GetSecondMostBlocksDestroyed()
{
    unsigned int first = 0;
    unsigned int second = 0;
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second == NULL)
            continue;
        const unsigned int value = static_cast<unsigned int>(it->second->getTotalBlocksDestroyed());
        if (value > first)
        {
            second = first;
            first = value;
        }
        else if (value > second)
            second = value;
    }
    return second;
}
unsigned int GameStats::GetThirdMostBlocksDestroyed()
{
    unsigned int first = 0;
    unsigned int second = 0;
    unsigned int third = 0;
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second == NULL)
            continue;
        const unsigned int value = static_cast<unsigned int>(it->second->getTotalBlocksDestroyed());
        if (value > first)
        {
            third = second;
            second = first;
            first = value;
        }
        else if (value > second)
        {
            third = second;
            second = value;
        }
        else if (value > third)
            third = value;
    }
    return third;
}
unsigned int GameStats::GetFastestPlayerTime()
{
    unsigned int fastest = 0xffffffffU;
    if (static_cast<int>(mCurrentRound) >= 0)
    {
        for (int round = 0; round <= static_cast<int>(mCurrentRound); ++round)
        {
            const unsigned int lifespan = GetShortestLifespan(static_cast<unsigned int>(round));
            if (lifespan < fastest)
                fastest = lifespan;
        }
    }
    return fastest;
}
unsigned int GameStats::GetHighestScoringPlayerScore()
{
    unsigned int result = 0;
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second != NULL)
            result = std::max(result, static_cast<unsigned int>(it->second->getBestPoints(true, false)));
    }
    return result;
}
unsigned int GameStats::GetFarthestCheckpointReached() { return mFarthestCheckpointReached; }
