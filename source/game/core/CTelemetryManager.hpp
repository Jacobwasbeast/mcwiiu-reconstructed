#ifndef GAME_CORE_CTELEMETRYMANAGER_HPP
#define GAME_CORE_CTELEMETRYMANAGER_HPP

#include <cstddef>
#include <cstdint>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/minigame/EMiniGameId.hpp"
#include "game/ui/UILayer.hpp"
#include "game/util/Vec3.hpp"

class LaunchMoreOptionsMenuInitData;
class LocalPlayer;
struct SAVE_DETAILS;
class CTelemetryManager;

struct CTelemetryManagerVTableEntry
{
    std::uint32_t function;
    std::int32_t adjustment;
};

struct CTelemetryManagerVTable
{
    std::uint32_t header[3];
    CTelemetryManagerVTableEntry slots[38];
};

static_assert(sizeof(CTelemetryManagerVTable) == 0x13c, "CTelemetryManager GHS table size");
static_assert(offsetof(CTelemetryManagerVTable, slots[0]) == 0x0c,
              "CTelemetryManager dtor slot offset");
static_assert(offsetof(CTelemetryManagerVTable, slots[37]) == 0x134,
              "CTelemetryManager final slot offset");

enum CTelemetryManagerVTableSlot
{
    eTelemetryDtor = 0,
    eTelemetryInit,
    eTelemetryTick,
    eTelemetryFlush,
    eTelemetryPlayerSessionStart,
    eTelemetryPlayerSessionExit,
    eTelemetryHeartBeat,
    eTelemetryLevelStart,
    eTelemetryLevelExit,
    eTelemetryLevelSaveOrCheckpoint,
    eTelemetryLevelResume,
    eTelemetryPauseOrInactive,
    eTelemetryUnpauseOrActive,
    eTelemetryMenuShown,
    eTelemetryAchievementUnlocked,
    eTelemetryMediaShareUpload,
    eTelemetryUpsellPresented,
    eTelemetryUpsellResponded,
    eTelemetryPlayerDiedOrFailed,
    eTelemetryEnemyKilledOrOvercome,
    eTelemetryTexturePackLoaded,
    eTelemetrySkinChanged,
    eTelemetryBanLevel,
    eTelemetryUnBanLevel,
    eTelemetryPrepareMiniGameHostOptions,
    eTelemetryMiniGameHostOptions,
    eTelemetryPlayerSettings,
    eTelemetryBlockPlaced,
    eTelemetryBlockBroken,
    eTelemetryItemAcquired,
    eTelemetryItemUsed,
    eTelemetryGameModeRoundStart,
    eTelemetryGameModeRoundFinished,
    eTelemetrySavesInformation,
    eTelemetryGetMultiplayerInstanceID,
    eTelemetryGenerateMultiplayerInstanceId,
    eTelemetrySetMultiplayerInstanceId,
    eTelemetryTransitionToLobby
};

enum ESen_FriendOrMatch
{
    eSen_FriendOrMatch_NotApplicable,
    eSen_FriendOrMatch_Friend,
    eSen_FriendOrMatch_Match
};

enum ESen_CompeteOrCoop
{
    eSen_CompeteOrCoop_NotApplicable,
    eSen_CompeteOrCoop_Compete,
    eSen_CompeteOrCoop_Cooperate,
    eSen_CompeteOrCoop_Both
};

enum ESen_LevelExitStatus
{
    eSen_LevelExitStatus_Unknown,
    eSen_LevelExitStatus_Completed,
    eSen_LevelExitStatus_Restarted,
    eSen_LevelExitStatus_Quit
};

enum ESen_MediaDestination
{
    eSen_MediaDestination_Unknown,
    eSen_MediaDestination_LocalStorage,
    eSen_MediaDestination_SocialNetwork
};

enum ESen_MediaType
{
    eSen_MediaType_Unknown,
    eSen_MediaType_Screenshot,
    eSen_MediaType_Video
};

enum ESen_UpsellID
{
    eSen_UpsellID_None,
    eSen_UpsellID_FullGame
};

enum ESen_UpsellOutcome
{
    eSen_UpsellOutcome_None,
    eSen_UpsellOutcome_Declined,
    eSen_UpsellOutcome_Cancelled,
    eSen_UpsellOutcome_Purchased
};

enum TelemetryJoinMethod
{
    eTelemetryJoinMethod_Host,
    eTelemetryJoinMethod_Matchmaking,
    eTelemetryJoinMethod_Invite,
    eTelemetryJoinMethod_FriendsList
};

