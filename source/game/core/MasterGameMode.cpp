#include "game/core/MasterGameMode.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/GameType.hpp"
#include "game/core/System.hpp"
#include "game/core/Voteable.hpp"
#include "game/core/Voteables.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/GameModeUtilities.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/KillboxRuleDefinition.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/minigame/WorldPositionRuleDefinition.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/item/DistributeItemsRuleDefinition.hpp"
#include "game/item/Items.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/core/Team.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/ClientboundMGPlayerSettingsUpdatePacket.hpp"
#include "game/network/PlayerReadyPacket.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/ClientboundPlayerAbilitiesPacket.hpp"
#include "game/network/MapSelectInfoPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerScoreboard.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/GameCondition.hpp"
#include "game/core/ConditionConnective.hpp"
#include "game/world/generation/PlayersCondition.hpp"
#include "game/world/generation/MapGenerator.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/MiniGamePlayerSettings.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/block/BlockRemover.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/item/Container.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerStats.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cwctype>
#include <deque>
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/File.hpp"
#include "game/core/OnGameStartSpawnPositions.hpp"
#include "game/core/RegionFileCache.hpp"
#include "game/core/SpawnPositionSet.hpp"
#include "game/core/maybe.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/server/ServerChunkCache.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include "game/world/chunk/MinigamePlayerChunkMap.hpp"
#include "game/world/generation/TumbleGenerator.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/FoodData.hpp"
#include "game/data/StringTable.hpp"

extern void instructionSynchronize();

typedef boost::unordered_map<MasterGameMode::ETimerID, MasterGameMode::_TimerInfo> TimerMap;

namespace {

struct ChestRefillWiiULayout
{
    char prefix[0x34];
    std::wstring lootGroup;
};

static std::wstring& ChestLootGroup(MasterGameMode::ChestRefillInfo& info)
{
    return reinterpret_cast<ChestRefillWiiULayout&>(info).lootGroup;
}

static const std::wstring& ChestLootGroup(const MasterGameMode::ChestRefillInfo& info)
{
    return reinterpret_cast<const ChestRefillWiiULayout&>(info).lootGroup;
}

struct WorldPositionWiiULayout
{
    char prefix[0x64];
    int x;
    int y;
    int z;
};

static BlockPos WorldPositionFromRule(const WorldPositionRuleDefinition* position)
{
    const WorldPositionWiiULayout& layout =
        reinterpret_cast<const WorldPositionWiiULayout&>(*position);
    return BlockPos(layout.x, layout.y, layout.z);
}

static const std::wstring kSpectatorTeamName;

static bool StringsEqualIgnoreCase(const std::wstring& first, const std::wstring& second)
{
    if (first.size() != second.size())
        return false;
    for (size_t index = 0; index < first.size(); ++index)
    {
        if (std::towlower(first[index]) != std::towlower(second[index]))
            return false;
    }
    return true;
}

static bool IsMidRoundJoinState(unsigned int state)
{
    if (state < 5)
        return false;
    if (state <= 9)
        return true;
    return state >= 13 && state <= 14;
}

static float HostChanceToFloat(unsigned int chance)
{
    return static_cast<float>(chance) / 100.0f;
}

static void SendGameModePacket(boost::shared_ptr<ServerPlayer> player,
                               GameModePacket::EMessage message,
                               const GameModePacket::Data& data)
{
    if (player == 0 || player->mConnection == 0)
        return;
    player->mConnection->send(GameModePacket::Create(message, data));
}

static unsigned char g_0x104C3EA1 = 0;
static unsigned char g_0x104C3EA2 = 0;

struct MasterGameModeWiiUChunkBounds
{
    int mMinChunkX;
    int mMaxChunkX;
    int mMinChunkZ;
    int mMaxChunkZ;
};

static MasterGameModeWiiUChunkBounds& WiiUChunkBounds(MasterGameMode* gameMode)
{
    return *reinterpret_cast<MasterGameModeWiiUChunkBounds*>(
        reinterpret_cast<char*>(gameMode) + 0x230);
}

static arrayWithLength<unsigned char>& WiiUSpawnDistributionData(MasterGameMode* gameMode)
{
    return *reinterpret_cast<arrayWithLength<unsigned char>*>(
        reinterpret_cast<char*>(gameMode) + 0x240);
}

static arrayWithLength<unsigned char>& WiiUAlternateSpawnDistributionData(MasterGameMode* gameMode)
{
    return *reinterpret_cast<arrayWithLength<unsigned char>*>(
        reinterpret_cast<char*>(gameMode) + 0x248);
}

static int& WiiULoadingPlayerCount(MasterGameMode* gameMode)
{
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(gameMode) + 0x2d8);
}

struct MasterGameModeWiiUBroadcastChatState
{
    unsigned long long lastBroadcastTime;
    int lastChatMessage;
};

static MasterGameModeWiiUBroadcastChatState& WiiUBroadcastChatState(MasterGameMode* gameMode)
{
    return *reinterpret_cast<MasterGameModeWiiUBroadcastChatState*>(
        reinterpret_cast<char*>(gameMode) + 0x308);
}

static std::vector<int>& WiiUBuildOffThemeIds(MasterGameMode* gameMode)
{
    return *reinterpret_cast<std::vector<int>*>(reinterpret_cast<char*>(gameMode) + 0xac);
}

static arrayWithLength<unsigned char>& WiiUPlaylistSyncData(MasterGameMode* gameMode)
{
    return *reinterpret_cast<arrayWithLength<unsigned char>*>(
        reinterpret_cast<char*>(gameMode) + 0x1f0);
}

static unsigned int GameStatsCurrentRound(GameStats& gameStats)
{
    return *reinterpret_cast<unsigned int*>(reinterpret_cast<char*>(&gameStats) + 8);
}

static StringTable*& WiiUEnglishStringTable(MasterGameMode* gameMode)
{
    return *reinterpret_cast<StringTable**>(reinterpret_cast<char*>(gameMode) + 0xc);
}

static EGameModeState TranslateInternalGameModeState(MasterGameMode::EInternalGameModeState state)
{
    switch (state)
    {
    case MasterGameMode::eInternalGameModeState_WaitingToStart:
        return static_cast<EGameModeState>(5);
    case MasterGameMode::eInternalGameModeState_RoundStarting:
        return static_cast<EGameModeState>(6);
    case MasterGameMode::eInternalGameModeState_RoundPlaying:
        return static_cast<EGameModeState>(7);
    case MasterGameMode::eInternalGameModeState_Showdown:
        return static_cast<EGameModeState>(3);
    case MasterGameMode::eInternalGameModeState_RoundEnding:
        return static_cast<EGameModeState>(9);
    case MasterGameMode::eInternalGameModeState_PostGame:
        return static_cast<EGameModeState>(10);
    case MasterGameMode::eInternalGameModeState_ReturningToLobby:
        return static_cast<EGameModeState>(2);
    case MasterGameMode::eInternalGameModeState_Lobby:
    default:
        return static_cast<EGameModeState>(0);
    }
}

static void ClearSpawnDistributionSlot(arrayWithLength<unsigned char>& distribution,
                                       unsigned char playerIndex)
{
    if (distribution.data == 0 || distribution.length == 0)
        return;

    for (unsigned int index = 0; index < distribution.length; ++index)
    {
        if (distribution.data[index] == playerIndex)
            distribution.data[index] = 0xff;
    }
}

static const SoundEvent* SelectFinishLineSound(unsigned int finishers,
                                               bool scoreVariant,
                                               const boost::shared_ptr<ServerPlayer>& winner,
                                               GameStats& gameStats)
{
    const SoundEvent* sound = SoundEvents::MG03_FINISH_LINE;
    if (winner == 0)
        return sound;

    const unsigned int round = GameStatsCurrentRound(gameStats);
    const StatsUID suid = winner->getSUID();
    PlayerStats* winnerStats = gameStats.GetPlayerStats(suid);
    if (winnerStats == 0)
        return sound;

    if (scoreVariant)
    {
        PlayerStats::RoundStats* roundStats = winnerStats->getRoundStats(round);
        if (roundStats == 0)
            return sound;

        const unsigned int playerScore =
            static_cast<unsigned int>(roundStats->mPoints + roundStats->mBonusPoints);
        const unsigned int highestScore = gameStats.GetHighestScoringPlayerScore();
        const bool bestScore = playerScore >= highestScore && finishers != 0;

        if (bestScore)
        {
            if (finishers == 1)
                sound = SoundEvents::MG03_FINISH_LINE_1ST_BEST_SCORE;
            else if (finishers < 3)
                sound = SoundEvents::MG03_FINISH_LINE_2ND_BEST_SCORE;
            else if (finishers != 3)
                sound = SoundEvents::MG03_FINISH_LINE_3RD_BEST_SCORE;
        }
        else if (finishers == 1)
            sound = SoundEvents::MG03_FINISH_LINE_1ST;
        else if (finishers < 3)
            sound = SoundEvents::MG03_FINISH_LINE_2ND;
        else if (finishers == 3)
            sound = SoundEvents::MG03_FINISH_LINE_3RD;
    }
    else
    {
        const unsigned int playerTime =
            static_cast<unsigned int>(winnerStats->getLifeSpan(round));
        const unsigned int fastestTime = gameStats.GetFastestPlayerTime();
        const bool bestTime = playerTime <= fastestTime && finishers != 0;

        if (bestTime)
        {
            if (finishers == 1)
                sound = SoundEvents::MG03_FINISH_LINE_1ST_BEST_TIME;
            else if (finishers < 3)
                sound = SoundEvents::MG03_FINISH_LINE_2ND_BEST_TIME;
            else if (finishers == 3)
                sound = SoundEvents::MG03_FINISH_LINE_3RD_BEST_TIME;
        }
        else if (finishers == 1)
            sound = SoundEvents::MG03_FINISH_LINE_1ST;
        else if (finishers < 3)
            sound = SoundEvents::MG03_FINISH_LINE_2ND;
        else if (finishers == 3)
            sound = SoundEvents::MG03_FINISH_LINE_3RD;
    }

    return sound;
}

static void BroadcastGameModePacket(GameModePacket::EMessage message,
                                    const GameModePacket::Data& data)
{
    if (gMinecraftServer == 0 || gMinecraftServer->getPlayers() == 0)
        return;
    gMinecraftServer->getPlayers()->broadcastAll(GameModePacket::Create(message, data));
}

static bool& WiiUBattleLootSetChosen(MasterGameMode* gameMode)
{
    return *reinterpret_cast<bool*>(reinterpret_cast<char*>(gameMode) + 0x168);
}

static bool& WiiUPlayersInvulnerable(MasterGameMode* gameMode)
{
    return *reinterpret_cast<bool*>(reinterpret_cast<char*>(gameMode) + 0x161);
}

static bool& WiiUAchievementExploitStopped(MasterGameMode* gameMode)
{
    return *reinterpret_cast<bool*>(reinterpret_cast<char*>(gameMode) + 0x174);
}

static int& WiiUTutorialLastPlayerCount(MasterGameMode* gameMode)
{
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(gameMode) + 0x2f8);
}

static int& WiiULayerBlocksDestroyed(MasterGameMode* gameMode)
{
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(gameMode) + 0x2ec);
}

static std::vector<GameCondition*>& ConditionConnectiveConditions(ConditionConnective* connective)
{
    return *reinterpret_cast<std::vector<GameCondition*>*>(reinterpret_cast<char*>(connective) + 8);
}

static int& GhostControllerLastTickHigh(GhostController* controller)
{
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(controller) + 0x68);
}

static unsigned int& GhostControllerLastTickLow(GhostController* controller)
{
    return *reinterpret_cast<unsigned int*>(reinterpret_cast<char*>(controller) + 0x6c);
}

static int& GhostControllerPlaybackTimeHigh(GhostController* controller)
{
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(controller) + 0x70);
}

static unsigned int& GhostControllerPlaybackTimeLow(GhostController* controller)
{
    return *reinterpret_cast<unsigned int*>(reinterpret_cast<char*>(controller) + 0x74);
}

static std::map<MasterGameMode::EInternalGameModeState, EGameModeState> g_0x104F5CC0;

static bool g_0x104C6DBD = true;

static bool& WiiUPlayerRoundEligible(MasterGameMode* gameMode, int playerIndex)
{
    return *reinterpret_cast<bool*>(reinterpret_cast<char*>(gameMode) + 0x2a8 + playerIndex);
}

} // namespace

MasterGameMode::MasterGameMode(std::vector<LevelRules::LevelRulesetGroup*>* playlist)
    : mRoundStartTime(0), mRoundTimeOffset(0), mPausedRoundTime(0), mShowdownTime(0), mPostGameTime(0), mTimers(), mNextLevelGenerationOptions(0), mGameStats(), mInternalState(eInternalGameModeState_Lobby), mPreviousState(eInternalGameModeState_Lobby), mPlaylist(), mCurrentPlaylistEntry(0), mCurrentRuleIndex(0), mReadyPlayers(0), mRequiredPlayers(0), mMinimumPlayers(0), mMaximumPlayers(8), mChestRefills(), mKillboxes(), mKillboxAreas(), mActiveChunkBounds(), mRandom(), mLockPlayerPositions(false), mRoundStarted(false), mRoundEnding(false), mShowdownStarted(false), mPlaylistDirty(false), mTutorialDirty(false), mPlayersPrepared(false), mReturningToLobby(false), mGeneratorSpawn(0), mSpawnLayerIndex(0), mChunkHistory(0), mRoundWinner(-1), mActiveTeamCount(0), mShowdownReduction(0), mChosenThemeWordId(0), mTutorialData(), mRoundLevel(0), mBlockRemover(0), mLastKnownPositionPlayers(), mScoreboard(0)
{
    for (int index = 0; index < 8; ++index) { mPlayerReady[index] = false; mLastKnownPositions[index] = 0; }
    if (playlist != 0) mPlaylist = *playlist;
    ResetRoundTime();
}

MasterGameMode::~MasterGameMode()
{
    KillTimers();
}

std::vector<boost::shared_ptr<ServerPlayer> > MasterGameMode::GetPlayers(bool includeSpectators, bool includeDead)
{
    std::vector<boost::shared_ptr<ServerPlayer> >* allPlayers = gMinecraftServer->getPlayers()->getPlayers();
    std::vector<boost::shared_ptr<ServerPlayer> > players;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = allPlayers->begin();
         playerIt != allPlayers->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;

        if (includeSpectators && player->isSpectator())
            continue;

        if (includeDead && !WiiUPlayerRoundEligible(this, player->mPlayerIndex))
            continue;

        players.push_back(player);
    }

    return players;
}

int MasterGameMode::GetPlayerCount(bool includeSpectators, bool includeDead)
{
    return static_cast<int>(GetPlayers(includeSpectators, includeDead).size());
}

unsigned long long MasterGameMode::GetCurrentTimeMillis()
{
    return System::processTimeInMilliSecs();
}

bool MasterGameMode::HasTimer(ETimerID timer)
{
    TimerMap::iterator entry = mTimers.find(timer);
    return entry != mTimers.end() && entry->second.active;
}

void MasterGameMode::KillTimer(ETimerID timer)
{
    TimerMap::iterator entry = mTimers.find(timer);
    if (entry != mTimers.end())
        entry->second.active = false;
}

void MasterGameMode::ResetRoundTime()
{
    mRoundStartTime = GetCurrentTimeMillis();
    mRoundTimeOffset = 0;
    mPausedRoundTime = 0;
    mShowdownTime = 0;
    mPostGameTime = 0;
    KillTimer(eTimerID_RoundLimit);
}

void MasterGameMode::StaticCtor()
{
    g_0x104F5CC0[eInternalGameModeState_Lobby] = static_cast<EGameModeState>(1);
    g_0x104F5CC0[eInternalGameModeState_WaitingToStart] = static_cast<EGameModeState>(1);
    g_0x104F5CC0[eInternalGameModeState_RoundStarting] = static_cast<EGameModeState>(2);
    g_0x104F5CC0[eInternalGameModeState_RoundPlaying] = static_cast<EGameModeState>(3);
    g_0x104F5CC0[eInternalGameModeState_Showdown] = static_cast<EGameModeState>(3);
    g_0x104F5CC0[eInternalGameModeState_RoundEnding] = static_cast<EGameModeState>(4);
    g_0x104F5CC0[eInternalGameModeState_PostGame] = static_cast<EGameModeState>(10);
    g_0x104F5CC0[eInternalGameModeState_ReturningToLobby] = static_cast<EGameModeState>(4);
    g_0x104F5CC0[static_cast<EInternalGameModeState>(8)] = static_cast<EGameModeState>(5);
    g_0x104F5CC0[static_cast<EInternalGameModeState>(9)] = static_cast<EGameModeState>(6);
    g_0x104F5CC0[static_cast<EInternalGameModeState>(10)] = static_cast<EGameModeState>(7);
    g_0x104F5CC0[static_cast<EInternalGameModeState>(11)] = static_cast<EGameModeState>(8);
    g_0x104F5CC0[static_cast<EInternalGameModeState>(12)] = static_cast<EGameModeState>(9);
    g_0x104F5CC0[static_cast<EInternalGameModeState>(13)] = static_cast<EGameModeState>(9);
    g_0x104F5CC0[static_cast<EInternalGameModeState>(14)] = static_cast<EGameModeState>(10);
}

