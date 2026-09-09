#ifndef GAME_CORE_MASTERGAMEMODE_HPP
#define GAME_CORE_MASTERGAMEMODE_HPP

#include <map>
#include <string>
#include <vector>
#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/GameStats.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelRules.hpp"
#include "game/world/chunk/ActiveChunkBoundsChecker.hpp"

class AABB;
class Abilities;
class BlockPos;
class BlockRemover;
class CheckpointRuleDefinition;
class Entity;
class GameType;
class KillboxRuleDefinition;
class LaunchMoreOptionsMenuInitData;
class LevelGenerationOptions;
class Player;
class PlayerTeam;
class PowerupRuleDefinition;
class Scoreboard;
class ServerLevel;
class ServerPlayer;
class ServerScoreboard;
class Settings;
class StatsUID;
class TargetAreaRuleDefinition;
class Vec3;

class MasterGameMode : public CommonMasterGameMode
{
public:
    enum ETimerID
    {
        eTimerID_GameChange,
        eTimerID_WaitingToStart,
        eTimerID_RoundStart,
        eTimerID_GracePeriod,
        eTimerID_ChestRefill,
        eTimerID_Showdown,
        eTimerID_GameEnd,
        eTimerID_ReturnToLobby,
        eTimerID_Tutorial,
        eTimerID_Transition,
        eTimerID_PostGame,
        eTimerID_RoundLimit
    };

    enum EInternalGameModeState
    {
        eInternalGameModeState_Lobby,
        eInternalGameModeState_WaitingToStart,
        eInternalGameModeState_RoundStarting,
        eInternalGameModeState_RoundPlaying,
        eInternalGameModeState_Showdown,
        eInternalGameModeState_RoundEnding,
        eInternalGameModeState_PostGame,
        eInternalGameModeState_ReturningToLobby
    };

    typedef void (*TimerCallback)(ETimerID, MasterGameMode*, void*);
    typedef void (*CountdownCallback)(MasterGameMode*, void*);

    struct _TimerInfo
    {
        long long endTime;
        long long startTime;
        int duration;
        int announceEvery;
        int changeAnnounceAfter;
        int finalAnnounceEvery;
        long long nextAnnouncement;
        bool active;
        void* callbackData;
        TimerCallback callback;
    };

    class ChestRefillInfo
    {
    public:
        ChestRefillInfo(const ChestRefillInfo& other);
        float distance;
        float weight;
        int x;
        short y;
        int z;
        short itemSet;
        int minimumPlayers;
        int maximumPlayers;
        short refillCount;
        int refillInterval;
        int nextRefill;
        short region;
        int chestIndex;
        int lootGroup;
    };

    class CountdownInfo
    {
    public:
        CountdownInfo();
        int message;
        std::vector<int> arguments;
        void* callbackData;
        CountdownCallback completeCallback;
        CountdownCallback tickCallback;
    };

    MasterGameMode(std::vector<LevelRules::LevelRulesetGroup*>* playlist);
    virtual ~MasterGameMode();