enum ETelemetryChallenges
{
    eTelemetryChallenges_None = 0,
    eTelemetryChallenges_DeathByFall = 25,
    eTelemetryChallenges_DeathByLava = 26,
    eTelemetryChallenges_DeathByFire = 27,
    eTelemetryChallenges_DeathByDrowning = 28,
    eTelemetryChallenges_DeathBySuffocation = 29,
    eTelemetryChallenges_DeathByVoid = 30,
    eTelemetryChallenges_DeathByCactus = 31,
    eTelemetryChallenges_DeathByPlayer = 32,
    eTelemetryChallenges_DeathByPlayerProjectile = 33,
    eTelemetryChallenges_DeathByTnt = 34,
    eTelemetryChallenges_DeathByCreeper = 35,
    eTelemetryChallenges_DeathByWolf = 36,
    eTelemetryChallenges_DeathByZombie = 37,
    eTelemetryChallenges_DeathBySkeleton = 38,
    eTelemetryChallenges_DeathBySpider = 39,
    eTelemetryChallenges_DeathBySlime = 40,
    eTelemetryChallenges_DeathByGhast = 41,
    eTelemetryChallenges_DeathByZombiePigman = 42,
    eTelemetryChallenges_DeathByIronGolem = 59,
    eTelemetryChallenges_DeathBySnowGolem = 60,
    eTelemetryChallenges_DeathByCaveSpider = 61,
    eTelemetryChallenges_DeathByWitch = 62,
    eTelemetryChallenges_DeathByEnderman = 63,
    eTelemetryChallenges_DeathByBlaze = 64,
    eTelemetryChallenges_DeathByMagmaCube = 65,
    eTelemetryChallenges_DeathByWither = 66,
    eTelemetryChallenges_DeathBySilverfish = 67,
    eTelemetryChallenges_DeathByEnderDragon = 68,
    eTelemetryChallenges_DeathByLightning = 69,
    eTelemetryChallenges_DeathByPhantom = 70,
    eTelemetryChallenges_DeathByGuardian = 71,
    eTelemetryChallenges_DeathByEndermite = 72,
    eTelemetryChallenges_DeathByRabbit = 73,
    eTelemetryChallenges_DeathByPolarBear = 74,
    eTelemetryChallenges_DeathByShulker = 75,
    eTelemetryChallenges_DeathByLlama = 80,
    eTelemetryChallenges_DeathByEvoker = 81,
    eTelemetryChallenges_DeathByVindicator = 82,
    eTelemetryChallenges_DeathByDolphin = 83,
    eTelemetryChallenges_DeathByPufferfish = 84
};

struct TelemetryDamageSource
{
    ETelemetryChallenges challenges;
    int causeOfDeathId;
    int playerWeaponId;
};

class CTelemetryManager
{
public:
    /*
     * The Wii U GHS ABI puts this class's vptr after its 36 bytes of state.
     * That is unusual enough that spelling the object as a normal C++
     * polymorphic class would move every observed member by four bytes under
     * the host compiler.  The provider is not subclassed in this source tree,
     * so the native declarations below remain direct entry points while the
     * original physical vtable slot is retained as the final word.
     */
    CTelemetryManager();
    ~CTelemetryManager();

    const CTelemetryManagerVTable* GetVTable() const;