void MasterGameMode::GeneratePlaylistSyncInfo()
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    const EMiniGameId miniGameType = gApp.GetFirstMiniGameType();
    if (miniGameType == eMiniGameId_Battle || miniGameType == eMiniGameId_Glide)
    {
        StringTable localTable;
        int totalRulesetCount = 0;
        for (std::deque<LevelRules::LevelRulesetGroup*>::iterator groupIt = mRuleGroups.begin();
             groupIt != mRuleGroups.end(); ++groupIt)
            totalRulesetCount += static_cast<int>((*groupIt)->rulesets.size());
        stream.writeInt(totalRulesetCount);

        std::wstring worldNameKey(L"Active");
        for (std::deque<LevelRules::LevelRulesetGroup*>::iterator groupIt = mRuleGroups.begin();
             groupIt != mRuleGroups.end(); ++groupIt)
        {
            LevelRules::LevelRulesetGroup* group = *groupIt;
            for (std::vector<LevelRuleset*>::iterator rulesetIt = group->rulesets.begin();
                 rulesetIt != group->rulesets.end(); ++rulesetIt)
            {
                LevelRuleset* ruleset = *rulesetIt;
                LevelGenerationOptions* options = ruleset->getGenerationOptions();
                stream.writeInt(static_cast<int>(options->getUniqueId()));
                worldNameKey = options->getWorldNameKey();
                stream.writeUTF(worldNameKey);
                options = ruleset->getGenerationOptions();
                stream.writeInt(static_cast<int>(options->getTexturePackId()));
                options = ruleset->getGenerationOptions();
                options->copyStringToTable(worldNameKey, &localTable);
            }
        }
        localTable.write(&stream);
    }
    else if (miniGameType == eMiniGameId_Build)
    {
        Random random;
        std::vector<int> themeIds = WiiUEnglishStringTable(this)->getRandomStringIds(5, &random);
        stream.writeInt(static_cast<int>(themeIds.size()));
        for (std::vector<int>::iterator idIt = themeIds.begin(); idIt != themeIds.end(); ++idIt)
            stream.writeInt(*idIt);
        WiiUBuildOffThemeIds(this) = themeIds;
    }

    WiiUPlaylistSyncData(this) = byteStream.toByteArray();
}

void MasterGameMode::ResetReadyStates(bool allPlayers)
{
    for (unsigned int index = 0; index < 8; ++index)
    {
        mPlayerReady[index] = allPlayers;
        if (gMinecraftServer != 0 && gMinecraftServer->getPlayers() != 0)
        {
            boost::shared_ptr<Packet> readyPacket(
                new PlayerReadyPacket(index, allPlayers));
            gMinecraftServer->getPlayers()->broadcastAll(readyPacket);
        }
    }
}

void MasterGameMode::ResetLastKnownPosition(int playerIndex)
{
    mPlaylistDirty = true;
}

int MasterGameMode::GetMinimumPlayersToStart()
{
    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame->GetHostOption(static_cast<eCustomGameModeHostOption>(0xe)))
        return 1;

    int result = 0;
    unsigned int miniGameId = miniGame->GetId();
    if (miniGameId == eMiniGameId_Lobby)
        miniGameId = GetNextGameMode();

    if (miniGameId != 0 && miniGameId != static_cast<unsigned int>(eMiniGameId_Any))
    {
        const MiniGameDef* customMode =
            MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(miniGameId), true);
        GameCondition* startConditions =
            reinterpret_cast<GameCondition*>(customMode->GetStartConditions());
        ConditionConnective* connective = dynamic_cast<ConditionConnective*>(startConditions);
        if (connective != 0 && ConditionConnectiveConditions(connective).size() == 1)
        {
            PlayersCondition* playersCondition =
                dynamic_cast<PlayersCondition*>(ConditionConnectiveConditions(connective)[0]);
            if (playersCondition != 0)
                result = playersCondition->GetValue(this);
        }
    }

    return result;
}

MasterGameMode::EInternalGameModeState MasterGameMode::GetState()
{
    return static_cast<EInternalGameModeState>(mInternalState);
}

void MasterGameMode::RestartTimer(ETimerID timer, int duration)
{
    TimerMap::iterator entry = mTimers.find(timer);
    if (entry != mTimers.end())
    {
        entry->second.startTime = GetCurrentTimeMillis();
        entry->second.duration = duration;
        entry->second.endTime = duration > 0 ? entry->second.startTime + duration : -1;
        entry->second.active = true;
    }
}

void MasterGameMode::TickTimers()
{
    if (g_0x104C6DBD)
    {
        RestartTimer(eTimerID_GameEnd, GetMiniGame()->getTimerSpec(eTimerID_GameEnd).TotalDurationMs());
        g_0x104C6DBD = false;
    }

    for (TimerMap::iterator entry = mTimers.begin(); entry != mTimers.end(); ++entry)
    {
        _TimerInfo& timer = entry->second;
        if (timer.active && timer.endTime >= 0 &&
            GetCurrentTimeMillis() >= static_cast<unsigned long long>(timer.endTime))
        {
            timer.active = false;
            if (timer.callback != 0)
                timer.callback(entry->first, this, timer.callbackData);
        }
    }
}

void MasterGameMode::SyncNextLevelGen()
{
    if (mCurrentLevelGenOptions == 0)
        return;

    LevelRuleset* ruleset = mCurrentLevelGenOptions->getRequiredGameRules();
    const MiniGameDef* miniGame =
        MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(ruleset->getRuleType()), true);
    const unsigned int miniGameId = miniGame->GetId();
    const unsigned int uniqueId = mCurrentLevelGenOptions->getUniqueId();

    GameModePacket::ChangeCustomGameMode data(miniGameId, uniqueId);
    boost::shared_ptr<Packet> packet(
        GameModePacket::Create(static_cast<GameModePacket::EMessage>(0x12), data));
    if (gMinecraftServer != 0)
        gMinecraftServer->getPlayers()->broadcastAll(packet);
}

bool MasterGameMode::OnGameModeChange(boost::shared_ptr<ServerPlayer> player)
{
    if (player->mConnection == 0)
        return false;

    boost::shared_ptr<Packet> packet(
        GameModePacket::Create(static_cast<GameModePacket::EMessage>(0x15), mTutorialData));
    player->mConnection->send(packet);
    return true;
}

void MasterGameMode::CheckTutorialUpdate()
{
    PlayerList* playerList = gMinecraftServer->getPlayers();
    std::vector<boost::shared_ptr<ServerPlayer> >* players = playerList->getPlayers();

    if (!mTutorialDirty)
    {
        if (static_cast<int>(players->size()) == WiiUTutorialLastPlayerCount(this))
            return;
    }

    WiiUTutorialLastPlayerCount(this) = static_cast<int>(players->size());
    if (WiiUTutorialLastPlayerCount(this) > 0)
    {
        SyncNextLevelGen();
        players = playerList->getPlayers();
    }

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (OnGameModeChange(player))
            mTutorialDirty = false;
    }
}

bool MasterGameMode::IsInKillBox(boost::shared_ptr<Entity> entity)
{
    return mRoundStarted || mRoundEnding;
}

bool MasterGameMode::AreLevelChunksLoaded()
{
    return mRoundStarted || mRoundEnding;
}

void MasterGameMode::ChooseNextGameRules(bool advancePlaylist)
{
    LevelGenerationOptions* selectedOptions = 0;
    LevelRules::LevelRulesetGroup* selectedGroup = 0;
    int totalVotes = 0;

    Voteable* voteable = mVoteables->getVoteable(0);
    int winningVote = voteable->getWinningVote(totalVotes);
    if (winningVote != -1)
    {
        for (std::vector<LevelRules::LevelRulesetGroup*>::iterator playlistIt = mPlaylist.begin();
             playlistIt != mPlaylist.end(); ++playlistIt)
        {
            LevelRules::LevelRulesetGroup* group = *playlistIt;
            if (static_cast<short>(LevelGenerationOptions::GetMapSizeFromUniqueId(winningVote)) == -1)
            {
                if (!group->rulesets.empty() &&
                    group->rulesets.front()->getThemeId() ==
                        LevelGenerationOptions::GetThemeFromUniqueId(winningVote))
                {
                    selectedGroup = group;
                    break;
                }
            }
            else if (!group->rulesets.empty())
            {
                for (std::vector<LevelRuleset*>::iterator rulesetIt = group->rulesets.begin();
                     rulesetIt != group->rulesets.end(); ++rulesetIt)
                {
                    LevelGenerationOptions* options = (*rulesetIt)->getGenerationOptions();
                    if (options->getUniqueId() == static_cast<unsigned int>(winningVote))
                    {
                        selectedOptions = options;
                        selectedGroup = group;
                        break;
                    }
                }
                if (selectedGroup != 0)
                    break;
            }
        }

        if (selectedGroup != 0)
        {
            std::deque<LevelRules::LevelRulesetGroup*>::iterator found =
                std::find(mRuleGroups.begin(), mRuleGroups.end(), selectedGroup);
            const int targetIndex = mCurrentPlaylistEntry + mCurrentRuleIndex;
            if (found != mRuleGroups.end())
            {
                const int foundIndex = static_cast<int>(found - mRuleGroups.begin());
                if (foundIndex != targetIndex)
                {
                    mShuffledRuleGroups.push_back(selectedGroup);
                    mRuleGroups.erase(found);
                }
            }
        }
    }

    if (selectedGroup == 0)
    {
        selectedGroup = mRuleGroups.front();
        if (!advancePlaylist)
        {
            mShuffledRuleGroups.push_back(mRuleGroups.front());
            mRuleGroups.pop_front();
        }
    }

    if (mCurrentRuleIndex == 0)
    {
        mRuleGroups.insert(mRuleGroups.begin() + mCurrentPlaylistEntry,
                           mShuffledRuleGroups.begin(), mShuffledRuleGroups.end());
        mShuffledRuleGroups.clear();
    }

    if (selectedOptions != 0)
    {
        mCurrentLevelGenOptions = selectedOptions;
        SyncNextLevelGen();
        return;
    }

    int mapSizeIndex = 2;
    const MiniGameDef* miniGame = GetMiniGame();
    unsigned int mapSizeOverride = miniGame->GetMapSizeOverride();
    if (mapSizeOverride == 0)
    {
        unsigned int playerCount = GetPlayerCount(false, false);
        if (playerCount < 9)
            mapSizeIndex = 1;
        playerCount = GetPlayerCount(false, false);
        miniGame = GetMiniGame();
        if (miniGame->GetMaxPlayersForSmallMaps() >= playerCount)
            mapSizeIndex = 0;
    }
    else
    {
        mapSizeIndex = static_cast<int>(mapSizeOverride) - 1;
    }

    std::vector<LevelRuleset*>::iterator rulesetIt = selectedGroup->rulesets.begin();
    if (rulesetIt == selectedGroup->rulesets.end())
        goto select_fallback;

    while (true)
    {
        LevelGenerationOptions* options = (*rulesetIt)->getGenerationOptions();
        if (options == 0)
            goto next_ruleset;
        if (options->getMapSize() != static_cast<unsigned int>(mapSizeIndex))
            goto next_ruleset;
        options = (*rulesetIt)->getGenerationOptions();
        if (options == 0)
            goto select_fallback;
        mCurrentLevelGenOptions = options;
        SyncNextLevelGen();
        return;

    next_ruleset:
        ++rulesetIt;
        if (rulesetIt == selectedGroup->rulesets.end())
            goto select_fallback;
    }

select_fallback:
    do
    {
        while (mapSizeIndex != 2)
        {
            if (mapSizeIndex != 1)
            {
                selectedOptions = selectedGroup->rulesets.front()->getGenerationOptions();
                if (selectedOptions == 0)
                    return;
                mCurrentLevelGenOptions = selectedOptions;
                SyncNextLevelGen();
                return;
            }
            rulesetIt = selectedGroup->rulesets.begin();
            mapSizeIndex = 0;
            if (rulesetIt != selectedGroup->rulesets.end())
                goto select_search;
        }
        rulesetIt = selectedGroup->rulesets.begin();
        mapSizeIndex = 1;
    }
    while (rulesetIt == selectedGroup->rulesets.end());

select_search:
    while (true)
    {
        selectedOptions = (*rulesetIt)->getGenerationOptions();
        if (selectedOptions == 0)
            goto select_next_ruleset;
        if (selectedOptions->getMapSize() != static_cast<unsigned int>(mapSizeIndex))
            goto select_next_ruleset;
        selectedOptions = (*rulesetIt)->getGenerationOptions();
        if (selectedOptions == 0)
            goto select_fallback;
        mCurrentLevelGenOptions = selectedOptions;
        SyncNextLevelGen();
        return;

    select_next_ruleset:
        ++rulesetIt;
        if (rulesetIt == selectedGroup->rulesets.end())
            goto select_fallback;
    }
}

void MasterGameMode::UpdateClientState(boost::shared_ptr<ServerPlayer> player)
{
    const EGameModeState wireState = TranslateInternalGameModeState(GetState());
    GameModePacket::StateChangeData stateData(wireState);
    boost::shared_ptr<Packet> packet(
        GameModePacket::Create(static_cast<GameModePacket::EMessage>(0x17), stateData));

    if (player == 0)
    {
        PlayerList* players = gMinecraftServer != 0 ? gMinecraftServer->getPlayers() : 0;
        players->broadcastAll(packet);
    }
    else
    {
        player->mConnection->send(packet);
    }
}

void MasterGameMode::SetState(EInternalGameModeState state)
{
    mPreviousState = mInternalState;
    mInternalState = state;
    mRoundStarted = state == eInternalGameModeState_RoundPlaying;
    mRoundEnding = state == eInternalGameModeState_RoundEnding;
}

void MasterGameMode::sendTutorialUpdate(eTutorial_GameMode tutorial)
{
    mPlaylistDirty = true;
}

void MasterGameMode::BroadcastChat(int message, std::vector<int>& arguments)
{
    if (message == 0)
        return;

    const unsigned long long now = System::processTimeInMilliSecs();
    MasterGameModeWiiUBroadcastChatState& chatState = WiiUBroadcastChatState(this);
    if (message == chatState.lastChatMessage)
    {
        const unsigned long long elapsed = now - chatState.lastBroadcastTime;
        if (elapsed < 501ULL)
            return;
    }

    const unsigned int argumentCount = static_cast<unsigned int>(arguments.size());
    int* packedArguments = 0;
    if (argumentCount != 0)
    {
        packedArguments = new int[argumentCount];
        for (unsigned int index = 0; index < argumentCount; ++index)
            packedArguments[index] = arguments[index];
    }

    boost::shared_ptr<Packet> chatPacket(
        new ClientboundChatPacket(static_cast<ClientboundChatPacket::EChatPacketMessage>(message),
                                  packedArguments, argumentCount, 0, 0));
    delete[] packedArguments;

    if (gMinecraftServer != 0 && gMinecraftServer->getPlayers() != 0)
        gMinecraftServer->getPlayers()->broadcastAll(chatPacket);

    chatState.lastBroadcastTime = now;
    chatState.lastChatMessage = message;
}

void MasterGameMode::CompleteTimer(ETimerID timer)
{
    TimerMap::iterator entry = mTimers.find(timer);
    if (entry != mTimers.end())
    {
        entry->second.active = false;
        if (entry->second.callback != 0)
            entry->second.callback(timer, this, entry->second.callbackData);
    }
}