    std::vector<boost::shared_ptr<ServerPlayer> > GetPlayers(bool includeSpectators, bool includeDead);
    int GetPlayerCount(bool includeSpectators, bool includeDead);
    unsigned long long GetCurrentTimeMillis();
    bool HasTimer(ETimerID timer);
    void KillTimer(ETimerID timer);
    void ResetRoundTime();
    static void StaticCtor();
    void GeneratePlaylistSyncInfo();
    void ResetReadyStates(bool allPlayers);
    void ResetLastKnownPosition(int playerIndex);
    int GetMinimumPlayersToStart();
    EInternalGameModeState GetState();
    void RestartTimer(ETimerID timer, int duration);
    void TickTimers();
    void SyncNextLevelGen();
    void OnGameModeChange(boost::shared_ptr<ServerPlayer> player);
    void CheckTutorialUpdate();
    bool IsInKillBox(boost::shared_ptr<Entity> entity);
    bool AreLevelChunksLoaded();
    void ChooseNextGameRules(bool advancePlaylist);
    void UpdateClientState(boost::shared_ptr<ServerPlayer> player);
    void SetState(EInternalGameModeState state);
    void sendTutorialUpdate(eTutorial_GameMode tutorial);
    void BroadcastChat(int message, std::vector<int>& arguments);
    void CompleteTimer(ETimerID timer);
    void CheckReadyStates();
    static void OnGameChange(MasterGameMode* gameMode, void* context);
    void AddTimer(ETimerID timer, TimerCallback callback, void* context, int duration, int announceEvery, int changeAnnounceAfter, int finalAnnounceEvery);
    void AddTimer(ETimerID timer, TimerCallback callback, void* context, MiniGameDef::TimerSpec specification);
    int GetTimerRemainingTime(ETimerID timer);
    void TimerCountdown(ETimerID timer, CountdownInfo* countdown);
    static void OnTimerCountdown(ETimerID timer, MasterGameMode* gameMode, void* context);
    void PopulateChests();
    int ChooseItemSet(const MiniGameDef& definition, bool central);
    int GetTeamCount();
    void SetupTeams();
    void SelectNewGameRules();
    void GetKillBoxes(LevelGenerationOptions* options, std::vector<KillboxRuleDefinition*>& killboxes);
    void ResetPlayers();
    static bool IsChunkWithinActiveBounds(void* context, int chunkX, int chunkZ);
    void GenerateChestRefillInfo();
    bool LoadNewGameRules();
    void SetLockPlayerPositions(bool locked);
    void UpdateGeneratorSeed();
    int GeneratorSpawn() const;
    boost::shared_ptr<ServerPlayer> GetRoundWinner();
    int GetSpawnLayerIndex() const;
    void PlacePlayers(std::vector<boost::shared_ptr<ServerPlayer> >* players, bool atStart, bool includeSpectators);
    void PositionPlayersAtStartingPosition();
    void RemoveAllEntities(bool keepPlayers);
    void AddPlayersToTeams();
    static void OnWaitingToStartCountdown(ETimerID timer, MasterGameMode* gameMode, void* context);
    bool CanRoundStart();
    void SetGameMode(const GameType* gameType);
    void ResetPlayersFlyingState(std::vector<boost::shared_ptr<ServerPlayer> >* players);
    void ResetPlayerHealths(std::vector<boost::shared_ptr<ServerPlayer> >* players, float health);
    void SetPlayerAbilities(boost::shared_ptr<ServerPlayer> player, const Abilities* abilities);
    void SetPlayersAbilities(std::vector<boost::shared_ptr<ServerPlayer> >* players, const Abilities* abilities);
    void UpdatePlayersAbilities(std::vector<boost::shared_ptr<ServerPlayer> >* players);
    bool ShouldRemoveArmourPiece(int slot);
    void EmptyPlayerInventory(std::vector<boost::shared_ptr<ServerPlayer> >* players, bool includeArmour);
    void ResetPlayersEquipmentAndInventory(std::vector<boost::shared_ptr<ServerPlayer> >* players);
    void PreparePlayersForGamemode(std::vector<boost::shared_ptr<ServerPlayer> >* players);
    void ResetGameRules();
    void SetPlayerInvulnerable(boost::shared_ptr<ServerPlayer> player, bool invulnerable);
    void SetPlayersInvulnerable(bool invulnerable);
    static void OnRefillChestTimer(MasterGameMode* gameMode, void* context);
    static void OnGracePeriodEnd(MasterGameMode* gameMode, void* context);
    bool StartTimer(ETimerID timer, CountdownInfo* countdown, int duration);
    void StartTimer(ETimerID timer, CountdownInfo* countdown);
    static void OnGameStart(MasterGameMode* gameMode, void* context);
    void SetLastKnownPosition(boost::shared_ptr<Player> player);
    void UpdateClientsCameras(boost::shared_ptr<ServerPlayer> player);
    void Tick_Round_Playing();
    bool CanRoundEnd();
    bool CanStartShowdown();
    std::vector<boost::shared_ptr<ServerPlayer> > GetGlidePlayers();
    int GetShowdownTime();
    void StartRoundEndTimer();
    void OnShowdownStart();
    std::vector<PlayerTeam*> GetActiveTeams();
    bool CanEndPostGame();
    bool ShouldPlayAnotherRound();
    static void OnGameEnd(MasterGameMode* gameMode, void* context);
    void ReturnPlayersToLobby();
    virtual void Tick();
    void SetupServerSettings(Settings* settings);
    const GameType* InitialGameType(boost::shared_ptr<ServerPlayer> player);
    bool IsInRound();
    void PlaceSpectators(std::vector<boost::shared_ptr<ServerPlayer> >* players);
    void OnSpawn(boost::shared_ptr<ServerPlayer> player, boost::shared_ptr<ServerPlayer> owner, bool joining);
    void OnSpawned(boost::shared_ptr<ServerPlayer> player, bool joining);
    void SendPlaylistInfo(boost::shared_ptr<ServerPlayer> player);
    void SetLockPlayerPosition(boost::shared_ptr<ServerPlayer> player, bool locked);
    void OnJoin(boost::shared_ptr<ServerPlayer> player);
    void OnFollow(boost::shared_ptr<ServerPlayer> player, bool started, bool force);
    void OnLeave(boost::shared_ptr<ServerPlayer> player);
    void OnDeath(boost::shared_ptr<ServerPlayer> player);
    void StopAchievementExploit(const std::wstring& playerName);
    void OnKickReqested(int playerIndex, int reason);
    void OnStartSpectating(boost::shared_ptr<ServerPlayer> player);
    void OnServerResume(long long suspendedTime);
    void PlayerFinishedLoadingLocally();
    static void ResetLobbyTimerAfterSuspend();
    void OnLevelLoad(ServerLevel* level);
    int GetChunkHistory() const;
    int GetRoundTime();
    int GetPostGameTime();
    int GetMaxKills();
    int GetActiveTeamCount();
    Vec3* GetLastKnownPosition(boost::shared_ptr<Player> player);
    virtual AABB* GetSweptVolumeFromLastKnownPosition(boost::shared_ptr<Player> player);
    virtual unsigned int GetLastCheckpointID(const StatsUID& uid);
    void CatchUpCheckpointCheck(boost::shared_ptr<ServerPlayer> player, AABB* sweptVolume);
    void KillTimers();
    void PositionPlayersInSpectatorLobby();
    void ResetPlayer(boost::shared_ptr<ServerPlayer> player);
    void ReduceShowdownTime(int amount, int minimum);
    static void SetDefaults(LaunchMoreOptionsMenuInitData& data);
    static void SaveDefaults(const LaunchMoreOptionsMenuInitData& data);
    static const std::vector<int>& GetOptionsList(EMiniGameId miniGame, eCustomGameModeHostOption option);
    static unsigned int GetOptionMin(EMiniGameId miniGame, eCustomGameModeHostOption option, bool includeRandom);
    static int GetOptionMax(EMiniGameId miniGame, eCustomGameModeHostOption option);
    static std::wstring GetValueText(EMiniGameId miniGame, eCustomGameModeHostOption option, int value);
    void UpdateRulePlayList(std::vector<LevelRules::LevelRulesetGroup*>* playlist);
    void SetPlayerReady(unsigned int playerIndex, bool ready);
    bool IsChunkWithinLobbyRegionOfRoundLevel(int chunkX, int chunkZ);
    void GetMaxActiveBounds(int& minimumX, int& minimumZ, int& maximumX, int& maximumZ);
    void BlockDestroyedAt(const BlockPos& position);
    void ResetSpawnLayers();
    void RegenTumbleLayers();
    virtual void RestartMapGenerator();
    virtual unsigned int GetGameModeState() const;
    virtual ServerLevel* GeneratorTargetLevel() const;
    virtual void setChosenThemeWordId(int wordId);
    virtual bool IsRoundPlaying() const;
    virtual Scoreboard* getScoreboard();
    virtual void OnCheckpointEnter(const boost::shared_ptr<Player>& player, unsigned int checkpoint, CheckpointRuleDefinition* rule);
    void PlayReachedEndSFX(const std::vector<boost::shared_ptr<ServerPlayer> >& players, boost::shared_ptr<ServerPlayer> winner);
    virtual void OnPrimaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule);
    virtual void OnSecondaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule);
    virtual void OnPowerupEnter(const boost::shared_ptr<Player>& player, unsigned int powerup, PowerupRuleDefinition* rule);

    void DebugPrintTimerStates();
    void getActiveViewAreas();
    void debugRoundTime();
    static void OnGameInitialised(MasterGameMode* gameMode, void* context);
    void DegradeBlocks();
    void ResetPlayerHealths(float health);
    void SetPlayersAbilities(const Abilities* abilities);
    static int GetOptionsCount(EMiniGameId miniGame, eCustomGameModeHostOption option);
    static std::wstring GetOptionName(EMiniGameId miniGame, eCustomGameModeHostOption option);
    bool IsPlayerReady(unsigned int playerIndex);