    unsigned int Init();
    unsigned int Tick();
    unsigned int Flush();
    unsigned int RecordPlayerSessionStart(int playerId);
    unsigned int RecordPlayerSessionExit(int playerId, int disconnectReason);
    unsigned int RecordHeartBeat(int playerId);
    unsigned int RecordLevelStart(int playerId, ESen_FriendOrMatch friendOrMatch,
                                  ESen_CompeteOrCoop competeOrCoop, int difficultyLevel,
                                  int localPlayerCount, int onlinePlayerCount);
    unsigned int RecordLevelExit(int playerId, ESen_LevelExitStatus exitStatus);
    unsigned int RecordLevelSaveOrCheckpoint(int playerId, int saveSlot, int saveSize);
    unsigned int RecordLevelResume(int playerId, ESen_FriendOrMatch friendOrMatch,
                                   ESen_CompeteOrCoop competeOrCoop, int difficultyLevel,
                                   int localPlayerCount, int onlinePlayerCount,
                                   int secondsPlayed);
    unsigned int RecordPauseOrInactive(int playerId);
    unsigned int RecordUnpauseOrActive(int playerId);
    unsigned int RecordMenuShown(int playerId, EUIScene scene, int previousScene);
    unsigned int RecordAchievementUnlocked(int playerId, int achievementId, int gamerScore);
    unsigned int RecordMediaShareUpload(int playerId, ESen_MediaDestination destination,
                                        ESen_MediaType mediaType);
    unsigned int RecordUpsellPresented(int playerId, ESen_UpsellID upsellId, int upsellContext);
    unsigned int RecordUpsellResponded(int playerId, ESen_UpsellID upsellId, int upsellContext,
                                       ESen_UpsellOutcome outcome);
    unsigned int RecordPlayerDiedOrFailed(int playerId, int locationX, int locationY, int locationZ,
                                          int enemyTypeId, int playerWeaponId, int causeOfDeathId,
                                          ETelemetryChallenges challenges);
    unsigned int RecordEnemyKilledOrOvercome(int playerId, int locationX, int locationY, int locationZ,
                                             int enemyTypeId, int playerWeaponId, int causeOfDeathId,
                                             ETelemetryChallenges challenges);
    unsigned int RecordTexturePackLoaded(int playerId, int texturePackId, bool isDefault);
    unsigned int RecordSkinChanged(int playerId, int skinId);
    unsigned int RecordBanLevel(int playerId);
    unsigned int RecordUnBanLevel(int playerId);
    unsigned int PrepareMiniGameHostOptions(int playerId, const LaunchMoreOptionsMenuInitData* initData,
                                            TelemetryJoinMethod joinMethod);
    unsigned int RecordMiniGameHostOptions(int playerId, EMiniGameId miniGameId);
    unsigned int RecordPlayerSettings(int playerId);
    unsigned int RecordBlockPlaced(boost::shared_ptr<LocalPlayer> player, int blockId, int blockData,
                                   unsigned long long position);
    unsigned int RecordBlockBroken(boost::shared_ptr<LocalPlayer> player, int blockId, int blockData,
                                   unsigned long long position);
    unsigned int RecordItemAcquired(boost::shared_ptr<LocalPlayer> player, int itemId, int itemData,
                                    int count, unsigned long long position, const Vec3* location);
    unsigned int RecordItemUsed(boost::shared_ptr<LocalPlayer> player, int itemId, int itemData,
                                unsigned long long position, int count, const Vec3* location);
    unsigned int RecordGameModeRoundStart(const std::wstring& gameModeName,
                                          const std::wstring& roundName);
    unsigned int RecordGameModeRoundFinished();
    unsigned int RecordSavesInformation(int playerId, const SAVE_DETAILS* saves, unsigned int saveCount);
    int GetMultiplayerInstanceID();
    int GenerateMultiplayerInstanceId();
    void SetMultiplayerInstanceId(int instanceId);
    void RecordTransitionToLobby();

public:
    int GetSecondsSinceInitialize();
    unsigned int GetMode(unsigned int playerId);
    unsigned int GetSubMode(unsigned int playerId);
    unsigned int GetLevelId(unsigned int playerId);
    unsigned int GetSubLevelId(unsigned int playerId);
    int GetTitleBuildId();
    int GetLevelInstanceID();
    unsigned int GetSingleOrMultiplayer();
    unsigned int GetDifficultyLevel(int difficultyId);
    unsigned int GetLicense();
    unsigned int GetDefaultGameControls(unsigned int playerId);
    unsigned int GetAudioSettings(unsigned int playerId);
    int GetLevelExitProgressStat1();
    int GetLevelExitProgressStat2();

    std::uint32_t mUnknown0;
    float mInitialiseTime;
    bool mLevelStarted;
    float mLevelStartTime[4];
    int mMultiplayerInstanceId;
    int mLevelInstanceId;
    std::uint32_t mVTable;
};

static_assert(sizeof(CTelemetryManager) == 40, "CTelemetryManager Wii U object size");
static_assert(offsetof(CTelemetryManager, mInitialiseTime) == 4, "CTelemetryManager state ABI");
static_assert(offsetof(CTelemetryManager, mMultiplayerInstanceId) == 28, "CTelemetryManager state ABI");
static_assert(offsetof(CTelemetryManager, mLevelInstanceId) == 32, "CTelemetryManager state ABI");
static_assert(offsetof(CTelemetryManager, mVTable) == 36, "CTelemetryManager GHS vptr ABI");

template <typename Return, typename... Arguments>
Return CTelemetryManagerDispatch(CTelemetryManager* manager, unsigned int slot,
                                 Arguments... arguments)
{
    typedef Return (*Function)(CTelemetryManager*, Arguments...);
    const CTelemetryManagerVTableEntry& entry = manager->GetVTable()->slots[slot];
    Function function = reinterpret_cast<Function>(static_cast<std::uintptr_t>(entry.function));
    return function(manager, arguments...);
}

#endif