void MasterGameMode::CheckReadyStates()
{
    unsigned int readyCount = 0;
    for (unsigned int index = 0; index < 8; ++index)
    {
        if (mPlayerReady[index])
            ++readyCount;
    }

    const unsigned int minimumPlayers = GetMinimumPlayersToStart();
    const unsigned int playerCount = GetPlayerCount(false, false);
    if (playerCount < minimumPlayers)
    {
        std::vector<int> arguments;
        arguments.push_back(static_cast<int>(minimumPlayers - playerCount));
        BroadcastChat(0x87, arguments);
    }

    unsigned int requiredReadyCount = playerCount;
    if (!GetMiniGame()->CanAutoStart())
    {
        const unsigned int scaled =
            static_cast<unsigned int>(ceilf(static_cast<float>(playerCount) * 0.5f));
        requiredReadyCount = scaled > minimumPlayers ? scaled : minimumPlayers;
    }

    if (readyCount < requiredReadyCount)
    {
        if (!GetMiniGame()->CanAutoStart())
        {
            std::vector<int> arguments;
            arguments.push_back(static_cast<int>(requiredReadyCount - readyCount));
            BroadcastChat(0x88, arguments);
            if (GetState() == eInternalGameModeState_WaitingToStart)
            {
                KillTimer(eTimerID_WaitingToStart);
                SetState(eInternalGameModeState_RoundStarting);
            }
        }
    }
    else
    {
        if (!GetMiniGame()->CanAutoStart())
        {
            if (GetState() == eInternalGameModeState_RoundStarting)
                SetState(eInternalGameModeState_WaitingToStart);
        }
        else if (GetState() == eInternalGameModeState_WaitingToStart)
        {
            const unsigned int currentPlayerCount = GetPlayerCount(false, false);
            const unsigned int currentMinimum = GetMinimumPlayersToStart();
            if (currentMinimum <= currentPlayerCount)
                CompleteTimer(eTimerID_WaitingToStart);
        }
    }
}

void MasterGameMode::OnGameChange(MasterGameMode* gameMode, void* context)
{
    boost::shared_ptr<Packet> packet(
        new ClientboundChatPacket(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x73), -1));
    PlayerList* players = gMinecraftServer != 0 ? gMinecraftServer->getPlayers() : 0;
    players->broadcastAll(packet);
    gameMode->SetState(eInternalGameModeState_RoundPlaying);
}

void MasterGameMode::AddTimer(ETimerID timer, TimerCallback callback, void* context, int duration, int announceEvery, int changeAnnounceAfter, int finalAnnounceEvery)
{
    _TimerInfo& info = mTimers[timer];
    info.startTime = GetCurrentTimeMillis();
    info.duration = duration;
    info.endTime = duration > 0 ? info.startTime + duration : -1;
    info.announceEvery = announceEvery;
    info.changeAnnounceAfter = changeAnnounceAfter;
    info.finalAnnounceEvery = finalAnnounceEvery;
    info.nextAnnouncement = announceEvery < 0 ? info.endTime : info.startTime + announceEvery;
    info.active = true;
    info.callbackData = context;
    info.callback = callback;
}

void MasterGameMode::AddTimer(ETimerID timer, TimerCallback callback, void* context, MiniGameDef::TimerSpec specification)
{
    AddTimer(timer, callback, context, specification.TotalDurationMs(),
             specification.AnnounceEveryMs(false),
             specification.ChangeAnnounceAfterMs(),
             specification.AnnounceEveryMs(true));
}

int MasterGameMode::GetTimerRemainingTime(ETimerID timer)
{
    TimerMap::iterator entry = mTimers.find(timer);
    if (entry == mTimers.end() || !entry->second.active)
        return 0;
    if (entry->second.endTime < 0)
        return 0x7fffffff;
    return static_cast<int>(entry->second.endTime - GetCurrentTimeMillis());
}

void MasterGameMode::TimerCountdown(ETimerID timer, CountdownInfo* countdown)
{
    int remaining = GetTimerRemainingTime(timer);
    if (remaining > 0)
    {
        std::vector<int> arguments = countdown->arguments;
        arguments.insert(arguments.begin(), (remaining + 999) / 1000);
        BroadcastChat(countdown->message, arguments);
        if (countdown->tickCallback != 0)
            countdown->tickCallback(this, countdown->callbackData);
    }
    else if (countdown->completeCallback != 0)
    {
        countdown->completeCallback(this, countdown->callbackData);
    }
}

void MasterGameMode::OnTimerCountdown(ETimerID timer, MasterGameMode* gameMode, void* context)
{
    if (gameMode != 0)
        gameMode->TimerCountdown(timer, static_cast<CountdownInfo*>(context));
}

void MasterGameMode::PopulateChests()
{
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (ruleset == 0)
        return;

    GameRuleDefinition* definition = ruleset->getRuleByType(
        static_cast<ConsoleGameRules::EGameRuleType>(30));
    if (definition == 0)
        return;

    ServerLevel* level = gMinecraftServer->getLevel(0);

    std::vector<GameRuleDefinition*> children;
    definition->getChildren(&children, static_cast<ConsoleGameRules::EGameRuleType>(6));
    for (std::vector<GameRuleDefinition*>::iterator it = children.begin();
         it != children.end(); ++it)
    {
        const MiniGameDef* miniGame = GetMiniGame();
        (*it)->evaluateRule(level, ruleset, *miniGame);
    }

    children.clear();
    definition->getChildren(&children, static_cast<ConsoleGameRules::EGameRuleType>(14));
    for (std::vector<GameRuleDefinition*>::iterator it = children.begin();
         it != children.end(); ++it)
    {
        const MiniGameDef* miniGame = GetMiniGame();
        (*it)->evaluateRule(level, ruleset, *miniGame);
    }

    children.clear();
    definition->getChildren(&children, static_cast<ConsoleGameRules::EGameRuleType>(32));
    for (std::vector<GameRuleDefinition*>::iterator it = children.begin();
         it != children.end(); ++it)
    {
        const MiniGameDef* miniGame = GetMiniGame();
        (*it)->evaluateRule(level, ruleset, *miniGame);
    }
}

int MasterGameMode::ChooseItemSet(const MiniGameDef& definition, bool central)
{
    if (definition.GetId() == eMiniGameId_Tumble)
    {
        if (central && definition.HostOption_Bool(eCustomGameModeHostOption_EnableVoting))
        {
            int winningVote = 0;
            Voteable* variantVote = mVoteables->getVoteable(0);
            if (variantVote->getWinningVote(winningVote) == -1)
            {
                const unsigned int variant = definition.GetGameVariant();
                const unsigned int variantCount = definition.GetGameVariantCount();
                definition.SetHostOption(eCustomGameModeHostOption_GameVariant,
                                         (variant + 1) % (variantCount - 2));
            }
            else
            {
                variantVote = mVoteables->getVoteable(0);
                variantVote->getWinningVote(winningVote);
                definition.SetHostOption(eCustomGameModeHostOption_GameVariant,
                                         static_cast<unsigned int>(winningVote));
            }
        }

        if (definition.HasGameVariants() &&
            !CGameNetworkManager::sInstance.IsInPublicJoinableGame() &&
            CGameNetworkManager::sInstance.IsHost())
        {
            int themeVote = 0;
            Voteable* variantVote = mVoteables->getVoteable(0);
            if (variantVote->getWinningVote(themeVote) != -1)
            {
                definition.SetHostOption(eCustomGameModeHostOption_GameVariant,
                                         static_cast<unsigned int>(themeVote));
                return themeVote;
            }
        }

        if (!definition.IsCustomGame())
            return static_cast<int>(definition.GetGameVariant());
    }
    else if (definition.GetId() == eMiniGameId_Battle)
    {
        const int lootSetId = definition.GetLootSetId();
        if (!WiiUBattleLootSetChosen(this) && lootSetId != 5)
            return lootSetId;

        WiiUBattleLootSetChosen(this) = true;
        Random random;
        return random.nextInt(5);
    }

    return definition.GetLootSetId();
}

int MasterGameMode::GetTeamCount()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (miniGame->GetId() == eMiniGameId_Build && mCurrentLevelGenOptions != 0)
    {
        std::vector<AABB*> teamAreas;
        LevelRuleset* ruleset = mCurrentLevelGenOptions->getRequiredGameRules();
        ruleset->getNamedAreas(0xff, &teamAreas);
        return static_cast<int>(teamAreas.size());
    }

    miniGame = Minecraft::GetInstance()->GetMiniGame();
    return static_cast<int>(miniGame->GetTeamCount());
}

void MasterGameMode::SetupTeams()
{
    ServerLevel* level = gMinecraftServer != 0 ? gMinecraftServer->getLevel(0) : 0;
    ServerScoreboard* scoreboard = level->GetServerScoreboard();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    const EMiniGameId miniGameId = static_cast<EMiniGameId>(miniGame->GetId());
    const bool hideDeathMessages = miniGameId == eMiniGameId_Tumble || miniGameId == eMiniGameId_Glide;

    for (int teamIndex = GetTeamCount(); teamIndex != 0; --teamIndex)
    {
        wchar_t nameBuffer[0x20];
        swprintf(nameBuffer, 0x20, L"%d", teamIndex);
        const std::wstring teamName(nameBuffer);

        PlayerTeam* team = scoreboard->getPlayerTeam(teamName);
        if (team == 0)
            team = scoreboard->addPlayerTeam(teamName);

        if (team != 0)
        {
            team->setAllowFriendlyFire(true);
            team->setNameTagVisibility(Team::HIDE_FOR_OTHER_TEAMS);
            team->setDeathMessageVisibility(hideDeathMessages ? Team::NEVER : Team::ALWAYS);
            team->setSeeFriendlyInvisibles(false);
            team->syncChanges();
        }
    }

    if (scoreboard->getPlayerTeam(kSpectatorTeamName) == 0)
    {
        PlayerTeam* spectatorTeam = scoreboard->addPlayerTeam(kSpectatorTeamName);
        spectatorTeam->setNameTagVisibility(Team::HIDE_FOR_OTHER_TEAMS);
        spectatorTeam->setDeathMessageVisibility(hideDeathMessages ? Team::NEVER : Team::ALWAYS);
        spectatorTeam->setSeeFriendlyInvisibles(false);
        spectatorTeam->syncChanges();
    }
}

void MasterGameMode::SelectNewGameRules()
{
    ChooseNextGameRules(false);
    if (mCurrentLevelGenOptions == 0)
        return;

    mCurrentLevelGenOptions->loadBaseSaveData();
    LevelRuleset* ruleset = mCurrentLevelGenOptions->getRequiredGameRules();
    const MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(
        static_cast<EMiniGameId>(ruleset->getRuleType()), true);
    const int lootSet = ChooseItemSet(*miniGame, true);
    miniGame->SetLootSet(lootSet);
    SetupMiniGameInstance(*miniGame, 0);
    SetupTeams();

    if (miniGame->GetId() != eMiniGameId_Build)
        return;

    int selectedTheme = 0;
    Voteable* voteable = mVoteables->getVoteable(3);
    if (voteable->getWinningVote(selectedTheme) == -1)
    {
        std::vector<int>& themeIds = WiiUBuildOffThemeIds(this);
        const int themeCount = static_cast<int>(themeIds.size());
        if (themeCount == 1)
            selectedTheme = themeIds.front();
        else if (themeCount > 1)
            selectedTheme = themeIds.at(mRandom.nextInt(themeCount - 1));
    }

    setChosenThemeWordId(selectedTheme);
    clearBuildOffVotes();
    GeneratePlaylistSyncInfo();

    if (gMinecraftServer == 0 || gMinecraftServer->getPlayers() == 0)
        return;

    arrayWithLength<unsigned char> syncData = WiiUPlaylistSyncData(this);
    boost::shared_ptr<Packet> mapPacket(
        new MapSelectInfoPacket(syncData, false));
    gMinecraftServer->getPlayers()->broadcastAll(mapPacket);
}

void MasterGameMode::GetKillBoxes(LevelGenerationOptions* options, std::vector<KillboxRuleDefinition*>& killboxes)
{
    LevelRuleset* ruleset = options->getRequiredGameRules();
    ruleset->getKillboxes(&killboxes);

    std::vector<AABB*> areas;
    ruleset = options->getRequiredGameRules();
    ruleset->getNamedAreas(0x10000000, &areas);

    for (std::vector<AABB*>::iterator areaIt = areas.begin(); areaIt != areas.end(); ++areaIt)
        killboxes.push_back(new KillboxRuleDefinition(*areaIt));
}

void MasterGameMode::ResetPlayers()
{
    PlayerList* playerList = gMinecraftServer != 0 ? gMinecraftServer->getPlayers() : 0;
    std::vector<boost::shared_ptr<ServerPlayer> >* players = playerList->getPlayers();

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;

        if (player->mInventory != 0)
            player->mInventory->setCarried(ItemInstance::EMPTY);

        player->getFoodData()->setExhaustion(0.0f);

        const MiniGameDef* miniGame = GetMiniGame();
        const MiniGameDef::FoodBalance* foodBalance = miniGame->GetFoodBalance();
        const unsigned int foodLevel = foodBalance->mMaxFoodEnabled
            ? static_cast<unsigned int>(foodBalance->mMaxFood)
            : FoodConstants::MaxFood();
        player->getFoodData()->setFoodLevel(static_cast<int>(foodLevel));

        miniGame = GetMiniGame();
        foodBalance = miniGame->GetFoodBalance();
        const double saturation = foodBalance->mMaxSaturationEnabled
            ? static_cast<double>(foodBalance->mMaxSaturation)
            : FoodConstants::MaxSaturation();
        player->getFoodData()->setSaturation(static_cast<float>(saturation));
    }

    memset(WiiUSpawnDistributionData(this).data, 0xff, WiiUSpawnDistributionData(this).length);
    memset(WiiUAlternateSpawnDistributionData(this).data, 0xff,
           WiiUAlternateSpawnDistributionData(this).length);
    instructionSynchronize();
}

bool MasterGameMode::IsChunkWithinActiveBounds(void* context, int chunkX, int chunkZ)
{
    MasterGameMode* gameMode = static_cast<MasterGameMode*>(context);
    return gameMode != 0 && gameMode->IsChunkWithinLobbyRegionOfRoundLevel(chunkX, chunkZ);
}

void MasterGameMode::GenerateChestRefillInfo()
{
    const MiniGameDef* miniGame = GetMiniGame();
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    ServerLevel* level = gMinecraftServer != 0 ? gMinecraftServer->getLevel(0) : 0;
    if (miniGame == 0 || ruleset == 0 || level == 0)
        return;

    const std::wstring centralLootGroup = miniGame->getCentralLootGroup();
    const std::wstring outerLootGroup = miniGame->getOuterLootGroup();
    const std::wstring hvLootGroup = miniGame->getHVLootGroup();

    GameRuleDefinition* compoundRule = ruleset->getRuleByType(
        static_cast<ConsoleGameRules::EGameRuleType>(30));
    if (compoundRule == 0)
        return;

    for (std::vector<ChestRefillInfo>::iterator chestIt = mChestRefills.begin();
         chestIt != mChestRefills.end(); ++chestIt)
        ChestLootGroup(*chestIt).clear();
    mChestRefills.clear();

    std::vector<GameRuleDefinition*> distributeRules;
    compoundRule->getChildren(&distributeRules,
                                static_cast<ConsoleGameRules::EGameRuleType>(14));

    const std::wstring startItems(L"StartItems");
    const std::wstring hvItems(L"HVItems");
    const std::wstring outerItems(L"OuterItems");

    for (std::vector<GameRuleDefinition*>::iterator ruleIt = distributeRules.begin();
         ruleIt != distributeRules.end(); ++ruleIt)
    {
        DistributeItemsRuleDefinition* distributeRule =
            dynamic_cast<DistributeItemsRuleDefinition*>(*ruleIt);
        if (distributeRule == 0)
            continue;

        const std::wstring& ruleId = distributeRule->getId();
        const std::wstring* assignedLootGroup = 0;
        if (StringsEqualIgnoreCase(ruleId, startItems))
            assignedLootGroup = &centralLootGroup;
        else if (StringsEqualIgnoreCase(ruleId, hvItems))
            assignedLootGroup = &hvLootGroup;
        else if (StringsEqualIgnoreCase(ruleId, outerItems))
            assignedLootGroup = &outerLootGroup;
        else
            continue;

        std::vector<GameRuleDefinition*> positionRules;
        distributeRule->getChildren(&positionRules);
        for (std::vector<GameRuleDefinition*>::iterator positionIt = positionRules.begin();
             positionIt != positionRules.end(); ++positionIt)
        {
            WorldPositionRuleDefinition* positionRule =
                dynamic_cast<WorldPositionRuleDefinition*>(*positionIt);
            if (positionRule == 0)
                continue;

            const BlockPos position = WorldPositionFromRule(positionRule);
            ChestRefillInfo info;
            info.distance = -1.0f;
            info.weight = 0.0f;
            info.x = position.x;
            info.y = static_cast<short>(position.y);
            info.z = position.z;
            ChestLootGroup(info) = *assignedLootGroup;
            mChestRefills.push_back(info);
        }
    }

    instructionSynchronize();
}