private:
    friend class CheckpointCommand;
    friend class GameMedal;
    friend class ServerPlayerGameMode;
    friend class ServerScoreboard;

    unsigned long long mRoundStartTime;
    int mRoundTimeOffset;
    int mPausedRoundTime;
    int mShowdownTime;
    int mPostGameTime;
    boost::unordered_map<ETimerID, _TimerInfo> mTimers;
    LevelGenerationOptions* mNextLevelGenerationOptions;
    GameStats mGameStats;
    int mInternalState;
    int mPreviousState;
    std::vector<LevelRules::LevelRulesetGroup*> mPlaylist;
    std::deque<LevelRules::LevelRulesetGroup*> mRuleGroups;
    std::deque<LevelRules::LevelRulesetGroup*> mShuffledRuleGroups;
    int mCurrentPlaylistEntry;
    int mCurrentRuleIndex;
    unsigned int mReadyPlayers;
    unsigned int mRequiredPlayers;
    int mMinimumPlayers;
    int mMaximumPlayers;
    bool mPlayerReady[8];
    std::vector<ChestRefillInfo> mChestRefills;
    std::vector<KillboxRuleDefinition*> mKillboxes;
    std::vector<AABB*> mKillboxAreas;
    ActiveChunkBoundsChecker mActiveChunkBounds;
    Random mRandom;
    bool mLockPlayerPositions;
    bool mRoundStarted;
    bool mRoundEnding;
    bool mShowdownStarted;
    bool mPlaylistDirty;
    bool mTutorialDirty;
    bool mPlayersPrepared;
    bool mReturningToLobby;
    int mGeneratorSpawn;
    int mSpawnLayerIndex;
    int mChunkHistory;
    int mRoundWinner;
    int mActiveTeamCount;
    int mShowdownReduction;
    int mChosenThemeWordId;
    GameModePacket::TutorialGameModeData mTutorialData;
    ServerLevel* mRoundLevel;
    BlockRemover* mBlockRemover;
    std::vector<StatsUID> mLastKnownPositionPlayers;
    Vec3* mLastKnownPositions[8];
    Scoreboard* mScoreboard;
};

#endif