bool MasterGameMode::LoadNewGameRules()
{
    mPlaylistDirty = false;
    return !mPlaylist.empty();
}

void MasterGameMode::SetLockPlayerPositions(bool locked)
{
    GameModePacket::LockPlayerLocation data(locked);
    boost::shared_ptr<Packet> packet(
        GameModePacket::Create(static_cast<GameModePacket::EMessage>(0x13), data));
    if (gMinecraftServer != 0)
        gMinecraftServer->getPlayers()->broadcastAll(packet);

    if (gMinecraftServer == 0)
        return;

    PlayerList* playerList = gMinecraftServer->getPlayers();
    std::vector<boost::shared_ptr<ServerPlayer> >* players = playerList->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        player->SetPositionLocked(locked);
    }
}

void MasterGameMode::UpdateGeneratorSeed()
{
    Random random;
    const long long seed = random.nextLong();

    int scaledPlayerCount = GetPlayerCount(false, false);
    if (scaledPlayerCount > 16)
        scaledPlayerCount = 16;
    if (scaledPlayerCount < 4)
        scaledPlayerCount = 4;

    const float scaleRatio =
        sqrtf(static_cast<float>(static_cast<double>(scaledPlayerCount))) / sqrtf(16.0f);
    const MiniGameDef* miniGame = GetMiniGame();
    const float scale = static_cast<float>(miniGame->GetLayerScale(scaleRatio));

    CommonMasterGameMode::UpdateGeneratorSeed(seed, scale);
    instructionSynchronize();
}

int MasterGameMode::GeneratorSpawn() const
{
    return mGeneratorSpawn;
}

boost::shared_ptr<ServerPlayer> MasterGameMode::GetRoundWinner()
{
    std::vector<boost::shared_ptr<ServerPlayer> > players = GetPlayers(false, false);
    return players.empty() ? boost::shared_ptr<ServerPlayer>() : players[0];
}

int MasterGameMode::GetSpawnLayerIndex() const
{
    return mSpawnLayerIndex;
}

void MasterGameMode::PlacePlayers(std::vector<boost::shared_ptr<ServerPlayer> >* players, bool atStart, bool includeSpectators)
{
    if (players == 0)
        return;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (player == 0)
            continue;
        if (player->mField0x82c != 0)
            player->StopSpectatingPlayer();
    }

    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (ruleset == 0)
        return;

    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame != 0 && miniGame->IsTeamBasedSpawn())
    {
        std::vector<AABB*> teamAreas;
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
             playerIt != players->end(); ++playerIt)
        {
            boost::shared_ptr<ServerPlayer> player = *playerIt;
            if (player == 0)
                continue;

            unsigned int teamIndex = 0;
            Team* team = player->getTeam();
            if (team != 0)
                teamIndex = team->getTeamIndex();

            teamAreas.clear();
            ruleset->getNamedAreas(1 << (teamIndex & 0x3f), &teamAreas);

            AABB* spawnArea = 0;
            if (teamAreas.size() > 1)
                spawnArea = teamAreas.at(mRandom.nextInt(static_cast<int>(teamAreas.size()) - 1));
            else if (!teamAreas.empty())
                spawnArea = teamAreas.front();

            if (spawnArea != 0)
                GameModeUtilities::SpawnWithinArea(player, spawnArea);
        }
        return;
    }

    if (Minecraft::InMiniGame(eMiniGameId_Glide, false) && miniGame != 0 &&
        miniGame->IsCheckpointBasedSpawn())
    {
        LevelGenerationOptions* generationOptions = gApp.getLevelGenerationOptions();
        if (generationOptions == 0)
            return;

        LevelRuleset* roundRules = generationOptions->getRequiredGameRules();
        if (roundRules == 0)
            return;

        std::vector<CheckpointRuleDefinition*>* checkpoints = roundRules->getCheckpoints();
        if (checkpoints == 0)
            return;

        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
             playerIt != players->end(); ++playerIt)
        {
            boost::shared_ptr<ServerPlayer> player = *playerIt;
            if (player == 0)
                continue;

            ServerPlayerGameMode* playerGameMode = player->GetGameMode();
            if (playerGameMode == 0)
                continue;

            if (atStart)
                playerGameMode->setLastCheckpoint(0);

            unsigned int checkpointIndex = playerGameMode->getLastCheckpoint();
            while (true)
            {
                if (static_cast<int>(checkpointIndex) < 0)
                    break;

                if (checkpointIndex < checkpoints->size())
                {
                    CheckpointRuleDefinition* checkpointRule = (*checkpoints)[checkpointIndex];
                    if (checkpointRule != 0 && checkpointRule->hasSpawnPoint())
                    {
                        checkpointRule->setPlayerPosition(player);
                        player->stopFallFlying();
                        mGameStats.RecordSpawnedAtCheckpoint(
                            player->getSUID(), checkpointRule->getSequenceID());
                        playerGameMode->RecordLastCheckpoint(
                            static_cast<int>(checkpointIndex),
                            checkpointRule->getSequenceID(), false, true, true);
                        ResetLastKnownPosition(player->mPlayerIndex);
                        break;
                    }
                }

                if (checkpointIndex == 0)
                    break;
                --checkpointIndex;
            }
        }
        return;
    }

    OnGameStartSpawnPositions* spawnPositions =
        static_cast<OnGameStartSpawnPositions*>(ruleset->getRuleByType(
            static_cast<ConsoleGameRules::EGameRuleType>(0x1d)));
    if (spawnPositions != 0)
    {
        unsigned char spawnType = 0;
        if (!includeSpectators && miniGame != 0 &&
            miniGame->HostOption_Bool(eCustomGameModeHostOption_CentreSpawn))
        {
            spawnType = 1;
        }

        SpawnPositionSet* primarySpawnSet = spawnPositions->getSpawnSet(spawnType ^ 1);
        SpawnPositionSet* winnerSpawnSet = spawnPositions->getSpawnSet(2);
        std::vector<GameRuleDefinition*> primaryRules;
        if (primarySpawnSet != 0)
            primarySpawnSet->getChildren(&primaryRules);

        if (winnerSpawnSet == 0)
        {
            GameModeUtilities::DistributePlayers(
                players, &primaryRules, &WiiUSpawnDistributionData(this));
            return;
        }

        std::vector<boost::shared_ptr<ServerPlayer> > winnerPlayers;
        std::vector<boost::shared_ptr<ServerPlayer> > otherPlayers;
        boost::shared_ptr<ServerPlayer> roundWinner = GetRoundWinner();

        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
             playerIt != players->end(); ++playerIt)
        {
            boost::shared_ptr<ServerPlayer> player = *playerIt;
            bool placeWithWinner = false;
            if (roundWinner != 0 && player != 0)
            {
                placeWithWinner = roundWinner->mPlayerIndex == player->mPlayerIndex &&
                                  CGameNetworkManager::sInstance.GetPlayerCount() > 1;
            }

            if (placeWithWinner)
                winnerPlayers.push_back(*playerIt);
            else
                otherPlayers.push_back(*playerIt);
        }

        std::vector<GameRuleDefinition*> winnerRules;
        winnerSpawnSet->getChildren(&winnerRules);
        GameModeUtilities::DistributePlayers(&winnerPlayers, &winnerRules, 0);
        GameModeUtilities::DistributePlayers(
            &otherPlayers, &primaryRules, &WiiUSpawnDistributionData(this));
        return;
    }

    AABB* spawnArea = 0;
    if (GeneratorSpawn() == 0)
    {
        LevelGenerationOptions* generationOptions = gApp.getLevelGenerationOptions();
        if (generationOptions != 0)
        {
            LevelRuleset* roundRules = generationOptions->getRequiredGameRules();
            if (roundRules != 0)
                spawnArea = roundRules->getNamedArea(L"RoundSpawn");
        }
    }
    else
    {
        spawnArea = reinterpret_cast<AABB*>(GeneratorSpawn());
    }

    if (spawnArea == 0)
        return;

    TumbleGenerator* tumbleGenerator = dynamic_cast<TumbleGenerator*>(mMapGenerator);
    Level* level = gMinecraftServer != 0 ? gMinecraftServer->getLevel(0) : 0;
    GameModeUtilities::SpreadPlayers(players, spawnArea, 3.0, level, false, tumbleGenerator);
}

void MasterGameMode::PositionPlayersAtStartingPosition()
{
    mPlaylistDirty = true;
}

void MasterGameMode::RemoveAllEntities(bool keepPlayers)
{
    mPlaylistDirty = true;
}

void MasterGameMode::AddPlayersToTeams()
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnWaitingToStartCountdown(ETimerID timer, MasterGameMode* gameMode, void* context)
{
    if (gameMode != 0)
        gameMode->CompleteTimer(timer);
}

bool MasterGameMode::CanRoundStart()
{
    return mReadyPlayers >= mRequiredPlayers &&
           GetPlayerCount(false, false) >= mMinimumPlayers;
}

void MasterGameMode::SetGameMode(const GameType* gameType)
{
    mPlayersPrepared = gameType != 0;
}

void MasterGameMode::ResetPlayersFlyingState(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        player->stopFallFlying();
    }

    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame == 0 || !miniGame->HasFeatureElytraFlyingOnSpawn())
        return;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;

        AABB* jumpOffPoint = 0;
        LevelGenerationOptions* generationOptions = gApp.getLevelGenerationOptions();
        if (generationOptions != 0)
        {
            LevelRuleset* ruleset = generationOptions->getRequiredGameRules();
            if (ruleset != 0)
                jumpOffPoint = ruleset->getNamedArea(L"JumpOffPoint");
        }

        bool startFlying = true;
        if (jumpOffPoint != 0)
        {
            Vec3* position = player->getPos(1.0f);
            startFlying = !jumpOffPoint->containsIncludingLowerBound(position);
        }

        if (startFlying)
        {
            player->startFallFlying();
            player->SetGlideCollisionDamage(true);
        }
    }

    instructionSynchronize();
}

void MasterGameMode::ResetPlayerHealths(std::vector<boost::shared_ptr<ServerPlayer> >* players, float health)
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (health < 0.0f)
            player->setHealth(player->getMaxHealth());
        else
            player->setHealth(health);
    }
    instructionSynchronize();
}

void MasterGameMode::SetPlayerAbilities(boost::shared_ptr<ServerPlayer> player, const Abilities* abilities)
{
    if (!player->isSpectator())
    {
        player->mAbilities = *abilities;
        player->onUpdateAbilities();
    }
}

void MasterGameMode::SetPlayersAbilities(std::vector<boost::shared_ptr<ServerPlayer> >* players, const Abilities* abilities)
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        SetPlayerAbilities(player, abilities);
    }
}

void MasterGameMode::UpdatePlayersAbilities(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    const MiniGameDef* miniGame = GetMiniGame();
    const Abilities abilities = miniGame->getPlayerAbilities();
    const unsigned int miniGameId = miniGame->GetId();

    if (miniGameId == eMiniGameId_Tumble)
        ResetPlayerHealths(players, 1.0f);
    else if (miniGameId == eMiniGameId_Glide)
        ResetPlayerHealths(players, 3.0f);
    else
        ResetPlayerHealths(players, -1.0f);

    SetPlayersAbilities(players, &abilities);
}

bool MasterGameMode::ShouldRemoveArmourPiece(int itemId)
{
    if (itemId != 0x18d && itemId != 0x56)
        return true;
    return false;
}

void MasterGameMode::EmptyPlayerInventory(std::vector<boost::shared_ptr<ServerPlayer> >* players, bool includeArmour)
{
    if (players == 0)
        return;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (player == 0 || player->mInventory == 0)
            continue;

        Inventory* inventory = player->mInventory.get();
        if (includeArmour)
        {
            inventory->dropAll();
            continue;
        }

        for (unsigned int slot = 0; slot < 36; ++slot)
            inventory->setItem(slot, ItemInstance::EMPTY);

        for (unsigned int armorSlot = 0; armorSlot < 4; ++armorSlot)
        {
            not_null_ptr<ItemInstance> armorItem = inventory->getArmor(armorSlot);
            if (armorItem->isEmpty())
                continue;

            if (ShouldRemoveArmourPiece(Item::getId(armorItem->getItem())))
                inventory->setItem(100 + armorSlot, ItemInstance::EMPTY);
        }

        inventory->setCarried(ItemInstance::EMPTY);
    }
}

void MasterGameMode::ResetPlayersEquipmentAndInventory(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    if (players == 0)
        return;

    EmptyPlayerInventory(players, false);

    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame != 0)
    {
        const std::wstring lootGroup = miniGame->getCentralLootGroup();
        if (miniGame->HasLootGroup(lootGroup))
        {
            std::vector<boost::shared_ptr<Container> > containers;
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
                 playerIt != players->end(); ++playerIt)
            {
                boost::shared_ptr<ServerPlayer> player = *playerIt;
                if (player == 0 || player->mInventory == 0)
                    continue;
                containers.push_back(
                    boost::static_pointer_cast<Container>(player->mInventory));
            }

            miniGame->PopulateContainers(&containers, lootGroup);
            miniGame->EquipPlayers(players);
        }
    }

    if (mRoundTimeOffset == 0 && mPausedRoundTime == 0)
        return;

    if (miniGame == 0 || miniGame->GetId() != eMiniGameId_Tumble)
        return;

    if (miniGame->GetGameVariant() != 0)
    {
        if (!miniGame->IsCustomGame())
            return;
        if (miniGame->GetLootSetId() != 0)
            return;
    }

    not_null_ptr<ItemInstance> elytraItem(new ItemInstance(Items::elytra));
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (player == 0)
            continue;
        player->setItemSlot(EquipmentSlot::CHEST, elytraItem);
    }
}

void MasterGameMode::PreparePlayersForGamemode(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    mPlaylistDirty = true;
}

void MasterGameMode::ResetGameRules()
{
    mPlaylistDirty = true;
}

void MasterGameMode::SetPlayerInvulnerable(boost::shared_ptr<ServerPlayer> player, bool invulnerable)
{
    bool applyInvulnerable = true;
    if (!invulnerable)
    {
        const Abilities abilities = GetMiniGame()->getPlayerAbilities();
        if (!abilities.mInvulnerable)
            applyInvulnerable = false;
    }

    player->setInvulnerable(applyInvulnerable);
    player->mAbilities.mInvulnerable = applyInvulnerable;

    gMinecraftServer->getPlayers()->broadcastAll(
        boost::shared_ptr<Packet>(
            new ClientboundPlayerAbilitiesPacket(&player->mAbilities, player->getId())));
}

void MasterGameMode::SetPlayersInvulnerable(bool invulnerable)
{
    WiiUPlayersInvulnerable(this) = invulnerable;

    PlayerList* playerList = gMinecraftServer != 0 ? gMinecraftServer->getPlayers() : 0;
    std::vector<boost::shared_ptr<ServerPlayer> >* players = playerList->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (!player->isSpectator())
            SetPlayerInvulnerable(player, invulnerable);
    }
}

void MasterGameMode::OnRefillChestTimer(MasterGameMode* gameMode, void* context)
{
    if (gameMode == 0)
        return;

    gameMode->sendTutorialUpdate(static_cast<eTutorial_GameMode>(4));

    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame == 0 || !miniGame->IsChestRefillEnabled())
        return;

    if (gMinecraftServer == 0)
        return;

    ServerLevel* level = gMinecraftServer->getLevel(0);
    if (level == 0 || gameMode->mChestRefills.empty())
        return;

    const unsigned int centralChance = miniGame->getCentralRefillChance();
    const unsigned int outerChance = miniGame->getOuterRefillChance();
    const unsigned int hvChance = miniGame->getHVRefillChance();
    const int chestsToRefill = static_cast<int>(miniGame->getNumberOfChestsToRefill());
    const float minDistance = HostChanceToFloat(miniGame->getMinPlayerDistanceToEffectChance());
    const float maxDistance = HostChanceToFloat(miniGame->getMaxPlayerDistanceToEffectChance());
    const float centralScale = HostChanceToFloat(centralChance);
    const float outerScale = HostChanceToFloat(outerChance);
    const float hvScale = HostChanceToFloat(hvChance);
    const std::wstring startItems(L"StartItems");
    const std::wstring hvItems(L"HVItems");
    const std::wstring outerItems(L"OuterItems");

    PlayerList* playerList = gMinecraftServer->getPlayers();
    if (playerList == 0)
        return;

    std::vector<boost::shared_ptr<ServerPlayer> >* players = playerList->getPlayers();
    if (players == 0)
        return;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (player == 0 || player->isSpectator())
            continue;

        for (std::vector<ChestRefillInfo>::iterator chestIt = gameMode->mChestRefills.begin();
             chestIt != gameMode->mChestRefills.end(); ++chestIt)
        {
            ChestRefillInfo& chest = *chestIt;
            Vec3 delta(static_cast<double>(player->mX - static_cast<double>(chest.x)),
                       static_cast<double>(player->mY - static_cast<double>(chest.y)),
                       static_cast<double>(player->mZ - static_cast<double>(chest.z)));
            const float distance = static_cast<float>(delta.length());
            if (chest.distance != -1.0f && distance >= chest.distance)
                continue;

            chest.distance = distance;
            float effectiveDistance = maxDistance;
            if (distance - maxDistance < 0.0f)
                effectiveDistance = distance;
            if (effectiveDistance - minDistance < 0.0f)
                effectiveDistance = minDistance;

            const std::wstring& lootGroup = ChestLootGroup(chest);
            if (StringsEqualIgnoreCase(lootGroup, startItems))
                chest.weight = effectiveDistance * centralScale;
            else if (StringsEqualIgnoreCase(lootGroup, hvItems))
                chest.weight = effectiveDistance * hvScale;
            else if (StringsEqualIgnoreCase(lootGroup, outerItems))
                chest.weight = effectiveDistance * outerScale;
            else
                chest.weight = effectiveDistance;
        }
    }

    double totalWeight = 0.0;
    for (std::vector<ChestRefillInfo>::iterator chestIt = gameMode->mChestRefills.begin();
         chestIt != gameMode->mChestRefills.end(); ++chestIt)
        totalWeight += static_cast<double>(chestIt->weight);

    Random random;
    std::vector<BlockPos> refillPositions;
    std::vector<boost::shared_ptr<Container> > refillContainers;
    std::vector<std::wstring> refillLootGroups;
    int selectedCount = 0;
    int attemptCount = 0;
    while (attemptCount < chestsToRefill * 3 && selectedCount < chestsToRefill)
    {
        double roll = static_cast<double>(random.nextFloat()) * totalWeight;
        bool chestChosen = false;
        for (std::vector<ChestRefillInfo>::iterator chestIt = gameMode->mChestRefills.begin();
             chestIt != gameMode->mChestRefills.end(); ++chestIt)
        {
            roll -= static_cast<double>(chestIt->weight);
            if (roll > 0.0)
                continue;

            BlockPos position(chestIt->x, chestIt->y, chestIt->z);
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(position);
            if (blockEntity == 0)
                break;

            boost::shared_ptr<Container> container =
                boost::dynamic_pointer_cast<Container>(blockEntity);
            if (container == 0 || container->isFull())
                break;

            refillPositions.push_back(position);
            refillContainers.push_back(container);
            refillLootGroups.push_back(ChestLootGroup(*chestIt));
            chestChosen = true;
            ++selectedCount;
            break;
        }

        if (!chestChosen)
            break;
        ++attemptCount;
    }

    for (size_t index = 0; index < refillContainers.size(); ++index)
    {
        std::vector<boost::shared_ptr<Container> > containers;
        containers.push_back(refillContainers[index]);
        miniGame->PopulateContainers(&containers, refillLootGroups[index]);
    }

    if (!refillPositions.empty() && playerList != 0)
    {
        GameModePacket::ChestRefill data(refillPositions);
        playerList->broadcastAll(GameModePacket::Create(
            static_cast<GameModePacket::EMessage>(0x14), data));
    }
}

void MasterGameMode::OnGracePeriodEnd(MasterGameMode* gameMode, void* context)
{
    boost::shared_ptr<Packet> chatPacket(
        new ClientboundChatPacket(
            static_cast<ClientboundChatPacket::EChatPacketMessage>(0x7c), -1));

    if (gMinecraftServer != 0)
        gMinecraftServer->getPlayers()->broadcastAll(chatPacket);

    gameMode->SetPlayersInvulnerable(false);
}

bool MasterGameMode::StartTimer(ETimerID timer, CountdownInfo* countdown, int duration)
{
    if (countdown == 0 || duration < 0)
        return false;
    AddTimer(timer, &MasterGameMode::OnTimerCountdown, countdown,
             duration, -1, -1, -1);
    return true;
}

void MasterGameMode::StartTimer(ETimerID timer, CountdownInfo* countdown)
{
    if (countdown != 0)
        AddTimer(timer, &MasterGameMode::OnTimerCountdown, countdown,
                 countdown->arguments.empty() ? 0 : countdown->arguments[0],
                 -1, -1, -1);
}

void MasterGameMode::OnGameStart(MasterGameMode* gameMode, void* context)
{
    if (gameMode != 0)
        gameMode->SetState(eInternalGameModeState_RoundPlaying);
}

void MasterGameMode::SetLastKnownPosition(boost::shared_ptr<Player> player)
{
    const int playerIndex = player->mPlayerIndex;
    Vec3* position = player->getPos(1.0f);
    if (mLastKnownPositions[playerIndex] == 0)
        mLastKnownPositions[playerIndex] = Vec3::newPermanent(position->x, position->y, position->z);
    else
        mLastKnownPositions[playerIndex]->set(position);
}

void MasterGameMode::UpdateClientsCameras(boost::shared_ptr<ServerPlayer> player)
{
    mPlaylistDirty = true;
}

void MasterGameMode::Tick_Round_Playing()
{
    std::vector<boost::shared_ptr<ServerPlayer> > serverPlayers = GetPlayers(true, false);
    std::vector<boost::shared_ptr<Player> > players;
    players.insert(players.end(), serverPlayers.begin(), serverPlayers.end());

    const MiniGameDef* miniGame = GetMiniGame();
    CheckCheckpoints(players);
    CheckTargets(players);
    CheckPowerups(players);
    CheckProgress(players);

    const bool pointsActive = miniGame != 0 && miniGame->ArePointsActive();
    mGameStats.RecordTimeSpentInPosition(pointsActive);

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = serverPlayers.begin();
         playerIt != serverPlayers.end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (player == 0)
            continue;

        const StatsUID suid = player->getSUID();

        if (miniGame != 0 && miniGame->GetId() == eMiniGameId_Glide)
        {
            AABB* jumpOffPoint = 0;
            LevelGenerationOptions* generationOptions = gApp.getLevelGenerationOptions();
            if (generationOptions != 0)
            {
                LevelRuleset* ruleset = generationOptions->getRequiredGameRules();
                if (ruleset != 0)
                    jumpOffPoint = ruleset->getNamedArea(L"JumpOffPoint");
            }

            bool trackGlideStats = mGameStats.GetEndZoneEntered(suid) == 0;
            if (trackGlideStats && jumpOffPoint != 0)
            {
                Vec3* position = player->getPos(1.0f);
                if (jumpOffPoint->containsIncludingLowerBound(position))
                    trackGlideStats = false;
            }

            if (trackGlideStats)
            {
                bool colliding = false;
                if (player->mBoundingBox != 0)
                {
                    AABB* collisionBox = player->mBoundingBox->grow(2.0f);
                    Level* level = player->getLevel();
                    if (level != 0 && collisionBox != 0)
                    {
                        std::vector<AABB*>* collisions =
                            level->getCollisionAABBs(player, collisionBox, false, false, false);
                        colliding = collisions != 0 && !collisions->empty();
                    }
                }
                mGameStats.TakeTimestamp(suid, static_cast<eTimeSpentCategory>(4), colliding);
                mGameStats.TakeTimestamp(suid, static_cast<eTimeSpentCategory>(5),
                                         player->isFallFlying());
            }

            bool shouldFallFly = trackGlideStats && !player->isFallFlyingStateDisabled();
            if (shouldFallFly)
                player->startFallFlying();
            else
                player->stopFallFlying();

            const bool endZoneEntered = mGameStats.GetEndZoneEntered(suid) != 0;
            if (endZoneEntered)
            {
                if (player->mFallFlying)
                    SetPlayerInvulnerable(player, endZoneEntered);
            }
            else if (player->mFallFlying)
            {
                if (!player->CheckPowerup(PowerupItems::eGlide_Timed_Powerup_ID_2))
                    SetPlayerInvulnerable(player, false);
            }

            unsigned int farthestCheckpoint = mGameStats.GetFarthestCheckpointReached();
            if (mCurrentCheckpoint < farthestCheckpoint)
            {
                unsigned int checkpointIndex = mCurrentCheckpoint;
                if (checkpointIndex < farthestCheckpoint + 3)
                {
                    do
                    {
                        std::vector<CheckpointRuleDefinition*>& checkpoints = mCheckpoints;
                        if (!checkpoints.empty() && checkpointIndex < checkpoints.size() &&
                            checkpoints[checkpointIndex] != 0 && gMinecraftServer != 0)
                        {
                            ServerLevel* level = gMinecraftServer->getLevel(0);
                            if (level != 0)
                                checkpoints[checkpointIndex]->processBeacons(level);
                        }
                        ++checkpointIndex;
                    } while (checkpointIndex < mGameStats.GetFarthestCheckpointReached() + 3);
                    farthestCheckpoint = mGameStats.GetFarthestCheckpointReached();
                }
                mCurrentCheckpoint = farthestCheckpoint;
            }
        }

        bool wasTracked = false;
        for (std::vector<StatsUID>::iterator trackedIt = mLastKnownPositionPlayers.begin();
             trackedIt != mLastKnownPositionPlayers.end(); ++trackedIt)
        {
            if (*trackedIt == suid)
            {
                wasTracked = true;
                break;
            }
        }

        if (wasTracked && mGameStats.GetPlayerStats(suid) != 0)
        {
            ServerLevel* level = player->getLevel();
            if (level != 0)
            {
                ServerScoreboard* scoreboard = level->GetServerScoreboard();
                if (scoreboard != 0)
                {
                    scoreboard->OnPlayerStatsChanged(
                        suid, static_cast<int>(mGameStats.GetDeathCount(suid)));
                }
            }
        }
    }

    mLastKnownPositionPlayers.clear();

    std::vector<boost::shared_ptr<ServerPlayer> > activePlayers = GetPlayers(false, false);
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = activePlayers.begin();
         playerIt != activePlayers.end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (player == 0)
            continue;

        SetLastKnownPosition(player);

        if (miniGame != 0 && miniGame->GetId() == eMiniGameId_Glide &&
            !player->IsSpectatingOtherPlayer() && !player->SpectateAsOtherEntity())
        {
            boost::shared_ptr<ServerPlayer> trailingPlayer =
                mGameStats.GetTrailingPlayer(player);
            if (trailingPlayer != 0)
            {
                player->SetCamera(trailingPlayer);
                UpdateClientsCameras(player);
            }
        }
    }

    instructionSynchronize();
}

bool MasterGameMode::CanRoundEnd()
{
    return mRoundEnding || GetRoundTime() >= mShowdownTime;
}

bool MasterGameMode::CanStartShowdown()
{
    return !mShowdownStarted && GetShowdownTime() > 0;
}

std::vector<boost::shared_ptr<ServerPlayer> > MasterGameMode::GetGlidePlayers()
{
    std::vector<boost::shared_ptr<ServerPlayer> > players;
    std::vector<boost::shared_ptr<ServerPlayer> >* allPlayers = gMinecraftServer->getPlayers()->getPlayers();

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = allPlayers->begin();
         playerIt != allPlayers->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (!player->isSpectator())
        {
            players.push_back(player);
            continue;
        }

        PlayerStats* stats = mGameStats.GetPlayerStats(player->getSUID());
        if (stats != 0 && stats->getReachedEnd(mCurrentPlaylistEntry))
            players.push_back(player);
    }

    return players;
}

int MasterGameMode::GetShowdownTime()
{
    return mShowdownTime - mShowdownReduction;
}

void MasterGameMode::StartRoundEndTimer()
{
    if (GetShowdownTime() != 0)
        return;

    CountdownInfo* countdown = new CountdownInfo();
    countdown->message = 0x7a;

    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    const MiniGameDef::TimerSpec timerSpec = miniGame->getTimerSpec(eTimerID_RoundLimit);
    const int totalDuration = static_cast<int>(timerSpec.TotalDurationS());

    std::vector<boost::shared_ptr<ServerPlayer> > glidePlayers = GetGlidePlayers();
    const int reduction = static_cast<int>(glidePlayers.size());

    const int reductionSeconds = Mth::clamp(((4 - reduction) * totalDuration) / 3, 0, (totalDuration * 2) / 3);

    const int amount = reductionSeconds * 1000;
    const unsigned int low = static_cast<unsigned int>(mPausedRoundTime);
    mPausedRoundTime = static_cast<int>(low - amount);
    mRoundTimeOffset -= static_cast<int>((amount >> 31) + static_cast<unsigned int>(low < static_cast<unsigned int>(amount)));

    StartTimer(eTimerID_RoundLimit, countdown, totalDuration - reductionSeconds);
}

void MasterGameMode::OnShowdownStart()
{
    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame != 0 && miniGame->GetId() == eMiniGameId_Tumble)
    {
        bool giveShowdownItem = true;
        if (miniGame->GetGameVariant() != 0)
            giveShowdownItem = miniGame->IsCustomGame() && miniGame->GetLootSetId() == 0;

        if (giveShowdownItem && gMinecraftServer != 0 && gMinecraftServer->getPlayers() != 0)
        {
            std::vector<boost::shared_ptr<ServerPlayer> >* players =
                gMinecraftServer->getPlayers()->getPlayers();
            if (players != 0)
            {
                for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
                     playerIt != players->end(); ++playerIt)
                {
                    boost::shared_ptr<ServerPlayer> player = *playerIt;
                    if (player == 0 || player->mInventory == 0)
                        continue;

                    not_null_ptr<ItemInstance> item(new ItemInstance(Items::snowball, 1));
                    player->setSlot(player->mInventory->m_selected, item);
                }
            }
        }
    }

    if (gMinecraftServer != 0)
    {
        ServerLevel* level = gMinecraftServer->getLevel(0);
        if (level != 0)
        {
            Scoreboard* scoreboard = level->GetScoreboard();
            if (scoreboard != 0)
            {
                std::vector<PlayerTeam*>* teams = scoreboard->getPlayerTeams();
                if (teams != 0)
                {
                    for (std::vector<PlayerTeam*>::iterator teamIt = teams->begin();
                         teamIt != teams->end(); ++teamIt)
                    {
                        PlayerTeam* team = *teamIt;
                        if (team == 0)
                            continue;
                        if (team->getName().compare(kSpectatorTeamName) != 0)
                        {
                            team->setNameTagVisibility(Team::ALWAYS);
                            team->syncChanges();
                        }
                    }
                    delete teams;
                }
            }
        }
    }

    GameModePacket::EmptyData emptyData;
    BroadcastGameModePacket(static_cast<GameModePacket::EMessage>(0x16), emptyData);

    if (miniGame != 0 && miniGame->GetId() == eMiniGameId_Glide)
        StartRoundEndTimer();
}

std::vector<PlayerTeam*> MasterGameMode::GetActiveTeams()
{
    ServerLevel* level = gMinecraftServer->getLevel(0);
    Scoreboard* scoreboard = level->GetScoreboard();

    std::vector<boost::shared_ptr<ServerPlayer> > players = GetPlayers(true, false);
    std::vector<PlayerTeam*> teams;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players.begin();
         playerIt != players.end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        PlayerTeam* team = scoreboard->getPlayersTeam(player->mXuid);
        if (std::find(teams.begin(), teams.end(), team) == teams.end())
            teams.push_back(team);
    }

    return teams;
}

bool MasterGameMode::CanEndPostGame()
{
    return mPostGameTime <= 0;
}

bool MasterGameMode::ShouldPlayAnotherRound()
{
    const unsigned int miniGameId = GetMiniGame()->GetId();
    if (miniGameId == eMiniGameId_Battle || miniGameId == eMiniGameId_Glide)
    {
        if (mCurrentPlaylistEntry + 1 >= GetMiniGame()->GetRoundCount())
            return false;
        return GetMinimumPlayersToStart() <= static_cast<unsigned int>(GetPlayerCount(false, false));
    }
    if (miniGameId == eMiniGameId_Tumble)
    {
        std::vector<boost::shared_ptr<ServerPlayer> >* players = gMinecraftServer->getPlayers()->getPlayers();
        if (players != 0)
        {
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
                 playerIt != players->end(); ++playerIt)
            {
                boost::shared_ptr<ServerPlayer> player = *playerIt;
                if (mGameStats.GetRoundsWon(player->getSUID()) > 2)
                    return false;
            }
        }
        return GetMinimumPlayersToStart() <= static_cast<unsigned int>(GetPlayerCount(false, false));
    }
    return false;
}

void MasterGameMode::OnGameEnd(MasterGameMode* gameMode, void* context)
{
    boost::shared_ptr<Packet> chatPacket(
        new ClientboundChatPacket(
            static_cast<ClientboundChatPacket::EChatPacketMessage>(0x76), -1));

    if (gMinecraftServer != 0)
        gMinecraftServer->getPlayers()->broadcastAll(chatPacket);

    EInternalGameModeState nextState = static_cast<EInternalGameModeState>(0xd);
    if (gameMode->ShouldPlayAnotherRound())
        nextState = static_cast<EInternalGameModeState>(0xe);
    gameMode->SetState(nextState);
}

void MasterGameMode::ReturnPlayersToLobby()
{
    if (gMinecraftServer == 0)
        return;

    PlayerList* playerList = gMinecraftServer->getPlayers();
    ServerLevel* serverLevel = gMinecraftServer->getLevel(0);
    MinigamePlayerChunkMap* chunkMap = 0;
    if (serverLevel != 0)
        chunkMap = dynamic_cast<MinigamePlayerChunkMap*>(serverLevel->getChunkMap());

    *reinterpret_cast<int*>(reinterpret_cast<char*>(gMinecraftServer) + 0xf0) = 0;
    mCurrentRuleIndex = mCurrentPlaylistEntry;

    const MiniGameDef* lobbyDefinition =
        MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(0), true);
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0 && lobbyDefinition != 0)
        minecraft->SetupMiniGameInstance(*lobbyDefinition, 0);

    arrayWithLength<unsigned char>& spawnDistribution = WiiUSpawnDistributionData(this);
    arrayWithLength<unsigned char>& alternateDistribution =
        *reinterpret_cast<arrayWithLength<unsigned char>*>(
            reinterpret_cast<char*>(this) + 0x248);
    if (spawnDistribution.data != 0 && spawnDistribution.length != 0)
        std::memset(spawnDistribution.data, 0xff, spawnDistribution.length);
    if (alternateDistribution.data != 0 && alternateDistribution.length != 0)
        std::memset(alternateDistribution.data, 0xff, alternateDistribution.length);

    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame != 0)
    {
        GameModePacket::ChangeCustomGameMode changeData(miniGame->GetId(), -1);
        BroadcastGameModePacket(static_cast<GameModePacket::EMessage>(0x11), changeData);
    }

    MasterGameModeWiiUChunkBounds& chunkBounds = WiiUChunkBounds(this);
    GameModePacket::TransitionData transitionData(
        chunkBounds.mMinChunkX, chunkBounds.mMinChunkZ,
        chunkBounds.mMaxChunkX, chunkBounds.mMaxChunkZ);
    BroadcastGameModePacket(static_cast<GameModePacket::EMessage>(4), transitionData);

    if (chunkMap != 0)
    {
        chunkMap->removeOutsideAreaForAllPlayers(
            chunkBounds.mMinChunkX, chunkBounds.mMinChunkZ,
            chunkBounds.mMaxChunkX, chunkBounds.mMaxChunkZ);
    }

    if (mNextLevelGenerationOptions != 0)
    {
        std::wstring savePath = mNextLevelGenerationOptions->getBaseSaveRoot();
        savePath += mNextLevelGenerationOptions->getBaseSavePath();
        mNextLevelGenerationOptions->setBaseSaveData(File(savePath));
        gApp.setLevelGenerationOptions(mNextLevelGenerationOptions);
    }

    if (serverLevel != 0)
        serverLevel->refreshFromLevelGen();
    gRegionFileCache._clear();

    if (mNextLevelGenerationOptions == 0)
        return;

    std::vector<NamedAreaRuleDefinition*> boundsAreas;
    LevelRuleset* lobbyRules = mNextLevelGenerationOptions->getRequiredGameRules();
    if (lobbyRules != 0)
        lobbyRules->getPlayerBoundsVolumes(&boundsAreas);

    if (playerList != 0)
    {
        std::vector<boost::shared_ptr<ServerPlayer> >* players = playerList->getPlayers();
        if (players != 0)
        {
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
                 playerIt != players->end(); ++playerIt)
            {
                boost::shared_ptr<ServerPlayer> player = *playerIt;
                if (player == 0)
                    continue;

                if (miniGame != 0)
                {
                    maybe<int> playerGameType = miniGame->getPlayerGameType();
                    if (playerGameType.isSet())
                        player->setGameMode(
                            const_cast<GameType*>(GameType::byId(playerGameType.get())));
                }

                if (player->mConnection != 0)
                {
                    GameModePacket::EmptyData emptyData;
                    player->mConnection->send(
                        GameModePacket::Create(static_cast<GameModePacket::EMessage>(0xf), emptyData));

                    for (std::vector<NamedAreaRuleDefinition*>::iterator areaIt = boundsAreas.begin();
                         areaIt != boundsAreas.end(); ++areaIt)
                    {
                        if (*areaIt == 0)
                            continue;
                        AABB* area = (*areaIt)->getArea();
                        if (area == 0)
                            continue;
                        GameModePacket::AddAreaBoundsData boundsData(
                            static_cast<int>(area->mMinX), static_cast<int>(area->mMinY),
                            static_cast<int>(area->mMinZ), static_cast<int>(area->mMaxX),
                            static_cast<int>(area->mMaxY), static_cast<int>(area->mMaxZ));
                        player->mConnection->send(
                            GameModePacket::Create(static_cast<GameModePacket::EMessage>(0xd),
                                                   boundsData));
                    }
                }

                *reinterpret_cast<unsigned char*>(
                    reinterpret_cast<char*>(player.get()) + 0x936) = 1;
                playerList->respawn(player, 0, false);
            }
        }
    }

    if (serverLevel != 0)
    {
        serverLevel->invalidateChunksOutsideArea(
            chunkBounds.mMinChunkX, chunkBounds.mMinChunkZ,
            chunkBounds.mMaxChunkX, chunkBounds.mMaxChunkZ);
    }
    if (playerList != 0)
    {
        playerList->resetAllChunkFlagsOutsideArea(
            chunkBounds.mMinChunkX, chunkBounds.mMinChunkZ,
            chunkBounds.mMaxChunkX, chunkBounds.mMaxChunkZ, 0);
    }

    if (serverLevel != 0 && serverLevel->getChunkSource() != 0)
    {
        static_cast<ServerChunkCache*>(serverLevel->getChunkSource())
            ->setOverrideBoundFunction(0, 0);
    }

    mChunkHistory = mSpawnLayerIndex;

    AABB* activeArea = 0;
    if (lobbyRules != 0)
        activeArea = lobbyRules->getNamedArea(L"ActiveArea");
    if (activeArea != 0)
        mKillboxAreas.push_back(activeArea);

    mActiveChunkBounds.DeactivateCache();
    mActiveChunkBounds.ClearCache();

    if (chunkMap != 0)
    {
        chunkMap->reloadAllPlayers(
            chunkBounds.mMinChunkX, chunkBounds.mMinChunkZ,
            chunkBounds.mMaxChunkX, chunkBounds.mMaxChunkZ);
    }

    ResetReadyStates(false);
    if (playerList != 0)
    {
        std::vector<boost::shared_ptr<ServerPlayer> >* players = playerList->getPlayers();
        if (players != 0)
            EmptyPlayerInventory(players, true);
    }
}

void MasterGameMode::Tick()
{
    CommonMasterGameMode::Tick();
    TickTimers();
    if (mTutorialDirty)
        CheckTutorialUpdate();
    if (mRoundLevel != 0 && mRoundLevel->getServer() != 0 &&
        mRoundLevel->getServer()->getPlayers() != 0)
    {
        std::vector<boost::shared_ptr<ServerPlayer> >* players =
            mRoundLevel->getServer()->getPlayers()->getPlayers();
        if (players != 0)
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin();
                 it != players->end(); ++it)
                if (*it && IsInKillBox(*it))
                    (*it)->kill();
    }
    if (mBlockRemover != 0)
    {
        if (!mBlockRemover->Finished())
            mBlockRemover->Tick();
        else
        {
            delete mBlockRemover;
            mBlockRemover = 0;
        }
    }
    if (mInternalState == 0 && AreLevelChunksLoaded())
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        if (minecraft != 0 && minecraft->mTexturePackRepository != 0)
            minecraft->mTexturePackRepository->getSelected();
        mRuleGroups.clear();
        mShuffledRuleGroups.clear();
        mRuleGroups.insert(mRuleGroups.end(), mPlaylist.begin(), mPlaylist.end());
        mShuffledRuleGroups.insert(mShuffledRuleGroups.end(), mRuleGroups.begin(), mRuleGroups.end());
        std::random_shuffle(mShuffledRuleGroups.begin(), mShuffledRuleGroups.end());
        ChooseNextGameRules(true);
        SetState(static_cast<EInternalGameModeState>(2));
        sendTutorialUpdate(static_cast<eTutorial_GameMode>(0));
    }
}

void MasterGameMode::SetupServerSettings(Settings* settings)
{
    mPlaylistDirty = true;
}

const GameType* MasterGameMode::InitialGameType(boost::shared_ptr<ServerPlayer> player)
{
    return GameType::LOBBY;
}

bool MasterGameMode::IsInRound()
{
    return mInternalState >= eInternalGameModeState_RoundStarting &&
           mInternalState <= eInternalGameModeState_RoundEnding;
}

void MasterGameMode::PlaceSpectators(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    if (players == 0)
        return;

    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame == 0)
        return;

    if (!miniGame->ForceSpectateModeFollow() && ruleset != 0)
    {
        AABB* spectatorSpawn = ruleset->getNamedArea(L"SpectatorSpawn");
        if (spectatorSpawn == 0)
        {
            std::vector<GameRuleDefinition*> spawnRules;
            ruleset->getChildren(&spawnRules,
                                 static_cast<ConsoleGameRules::EGameRuleType>(0x1c));
            if (spawnRules.empty())
            {
                for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt =
                         players->begin();
                     playerIt != players->end(); ++playerIt)
                {
                    boost::shared_ptr<Player> player = *playerIt;
                    if (player != 0)
                        ruleset->postProcessPlayer(player);
                }
            }
            else
            {
                arrayWithLength<unsigned char>* distributionData =
                    reinterpret_cast<arrayWithLength<unsigned char>*>(
                        reinterpret_cast<char*>(this) + 0x248);
                GameModeUtilities::DistributePlayers(players, &spawnRules, distributionData);
            }
        }
        else
        {
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt =
                     players->begin();
                 playerIt != players->end(); ++playerIt)
            {
                GameModeUtilities::SpawnWithinArea(*playerIt, spectatorSpawn);
            }
        }
    }
    else
    {
        boost::shared_ptr<ServerPlayer> leadingPlayer = mGameStats.GetLeadingPlayer();
        if (leadingPlayer == 0)
        {
            std::vector<boost::shared_ptr<ServerPlayer> > activePlayers =
                GetPlayers(true, false);
            if (!activePlayers.empty())
                leadingPlayer = activePlayers[0];
        }

        if (leadingPlayer != 0)
        {
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt =
                     players->begin();
                 playerIt != players->end(); ++playerIt)
            {
                boost::shared_ptr<ServerPlayer> player = *playerIt;
                if (player != 0)
                    player->SetCamera(leadingPlayer);
            }
        }
    }

    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft == 0 || minecraft->GetSpectateMode() != 4)
        return;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (player == 0 || !player->SpectateAsSkull())
            continue;

        not_null_ptr<ItemInstance> helmet = player->getItemSlot(EquipmentSlot::HEAD);
        bool equipSkull = helmet->isEmpty();
        if (!equipSkull)
            continue;

        player->mField0x82c = 1;

        bool useBlockSkull = false;
        if (g_0x104C3EA1 != 0)
        {
            useBlockSkull = true;
        }
        else if (g_0x104C3EA2 != 0 && ruleset != 0)
        {
            LevelGenerationOptions* generationOptions = ruleset->getGenerationOptions();
            if (generationOptions != 0 &&
                generationOptions->getTexturePackId() == 0x405)
            {
                useBlockSkull = true;
            }
        }

        not_null_ptr<ItemInstance> skullItem;
        if (useBlockSkull)
            skullItem = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::skull));
        else
            skullItem = not_null_ptr<ItemInstance>(new ItemInstance(Items::skull));

        player->setItemSlot(EquipmentSlot::HEAD, skullItem);
    }
}

void MasterGameMode::OnSpawn(boost::shared_ptr<ServerPlayer> player, boost::shared_ptr<ServerPlayer> owner, bool joining)
{
    if (player == 0)
        return;

    if (!IsInRound())
    {
        std::vector<boost::shared_ptr<ServerPlayer> > players;
        players.push_back(player);
        PlacePlayers(&players, false, false);
        UpdateClientsCameras(player);
        return;
    }

    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    bool spawnedAsSpectator = false;
    if (ruleset != 0)
    {
        const MiniGameDef* miniGame = GetMiniGame();
        const unsigned int livesPerRound =
            miniGame != 0 ? miniGame->GetLivesPerRound() : 0;
        if (livesPerRound != 0)
        {
            const unsigned int deaths = player->mGameMode != 0
                ? player->mGameMode->GetDeaths() : 0;
            if (livesPerRound <= deaths)
            {
                if (gMinecraftServer != 0)
                {
                    ServerLevel* level = gMinecraftServer->getLevel(0);
                    if (level != 0)
                    {
                        Scoreboard* scoreboard = level->GetScoreboard();
                        if (scoreboard != 0)
                            scoreboard->removePlayerFromTeam(player->getSUID());
                    }
                }

                std::vector<boost::shared_ptr<ServerPlayer> > spectators;
                spectators.push_back(player);
                PlaceSpectators(&spectators);
                spawnedAsSpectator = true;
            }
        }

        if (!spawnedAsSpectator)
        {
            std::vector<boost::shared_ptr<ServerPlayer> > players;
            players.push_back(player);
            PlacePlayers(&players, false, true);
            ResetPlayersFlyingState(&players);
            ResetPlayersEquipmentAndInventory(&players);
        }
    }

    player->removeAllEffects();

    boost::shared_ptr<ServerPlayer> roundWinner = GetRoundWinner();
    if (roundWinner != 0 && player != roundWinner)
        player->setGameMode(const_cast<GameType*>(GameType::SPECTATOR));

    UpdateClientsCameras(player);
}

void MasterGameMode::OnSpawned(boost::shared_ptr<ServerPlayer> player, bool joining)
{
    if (!IsInRound())
        return;

    if (gApp.getGameRuleDefinitions() != 0)
    {
        const MiniGameDef* miniGame = GetMiniGame();
        const unsigned int livesPerRound = miniGame->GetLivesPerRound();
        bool grantAbilities = true;
        if (livesPerRound != 0)
        {
            const unsigned int deaths =
                player->mGameMode != 0 ? player->mGameMode->GetDeaths() : 0;
            grantAbilities = livesPerRound > deaths;
        }
        if (grantAbilities)
        {
            std::vector<boost::shared_ptr<ServerPlayer> > players;
            players.push_back(player);
            UpdatePlayersAbilities(&players);
        }
    }

    boost::shared_ptr<ServerPlayer> roundWinner = GetRoundWinner();
    if (roundWinner != 0 && player != roundWinner)
    {
        player->setGameMode(const_cast<GameType*>(GameType::SPECTATOR));
        UpdateClientsCameras(player);
    }
}

void MasterGameMode::SendPlaylistInfo(boost::shared_ptr<ServerPlayer> player)
{
    arrayWithLength<unsigned char> syncData = WiiUPlaylistSyncData(this);
    boost::shared_ptr<Packet> mapPacket(new MapSelectInfoPacket(syncData, false));
    player->mConnection->send(mapPacket);
}

void MasterGameMode::SetLockPlayerPosition(boost::shared_ptr<ServerPlayer> player, bool locked)
{
    GameModePacket::LockPlayerLocation data(locked);
    boost::shared_ptr<Packet> packet(
        GameModePacket::Create(static_cast<GameModePacket::EMessage>(0x13), data));
    player->mConnection->send(packet);
    player->SetPositionLocked(locked);
}

void MasterGameMode::OnJoin(boost::shared_ptr<ServerPlayer> player)
{
    if (player == 0)
        return;

    int customGameModeId = -1;
    LevelGenerationOptions* generationOptions = gApp.getLevelGenerationOptions();
    if (generationOptions != 0 && !generationOptions->isGameModeLobby())
        customGameModeId = static_cast<int>(generationOptions->getUniqueId());

    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame != 0 && player->mConnection != 0)
    {
        GameModePacket::ChangeCustomGameMode changeData(miniGame->GetId(), customGameModeId);
        SendGameModePacket(player, static_cast<GameModePacket::EMessage>(0x11), changeData);

        boost::shared_ptr<ClientboundMGPlayerSettingsUpdatePacket> settingsPacket =
            miniGame->GetPlayerSettingsPacket();
        if (settingsPacket != 0)
            player->mConnection->send(settingsPacket);
    }

    ServerLevel* level = player->getLevel();
    if (level != 0)
    {
        ServerScoreboard* scoreboard = level->GetServerScoreboard();
        if (scoreboard != 0)
        {
            mGameStats.RemovePlayerStats(player);
            std::vector<boost::shared_ptr<ServerPlayer> > players = GetPlayers(false, false);
            scoreboard->SyncPlayerScores(&players, player);
        }
    }

    SendPlaylistInfo(player);
    UpdateClientState(player);

    if (player->mConnection != 0)
    {
        for (int index = 0; index < 8; ++index)
        {
            boost::shared_ptr<Packet> readyPacket(
                new PlayerReadyPacket(static_cast<unsigned int>(index), mPlayerReady[index]));
            player->mConnection->send(readyPacket);
        }
    }

    if (mVoteables != 0)
        mVoteables->syncAllVoteablesToPlayer(player);

    const unsigned int state = static_cast<unsigned int>(GetState());
    const bool midRoundJoin = IsMidRoundJoinState(state);
    const bool spectator = player->GetGameType() == GameType::SPECTATOR || player->isSpectator();

    if (spectator)
    {
        const unsigned long long currentTime = GetCurrentTimeMillis();
        GameModePacket::UpdateStartTimeData startTimeData(static_cast<long long>(currentTime));
        SendGameModePacket(player, static_cast<GameModePacket::EMessage>(0x1c), startTimeData);

        player->setGameMode(const_cast<GameType*>(GameType::SPECTATOR));

        std::vector<NamedAreaRuleDefinition*> boundsAreas;
        if (generationOptions != 0)
        {
            LevelRuleset* rules = generationOptions->getRequiredGameRules();
            if (rules != 0)
                rules->getPlayerBoundsVolumes(&boundsAreas);
        }

        for (std::vector<NamedAreaRuleDefinition*>::iterator areaIt = boundsAreas.begin();
             areaIt != boundsAreas.end(); ++areaIt)
        {
            if (*areaIt == 0)
                continue;
            AABB* area = (*areaIt)->getArea();
            if (area == 0)
                continue;
            GameModePacket::AddAreaBoundsData boundsData(
                static_cast<int>(area->mMinX), static_cast<int>(area->mMinY), static_cast<int>(area->mMinZ),
                static_cast<int>(area->mMaxX), static_cast<int>(area->mMaxY), static_cast<int>(area->mMaxZ));
            SendGameModePacket(player, static_cast<GameModePacket::EMessage>(0xd), boundsData);
        }

        if (gMinecraftServer != 0)
        {
            ServerLevel* serverLevel = gMinecraftServer->getLevel(0);
            Scoreboard* scoreboard = serverLevel != 0 ? serverLevel->GetServerScoreboard() : 0;
            if (scoreboard != 0)
            {
                static const std::wstring emptyTeam;
                PlayerTeam* team = scoreboard->getPlayerTeam(emptyTeam);
                scoreboard->addPlayerToTeam(player->mXuid, team);
            }
        }

        std::vector<boost::shared_ptr<ServerPlayer> > spectatorPlayers;
        spectatorPlayers.push_back(player);
        PlaceSpectators(&spectatorPlayers);
    }
    else if (midRoundJoin)
    {
        std::vector<boost::shared_ptr<ServerPlayer> > joiningPlayers;
        joiningPlayers.push_back(player);
        PlacePlayers(&joiningPlayers, false, false);
        PreparePlayersForGamemode(&joiningPlayers);
        mGameStats.AddPlayerStats(player);
        SetPlayerInvulnerable(player, true);
        SetLockPlayerPosition(player, true);

        if (gMinecraftServer != 0)
        {
            ServerLevel* serverLevel = gMinecraftServer->getLevel(0);
            if (serverLevel != 0)
            {
                Scoreboard* scoreboard = serverLevel->GetServerScoreboard();
                GameModeUtilities::DistributePlayersInTeams(&joiningPlayers, scoreboard);
            }
        }
    }
    else
    {
        std::vector<boost::shared_ptr<ServerPlayer> > joiningPlayers;
        joiningPlayers.push_back(player);
        PlacePlayers(&joiningPlayers, false, false);
    }

    const int themeId = static_cast<int>(mRoundInfo.getThemeId());
    const int themeSubId = static_cast<int>(mRoundInfo.getThemeSubId());
    if (themeId >= 0 && themeSubId >= 0)
    {
        GameModePacket::UpdateThemeIdData themeData(themeId, themeSubId);
        SendGameModePacket(player, static_cast<GameModePacket::EMessage>(0x1b), themeData);
    }

    UpdateClientsCameras(player);

    std::vector<CheckpointRuleDefinition*>& checkpoints = mCheckpoints;
    for (std::vector<CheckpointRuleDefinition*>::iterator checkpointIt = checkpoints.begin();
         checkpointIt != checkpoints.end(); ++checkpointIt)
    {
        if (*checkpointIt == 0 || gMinecraftServer == 0)
            continue;
        ServerLevel* serverLevel = gMinecraftServer->getLevel(0);
        if (serverLevel != 0)
            (*checkpointIt)->processBeacons(serverLevel);
    }

    if (Minecraft::InMiniGame(static_cast<EMiniGameId>(1), true))
    {
        const unsigned int networkPlayerCount =
            static_cast<unsigned int>(CGameNetworkManager::sInstance.GetPlayerCount());
        if (miniGame != 0)
        {
            const unsigned int mapSizeOverride = miniGame->GetMapSizeOverride();
            if ((mapSizeOverride == 1 || mapSizeOverride == 2) && networkPlayerCount > 8)
            {
                MiniGamePlayerSettings settings;
                MiniGameDef::GetPlayerSettings(settings);
                if (settings.Get(eCustomGameModeHostOption_CentreSpawn) != 0)
                {
                    settings.Set(eCustomGameModeHostOption_CentreSpawn, 0);
                    MiniGameDef::SetPlayerSettings(settings);
                    if (gMinecraftServer != 0 && gMinecraftServer->getPlayers() != 0)
                    {
                        boost::shared_ptr<ClientboundMGPlayerSettingsUpdatePacket> hostSettings =
                            miniGame->GetPlayerSettingsPacket();
                        if (hostSettings != 0)
                            gMinecraftServer->getPlayers()->broadcastAll(hostSettings);
                    }
                }
            }
        }
    }
}

void MasterGameMode::OnFollow(boost::shared_ptr<ServerPlayer> player, bool started, bool force)
{
    if (!started)
    {
        if (player->IsSpectatingOtherPlayer())
            player->StopSpectatingPlayer();
    }
    else
    {
        uint32_t changed = 0;
        player->SpectatePlayerChange(reinterpret_cast<uint32_t>(&changed), static_cast<int32_t>(force));
        if (changed == 0)
            player->StopSpectatingPlayer();
    }

    UpdateClientsCameras(player);
}

void MasterGameMode::OnLeave(boost::shared_ptr<ServerPlayer> player)
{
    if (player == 0)
        return;

    const unsigned char playerIndex = static_cast<unsigned char>(player->mPlayerIndex);
    ClearSpawnDistributionSlot(WiiUSpawnDistributionData(this), playerIndex);
    ClearSpawnDistributionSlot(WiiUAlternateSpawnDistributionData(this), playerIndex);

    int& loadingPlayerCount = WiiULoadingPlayerCount(this);
    if (loadingPlayerCount > 0)
        --loadingPlayerCount;

    if (gMinecraftServer != 0)
    {
        ServerLevel* level = gMinecraftServer->getLevel(0);
        if (level != 0)
        {
            Scoreboard* scoreboard = level->GetServerScoreboard();
            if (scoreboard != 0)
                scoreboard->removePlayerFromTeam(player->getSUID());
        }
    }

    const StatsUID suid = player->getSUID();
    if (mGameStats.GetPlayerStats(suid) != 0)
    {
        ServerLevel* playerLevel = player->getLevel();
        if (playerLevel != 0)
        {
            ServerScoreboard* scoreboard = playerLevel->GetServerScoreboard();
            if (scoreboard != 0)
            {
                scoreboard->OnPlayerStatsChanged(
                    suid, static_cast<int>(mGameStats.GetDeathCount(suid)));
            }
        }
    }

    if (playerIndex < 8)
        mPlayerReady[playerIndex] = false;

    if (gMinecraftServer != 0 && gMinecraftServer->getPlayers() != 0)
    {
        boost::shared_ptr<Packet> readyPacket(
            new PlayerReadyPacket(static_cast<unsigned int>(playerIndex), false));
        gMinecraftServer->getPlayers()->broadcastAll(readyPacket);
    }

    if (mVoteables != 0)
    {
        mVoteables->getVoteable(0)->setIncludedVotesAndSync(
            static_cast<int>(playerIndex), static_cast<char>(-1));
        mVoteables->getVoteable(1)->setIncludedVotesAndSync(
            static_cast<int>(playerIndex), static_cast<int>(playerIndex), static_cast<char>(-1));
        if (Minecraft::InMiniGame(eMiniGameId_Build, true))
        {
            mVoteables->getVoteable(2)->setIncludedVotesAndSync(
                static_cast<int>(playerIndex), static_cast<int>(playerIndex), static_cast<char>(-1));
            mVoteables->getVoteable(3)->setIncludedVotesAndSync(
                static_cast<int>(playerIndex), static_cast<char>(-1));
        }
    }

    mGameStats.RemovePlayerStats(player);

    if (gMinecraftServer != 0 && gMinecraftServer->getPlayers() != 0)
    {
        std::vector<boost::shared_ptr<ServerPlayer> >* players =
            gMinecraftServer->getPlayers()->getPlayers();
        if (players != 0)
        {
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
                 playerIt != players->end(); ++playerIt)
            {
                boost::shared_ptr<ServerPlayer> otherPlayer = *playerIt;
                if (otherPlayer == 0 || otherPlayer == player)
                    continue;

                boost::shared_ptr<Entity> camera = otherPlayer->GetCamera();
                if (camera == player)
                    OnFollow(otherPlayer, true, false);
            }
        }
    }
}

void MasterGameMode::OnDeath(boost::shared_ptr<ServerPlayer> player)
{
    if (player == 0)
        return;

    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame != 0 && miniGame->GetLivesPerRound() != 0)
    {
        const unsigned int deaths =
            player->mGameMode != 0 ? player->mGameMode->GetDeaths() : 0;
        int remainingLives = static_cast<int>(miniGame->GetLivesPerRound()) -
                             static_cast<int>(deaths);
        if (remainingLives < 1)
        {
            if (player->mGameMode != 0)
                player->mGameMode->OnDeath(player);
            player->setGameMode(const_cast<GameType*>(GameType::SPECTATOR));
        }
        else
            player->sendMessage(0x79, &remainingLives, 1, 0, 0);
    }

    if (miniGame != 0 && miniGame->GetId() == eMiniGameId_Glide)
    {
        ServerLevel* level = player->getLevel();
        if (level != 0)
        {
            ServerScoreboard* scoreboard = level->GetServerScoreboard();
            if (scoreboard != 0)
                scoreboard->ResetPlayerScores();
        }
    }

    GameModePacket::SoundPositionData soundData(
        static_cast<float>(player->mX),
        static_cast<float>(player->mY),
        static_cast<float>(player->mZ));
    BroadcastGameModePacket(static_cast<GameModePacket::EMessage>(0x18), soundData);

    if (player->mInventory != 0)
        player->mInventory->pickSlot(0);
    player->removeAllEffects();
    UpdateClientsCameras(player);
}

void MasterGameMode::StopAchievementExploit(const std::wstring& playerName)
{
    if (gMinecraftServer == 0)
        return;

    PlayerList* playerList = gMinecraftServer->getPlayers();
    std::vector<boost::shared_ptr<ServerPlayer> >* players = playerList->getPlayers();

    int validPlayerCount = 0;
    bool matchedExploiterName = false;
    bool checkedFirstPlayer = false;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (!player->isAlive())
            continue;
        if (player->isSpectator())
            continue;

        ++validPlayerCount;
        if (!checkedFirstPlayer)
        {
            checkedFirstPlayer = true;
            if (player->getDisplayName().compare(playerName) == 0)
                matchedExploiterName = true;
        }
    }

    if (matchedExploiterName && validPlayerCount == 2)
        WiiUAchievementExploitStopped(this) = true;
}

void MasterGameMode::OnKickReqested(int playerIndex, int reason)
{
    if (mVoteables == 0)
        return;

    const int voteCount = mVoteables->getVoteable(1)->getTotalledVotes()[reason];

    INetworkPlayer* voterPlayer = getNetworkPlayerFromIndex(playerIndex);
    INetworkPlayer* targetPlayer = getNetworkPlayerFromIndex(reason);

    int cappedPlayerCount = GetPlayerCount(false, false) - 1;
    if (cappedPlayerCount > 3)
        cappedPlayerCount = 3;

    if (voterPlayer != 0 && targetPlayer != 0)
    {
        std::wstring stringParams[2];
        stringParams[0] = voterPlayer->GetDisplayName();
        stringParams[1] = targetPlayer->GetDisplayName();
        int intParams[2];
        intParams[0] = voteCount;
        intParams[1] = cappedPlayerCount;

        boost::shared_ptr<Packet> chatPacket(
            new ClientboundChatPacket(
                static_cast<ClientboundChatPacket::EChatPacketMessage>(11),
                intParams, 2, stringParams, 2));

        if (gMinecraftServer != 0 && gMinecraftServer->getPlayers() != 0)
            gMinecraftServer->getPlayers()->broadcastAll(chatPacket);
    }

    if (cappedPlayerCount <= voteCount)
    {
        if (targetPlayer != 0)
        {
            const wchar_t* targetName = targetPlayer->GetOnlineName();
            if (targetName != 0)
                StopAchievementExploit(std::wstring(targetName));

            if (gMinecraftServer != 0 && gMinecraftServer->getPlayers() != 0)
                gMinecraftServer->getPlayers()->kickPlayerByShortId(targetPlayer->GetSmallId());
        }

        mVoteables->getVoteable(1)->setIncludedVotesAndSync(
            reason, reason, static_cast<char>(-1));
    }
}

void MasterGameMode::OnStartSpectating(boost::shared_ptr<ServerPlayer> player)
{
    player->setGameMode(const_cast<GameType*>(GameType::SPECTATOR));

    if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        boost::shared_ptr<ServerPlayer> trailingPlayer = mGameStats.GetTrailingPlayer(player);
        if (trailingPlayer != 0)
        {
            player->SetCamera(trailingPlayer);
            UpdateClientsCameras(player);
        }
    }
}

void MasterGameMode::OnServerResume(long long suspendedTime)
{
    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame->GetId() != eMiniGameId_Glide && miniGame->GetId() != eMiniGameId_Lobby)
        return;

    miniGame = GetMiniGame();
    if (miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
        return;

    if (IsRoundPlaying())
    {
        mGameStats.increasePlayerRoundStartTimeMs(suspendedTime);
        mRoundStartTime += static_cast<unsigned long long>(suspendedTime);
    }
    else if (HasTimer(eTimerID_RoundStart))
    {
        RestartTimer(eTimerID_RoundStart,
                     GetTimerRemainingTime(eTimerID_RoundStart) + static_cast<int>(suspendedTime));
    }

    GhostController* ghostController = Minecraft::GetInstance()->GetGhostController();
    if (ghostController != 0)
    {
        const long long lastTick =
            (static_cast<long long>(GhostControllerLastTickHigh(ghostController)) << 32) |
            static_cast<unsigned int>(GhostControllerLastTickLow(ghostController));
        const long long newLastTick = lastTick + suspendedTime;
        GhostControllerLastTickHigh(ghostController) = static_cast<int>(newLastTick >> 32);
        GhostControllerLastTickLow(ghostController) = static_cast<unsigned int>(newLastTick);

        const long long playbackTime =
            (static_cast<long long>(GhostControllerPlaybackTimeHigh(ghostController)) << 32) |
            static_cast<unsigned int>(GhostControllerPlaybackTimeLow(ghostController));
        const long long newPlaybackTime = playbackTime + suspendedTime;
        GhostControllerPlaybackTimeHigh(ghostController) = static_cast<int>(newPlaybackTime >> 32);
        GhostControllerPlaybackTimeLow(ghostController) = static_cast<unsigned int>(newPlaybackTime);
    }

    Minecraft::GetInstance()->GetClientMasterGameMode()->SetLocalRoundTimeStartOverride(mGameStats.GetStartTime());

    if (HasTimer(eTimerID_GameEnd))
    {
        RestartTimer(eTimerID_GameEnd,
                     GetTimerRemainingTime(eTimerID_GameEnd) + static_cast<int>(suspendedTime));
    }

    if (HasTimer(eTimerID_ChestRefill))
    {
        RestartTimer(eTimerID_ChestRefill,
                     GetTimerRemainingTime(eTimerID_ChestRefill) + static_cast<int>(suspendedTime));
    }

    std::vector<boost::shared_ptr<ServerPlayer> > roundPlayers = GetPlayers(true, false);
    std::vector<boost::shared_ptr<Player> > players(roundPlayers.begin(), roundPlayers.end());
    CheckTargets(players);
    CheckCheckpoints(players);
}

void MasterGameMode::PlayerFinishedLoadingLocally()
{
    mPlaylistDirty = true;
}

void MasterGameMode::ResetLobbyTimerAfterSuspend()
{
    System::processTimeInMilliSecs();
}

void MasterGameMode::OnLevelLoad(ServerLevel* level)
{
    mPlaylistDirty = true;
}

int MasterGameMode::GetChunkHistory() const
{
    return mChunkHistory;
}

int MasterGameMode::GetRoundTime()
{
    return mRoundTimeOffset +
           static_cast<int>(GetCurrentTimeMillis() - mRoundStartTime);
}

int MasterGameMode::GetPostGameTime()
{
    return mPostGameTime;
}

int MasterGameMode::GetMaxKills()
{
    unsigned int maxKills = 0;
    std::vector<boost::shared_ptr<ServerPlayer> >* players = gMinecraftServer->getPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        const unsigned int kills = (*playerIt)->GetGameMode()->GetKills();
        if (maxKills < kills)
            maxKills = kills;
    }
    return static_cast<int>(maxKills);
}

int MasterGameMode::GetActiveTeamCount()
{
    return mActiveTeamCount;
}

Vec3* MasterGameMode::GetLastKnownPosition(boost::shared_ptr<Player> player)
{
    return mLastKnownPositions[0];
}

AABB* MasterGameMode::GetSweptVolumeFromLastKnownPosition(boost::shared_ptr<Player> player)
{
    Vec3* position = GetLastKnownPosition(player);
    if (position == 0)
        return 0;
    return AABB::newTemp(position->x, position->y, position->z,
                         position->x, position->y, position->z);
}

unsigned int MasterGameMode::GetLastCheckpointID(const StatsUID& uid)
{
    return mGameStats.GetLastCheckpointID(uid);
}

void MasterGameMode::CatchUpCheckpointCheck(boost::shared_ptr<ServerPlayer> player, AABB* sweptVolume)
{
    for (unsigned int index = 0; index < mCheckpoints.size(); ++index)
    {
        AABB* checkpointArea = mCheckpoints[index]->getArea();
        if (checkpointArea->intersects(sweptVolume->mMinX, sweptVolume->mMinY, sweptVolume->mMinZ,
                                        sweptVolume->mMaxX, sweptVolume->mMaxY, sweptVolume->mMaxZ))
        {
            OnCheckpointEnter(player, index, mCheckpoints[index]);
        }
    }
}

void MasterGameMode::KillTimers()
{
    for (TimerMap::iterator entry = mTimers.begin(); entry != mTimers.end(); ++entry)
        entry->second.active = false;
}

void MasterGameMode::PositionPlayersInSpectatorLobby()
{
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (ruleset == 0)
        return;

    AABB* spectatorSpawn = ruleset->getNamedArea(L"SpectatorSpawn");
    if (gMinecraftServer == 0 || gMinecraftServer->getPlayers() == 0)
        return;

    std::vector<boost::shared_ptr<ServerPlayer> >* players =
        gMinecraftServer->getPlayers()->getPlayers();
    if (players == 0)
        return;

    if (spectatorSpawn != 0)
    {
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
             playerIt != players->end(); ++playerIt)
            GameModeUtilities::SpawnWithinArea(*playerIt, spectatorSpawn);
        return;
    }

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (player == 0)
            continue;

        ruleset->postProcessPlayer(player);
        if (player->mConnection != 0)
            player->mConnection->teleport(player->mX, player->mY, player->mZ,
                                          player->mXRot, player->mYRot, true, true);
    }
}

void MasterGameMode::ResetPlayer(boost::shared_ptr<ServerPlayer> player)
{
    mPlaylistDirty = true;
}

void MasterGameMode::ReduceShowdownTime(int amount, int minimum)
{
    bool timerAdjusted = false;
    int remaining = 0;

    if (HasTimer(eTimerID_RoundLimit))
    {
        remaining = GetTimerRemainingTime(eTimerID_RoundLimit);
        if (minimum < remaining)
        {
            if (remaining - amount < minimum)
            {
                amount = remaining - minimum;
                RestartTimer(eTimerID_RoundLimit, minimum);
                timerAdjusted = true;
            }
            else
            {
                RestartTimer(eTimerID_RoundLimit, remaining - amount);
                timerAdjusted = true;
            }
        }
    }

    if (HasTimer(eTimerID_Tutorial))
    {
        remaining = GetTimerRemainingTime(eTimerID_Tutorial);
        if (minimum < remaining)
        {
            if (remaining - amount < minimum)
            {
                amount = remaining - minimum;
                RestartTimer(eTimerID_Tutorial, minimum);
                timerAdjusted = true;
            }
            else
            {
                RestartTimer(eTimerID_Tutorial, remaining - amount);
                timerAdjusted = true;
            }
        }
    }

    if ((mRoundTimeOffset != 0 || mPausedRoundTime != 0) && timerAdjusted)
    {
        const unsigned int low = static_cast<unsigned int>(mPausedRoundTime);
        mPausedRoundTime = static_cast<int>(low - amount);
        mRoundTimeOffset -= static_cast<int>((amount >> 31) + static_cast<unsigned int>(low < static_cast<unsigned int>(amount)));
    }
}

void MasterGameMode::SetDefaults(LaunchMoreOptionsMenuInitData& data)
{
    System::processTimeInMilliSecs();
}

void MasterGameMode::SaveDefaults(const LaunchMoreOptionsMenuInitData& data)
{
    System::processTimeInMilliSecs();
}

const std::vector<int>& MasterGameMode::GetOptionsList(EMiniGameId miniGame, eCustomGameModeHostOption option)
{
    return MiniGameDef::GetCustomGameModeById(miniGame, true)->GetOptionsList(option);
}

unsigned int MasterGameMode::GetOptionMin(EMiniGameId miniGame, eCustomGameModeHostOption option, bool includeRandom)
{
    return MiniGameDef::GetCustomGameModeById(miniGame, true)->GetOptionMin(option, includeRandom);
}

int MasterGameMode::GetOptionMax(EMiniGameId miniGame, eCustomGameModeHostOption option)
{
    return MiniGameDef::GetCustomGameModeById(miniGame, true)->GetOptionMax(option);
}

std::wstring MasterGameMode::GetValueText(EMiniGameId miniGame, eCustomGameModeHostOption option, int value)
{
    return MiniGameDef::GetCustomGameModeById(miniGame, true)->GetValueText(option, value);
}

void MasterGameMode::UpdateRulePlayList(std::vector<LevelRules::LevelRulesetGroup*>* playlist)
{
    mPlaylist.insert(mPlaylist.end(), playlist->begin(), playlist->end());
    std::sort(mPlaylist.begin(), mPlaylist.end(), LevelRules::LevelRulesetGroup::SortMapRules);
    mRuleGroups.clear();
    mShuffledRuleGroups.clear();
    mShuffledRuleGroups.insert(mShuffledRuleGroups.end(), mPlaylist.begin(), mPlaylist.end());
}

void MasterGameMode::SetPlayerReady(unsigned int playerIndex, bool ready)
{
    EInternalGameModeState state = GetState();
    if (state != eInternalGameModeState_RoundStarting && state != eInternalGameModeState_WaitingToStart)
        return;

    mPlayerReady[playerIndex] = ready;

    gMinecraftServer->getPlayers()->broadcastAll(
        boost::shared_ptr<Packet>(
            new PlayerReadyPacket(playerIndex, ready)));
}

bool MasterGameMode::IsChunkWithinLobbyRegionOfRoundLevel(int chunkX, int chunkZ)
{
    return mRoundStarted || mRoundEnding;
}

void MasterGameMode::GetMaxActiveBounds(int& minimumX, int& minimumZ, int& maximumX, int& maximumZ)
{
    if (!mTeamAreas.empty())
    {
        std::vector<AABB*>::const_iterator it = mTeamAreas.begin();
        const AABB* bounds = *it;
        minimumX = static_cast<int>(bounds->mMinX);
        minimumZ = static_cast<int>(bounds->mMinZ);
        maximumX = static_cast<int>(bounds->mMaxX);
        maximumZ = static_cast<int>(bounds->mMaxZ);
        for (++it; it != mTeamAreas.end(); ++it)
        {
            bounds = *it;
            if (static_cast<int>(bounds->mMinX) < minimumX)
                minimumX = static_cast<int>(bounds->mMinX);
            if (static_cast<int>(bounds->mMinZ) < minimumZ)
                minimumZ = static_cast<int>(bounds->mMinZ);
            if (maximumX < static_cast<int>(bounds->mMaxX))
                maximumX = static_cast<int>(bounds->mMaxX);
            if (maximumZ < static_cast<int>(bounds->mMaxZ))
                maximumZ = static_cast<int>(bounds->mMaxZ);
        }
    }
}

void MasterGameMode::BlockDestroyedAt(const BlockPos& position)
{
    if (mMapGenerator == 0 || GetState() != static_cast<EInternalGameModeState>(0xa))
        return;

    AABB* layerArea = mMapGenerator->GetLayerBB(static_cast<unsigned int>(mSpawnLayerIndex));
    if (layerArea == 0 || !layerArea->contains(position))
        return;

    const int destroyedCount = WiiULayerBlocksDestroyed(this);
    WiiULayerBlocksDestroyed(this) = destroyedCount + 1;

    const int blockLimit = static_cast<int>(
        mMapGenerator->GetBlockCountOnLayer(static_cast<unsigned int>(mSpawnLayerIndex)));
    if (blockLimit < destroyedCount + 11)
    {
        ++mSpawnLayerIndex;
        WiiULayerBlocksDestroyed(this) = 0;

        const unsigned int layerCount = static_cast<unsigned int>(mMapGenerator->GetNumberOfLayers());
        if (layerCount <= static_cast<unsigned int>(mSpawnLayerIndex))
            mSpawnLayerIndex = static_cast<int>(layerCount) - 1;
    }
}

void MasterGameMode::ResetSpawnLayers()
{
    mSpawnLayerIndex = 0;
}

void MasterGameMode::RegenTumbleLayers()
{
    ResetSpawnLayers();
    RestartMapGenerator();
}

void MasterGameMode::RestartMapGenerator()
{
    ResetSpawnLayers();
    CommonMasterGameMode::RestartMapGenerator();
}

unsigned int MasterGameMode::GetGameModeState() const
{
    return static_cast<unsigned int>(mInternalState);
}

ServerLevel* MasterGameMode::GeneratorTargetLevel() const
{
    return mRoundLevel;
}

void MasterGameMode::setChosenThemeWordId(int wordId)
{
    mChosenThemeWordId = wordId;
    CommonMasterGameMode::setChosenThemeWordId(wordId);

    GameModePacket::UpdateThemeGameModeData data(wordId);
    boost::shared_ptr<Packet> packet(
        GameModePacket::Create(static_cast<GameModePacket::EMessage>(0x1a), data));
    if (gMinecraftServer != 0)
        gMinecraftServer->getPlayers()->broadcastAll(packet);
}

bool MasterGameMode::IsRoundPlaying() const
{
    return mInternalState == eInternalGameModeState_RoundPlaying;
}

Scoreboard* MasterGameMode::getScoreboard()
{
    return mScoreboard;
}

void MasterGameMode::OnCheckpointEnter(const boost::shared_ptr<Player>& player, unsigned int checkpoint, CheckpointRuleDefinition* rule)
{
    mPlaylistDirty = true;
}

void MasterGameMode::PlayReachedEndSFX(const std::vector<boost::shared_ptr<ServerPlayer> >& players, boost::shared_ptr<ServerPlayer> winner)
{
    (void)players;
    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame == 0 || !miniGame->HasFeatureFinishLineSoundEffect())
        return;

    unsigned int finishers = 0;
    if (gMinecraftServer != 0 && gMinecraftServer->getPlayers() != 0)
    {
        std::vector<boost::shared_ptr<ServerPlayer> >* activePlayers =
            gMinecraftServer->getPlayers()->getPlayers();
        if (activePlayers != 0)
        {
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt =
                     activePlayers->begin();
                 playerIt != activePlayers->end(); ++playerIt)
            {
                boost::shared_ptr<ServerPlayer> activePlayer = *playerIt;
                if (activePlayer == 0)
                    continue;

                const StatsUID suid = activePlayer->getSUID();
                if (mGameStats.GetPlayerStats(suid) != 0 &&
                    mGameStats.GetEndZoneEntered(suid) != 0)
                {
                    ++finishers;
                }
            }
        }
    }

    const bool scoreVariant = miniGame->GetGameVariant() != 0;
    const SoundEvent* sound =
        SelectFinishLineSound(finishers, scoreVariant, winner, mGameStats);
    if (winner == 0 || sound == 0 || gMinecraftServer == 0 ||
        gMinecraftServer->getPlayers() == 0)
    {
        return;
    }

    boost::shared_ptr<Packet> packet(new ClientboundSoundPacket(
        ClientboundSoundPacket::eSoundInstances_Default, sound, 1.0f, 1.0f, false,
        winner->getId()));
    gMinecraftServer->getPlayers()->broadcast(
        winner, winner->mX, winner->mY, winner->mZ, 30.0, winner->mDimension, packet);
}

void MasterGameMode::OnPrimaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule)
{
    std::vector<boost::shared_ptr<ServerPlayer> > players = GetPlayers(true, false);
    boost::shared_ptr<ServerPlayer> serverPlayer = boost::static_pointer_cast<ServerPlayer>(player);
    if (mGameStats.RecordEndZoneEntered(serverPlayer->getSUID()) != 0)
    {
        ReduceShowdownTime(60000, 30000);
        mLastKnownPositionPlayers.push_back(serverPlayer->getSUID());
        PlayReachedEndSFX(players, serverPlayer);
    }
}

void MasterGameMode::OnSecondaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule)
{
    boost::shared_ptr<ServerPlayer> serverPlayer = boost::static_pointer_cast<ServerPlayer>(player);
    ServerPlayerGameMode* gameMode = serverPlayer->GetGameMode();
    if (gameMode != 0)
    {
        if (gameMode->RecordSecondaryTargetReached(
                rule->getName(), rule->getPointValue(),
                static_cast<GlideRingGenerator::eGlideRingSize>(rule->getSize())))
        {
            mLastKnownPositionPlayers.push_back(serverPlayer->getSUID());
        }
    }
}

void MasterGameMode::OnPowerupEnter(const boost::shared_ptr<Player>& player, unsigned int powerup, PowerupRuleDefinition* rule)
{
    mPlaylistDirty = true;
}

MasterGameMode::ChestRefillInfo::ChestRefillInfo()
{}

MasterGameMode::ChestRefillInfo::ChestRefillInfo(const ChestRefillInfo& other)
    : distance(other.distance), weight(other.weight), x(other.x), y(other.y), z(other.z), itemSet(other.itemSet), minimumPlayers(other.minimumPlayers), maximumPlayers(other.maximumPlayers), refillCount(other.refillCount), refillInterval(other.refillInterval), nextRefill(other.nextRefill), region(other.region), chestIndex(other.chestIndex), lootGroup(other.lootGroup)
{}

MasterGameMode::CountdownInfo::CountdownInfo()
    : message(0), arguments(), callbackData(0), completeCallback(0), tickCallback(0)
{}
