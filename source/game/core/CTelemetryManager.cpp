#include "game/core/CTelemetryManager.hpp"

#include <cstring>
#include <new>

#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"

namespace
{
template <typename Function>
std::uint32_t TelemetryFunctionWord(Function function)
{
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(function));
}

std::uint32_t TelemetryInitThunk(CTelemetryManager* self) { return self->Init(); }
std::uint32_t TelemetryTickThunk(CTelemetryManager* self) { return self->Tick(); }
std::uint32_t TelemetryFlushThunk(CTelemetryManager* self) { return self->Flush(); }
std::uint32_t TelemetrySessionStartThunk(CTelemetryManager* self, int p) { return self->RecordPlayerSessionStart(p); }
std::uint32_t TelemetrySessionExitThunk(CTelemetryManager* self, int p, int r) { return self->RecordPlayerSessionExit(p, r); }
std::uint32_t TelemetryHeartBeatThunk(CTelemetryManager* self, int p) { return self->RecordHeartBeat(p); }
std::uint32_t TelemetryLevelStartThunk(CTelemetryManager* self, int p, ESen_FriendOrMatch f, ESen_CompeteOrCoop c, int d, int l, int o) { return self->RecordLevelStart(p, f, c, d, l, o); }
std::uint32_t TelemetryLevelExitThunk(CTelemetryManager* self, int p, ESen_LevelExitStatus s) { return self->RecordLevelExit(p, s); }
std::uint32_t TelemetryLevelSaveThunk(CTelemetryManager* self, int p, int slot, int size) { return self->RecordLevelSaveOrCheckpoint(p, slot, size); }
std::uint32_t TelemetryLevelResumeThunk(CTelemetryManager* self, int p, ESen_FriendOrMatch f, ESen_CompeteOrCoop c, int d, int l, int o, int seconds) { return self->RecordLevelResume(p, f, c, d, l, o, seconds); }
std::uint32_t TelemetryPauseThunk(CTelemetryManager* self, int p) { return self->RecordPauseOrInactive(p); }
std::uint32_t TelemetryUnpauseThunk(CTelemetryManager* self, int p) { return self->RecordUnpauseOrActive(p); }
std::uint32_t TelemetryMenuThunk(CTelemetryManager* self, int p, EUIScene s, int previous) { return self->RecordMenuShown(p, s, previous); }
std::uint32_t TelemetryAchievementThunk(CTelemetryManager* self, int p, int a, int score) { return self->RecordAchievementUnlocked(p, a, score); }
std::uint32_t TelemetryMediaThunk(CTelemetryManager* self, int p, ESen_MediaDestination d, ESen_MediaType t) { return self->RecordMediaShareUpload(p, d, t); }
std::uint32_t TelemetryUpsellPresentedThunk(CTelemetryManager* self, int p, ESen_UpsellID id, int context) { return self->RecordUpsellPresented(p, id, context); }
std::uint32_t TelemetryUpsellRespondedThunk(CTelemetryManager* self, int p, ESen_UpsellID id, int context, ESen_UpsellOutcome outcome) { return self->RecordUpsellResponded(p, id, context, outcome); }
std::uint32_t TelemetryDiedThunk(CTelemetryManager* self, int p, int x, int y, int z, int enemy, int weapon, int cause, ETelemetryChallenges c) { return self->RecordPlayerDiedOrFailed(p, x, y, z, enemy, weapon, cause, c); }
std::uint32_t TelemetryKilledThunk(CTelemetryManager* self, int p, int x, int y, int z, int enemy, int weapon, int cause, ETelemetryChallenges c) { return self->RecordEnemyKilledOrOvercome(p, x, y, z, enemy, weapon, cause, c); }
std::uint32_t TelemetryTextureThunk(CTelemetryManager* self, int p, int pack, bool d) { return self->RecordTexturePackLoaded(p, pack, d); }
std::uint32_t TelemetrySkinThunk(CTelemetryManager* self, int p, int skin) { return self->RecordSkinChanged(p, skin); }
std::uint32_t TelemetryBanThunk(CTelemetryManager* self, int p) { return self->RecordBanLevel(p); }
std::uint32_t TelemetryUnbanThunk(CTelemetryManager* self, int p) { return self->RecordUnBanLevel(p); }
std::uint32_t TelemetryPrepareThunk(CTelemetryManager* self, int p, const LaunchMoreOptionsMenuInitData* data, TelemetryJoinMethod m) { return self->PrepareMiniGameHostOptions(p, data, m); }
std::uint32_t TelemetryMiniGameThunk(CTelemetryManager* self, int p, EMiniGameId id) { return self->RecordMiniGameHostOptions(p, id); }
std::uint32_t TelemetrySettingsThunk(CTelemetryManager* self, int p) { return self->RecordPlayerSettings(p); }
std::uint32_t TelemetryBlockPlacedThunk(CTelemetryManager* self, boost::shared_ptr<LocalPlayer> p, int id, int data, unsigned long long pos) { return self->RecordBlockPlaced(p, id, data, pos); }
std::uint32_t TelemetryBlockBrokenThunk(CTelemetryManager* self, boost::shared_ptr<LocalPlayer> p, int id, int data, unsigned long long pos) { return self->RecordBlockBroken(p, id, data, pos); }
std::uint32_t TelemetryItemAcquiredThunk(CTelemetryManager* self, boost::shared_ptr<LocalPlayer> p, int id, int data, int count, unsigned long long pos, const Vec3* location) { return self->RecordItemAcquired(p, id, data, count, pos, location); }
std::uint32_t TelemetryItemUsedThunk(CTelemetryManager* self, boost::shared_ptr<LocalPlayer> p, int id, int data, unsigned long long pos, int count, const Vec3* location) { return self->RecordItemUsed(p, id, data, pos, count, location); }
std::uint32_t TelemetryRoundStartThunk(CTelemetryManager* self, const std::wstring& game, const std::wstring& round) { return self->RecordGameModeRoundStart(game, round); }
std::uint32_t TelemetryRoundFinishedThunk(CTelemetryManager* self) { return self->RecordGameModeRoundFinished(); }
std::uint32_t TelemetrySavesThunk(CTelemetryManager* self, int p, const SAVE_DETAILS* saves, unsigned int count) { return self->RecordSavesInformation(p, saves, count); }
int TelemetryGetIdThunk(CTelemetryManager* self) { return self->GetMultiplayerInstanceID(); }
int TelemetryGenerateIdThunk(CTelemetryManager* self) { return self->GenerateMultiplayerInstanceId(); }
void TelemetrySetIdThunk(CTelemetryManager* self, int id) { self->SetMultiplayerInstanceId(id); }
void TelemetryLobbyThunk(CTelemetryManager* self) { self->RecordTransitionToLobby(); }

void TelemetryDestructorThunk(CTelemetryManager* self, unsigned int flags)
{
    if ((flags & 1u) == 0u)
        return;
    ::operator delete(self);
}

const CTelemetryManagerVTable gCTelemetryManagerVTable = {
    {0, 0, 0},
    {{TelemetryFunctionWord(&TelemetryDestructorThunk), 0},
     {TelemetryFunctionWord(&TelemetryInitThunk), 0},
     {TelemetryFunctionWord(&TelemetryTickThunk), 0},
     {TelemetryFunctionWord(&TelemetryFlushThunk), 0},
     {TelemetryFunctionWord(&TelemetrySessionStartThunk), 0},
     {TelemetryFunctionWord(&TelemetrySessionExitThunk), 0},
     {TelemetryFunctionWord(&TelemetryHeartBeatThunk), 0},
     {TelemetryFunctionWord(&TelemetryLevelStartThunk), 0},
     {TelemetryFunctionWord(&TelemetryLevelExitThunk), 0},
     {TelemetryFunctionWord(&TelemetryLevelSaveThunk), 0},
     {TelemetryFunctionWord(&TelemetryLevelResumeThunk), 0},
     {TelemetryFunctionWord(&TelemetryPauseThunk), 0},
     {TelemetryFunctionWord(&TelemetryUnpauseThunk), 0},
     {TelemetryFunctionWord(&TelemetryMenuThunk), 0},
     {TelemetryFunctionWord(&TelemetryAchievementThunk), 0},
     {TelemetryFunctionWord(&TelemetryMediaThunk), 0},
     {TelemetryFunctionWord(&TelemetryUpsellPresentedThunk), 0},
     {TelemetryFunctionWord(&TelemetryUpsellRespondedThunk), 0},
     {TelemetryFunctionWord(&TelemetryDiedThunk), 0},
     {TelemetryFunctionWord(&TelemetryKilledThunk), 0},
     {TelemetryFunctionWord(&TelemetryTextureThunk), 0},
     {TelemetryFunctionWord(&TelemetrySkinThunk), 0},
     {TelemetryFunctionWord(&TelemetryBanThunk), 0},
     {TelemetryFunctionWord(&TelemetryUnbanThunk), 0},
     {TelemetryFunctionWord(&TelemetryPrepareThunk), 0},
     {TelemetryFunctionWord(&TelemetryMiniGameThunk), 0},
     {TelemetryFunctionWord(&TelemetrySettingsThunk), 0},
     {TelemetryFunctionWord(&TelemetryBlockPlacedThunk), 0},
     {TelemetryFunctionWord(&TelemetryBlockBrokenThunk), 0},
     {TelemetryFunctionWord(&TelemetryItemAcquiredThunk), 0},
     {TelemetryFunctionWord(&TelemetryItemUsedThunk), 0},
     {TelemetryFunctionWord(&TelemetryRoundStartThunk), 0},
     {TelemetryFunctionWord(&TelemetryRoundFinishedThunk), 0},
     {TelemetryFunctionWord(&TelemetrySavesThunk), 0},
     {TelemetryFunctionWord(&TelemetryGetIdThunk), 0},
     {TelemetryFunctionWord(&TelemetryGenerateIdThunk), 0},
     {TelemetryFunctionWord(&TelemetrySetIdThunk), 0},
     {TelemetryFunctionWord(&TelemetryLobbyThunk), 0}}};
}

CTelemetryManager::CTelemetryManager()
    : mUnknown0(0), mInitialiseTime(0.0f), mLevelStarted(false), mLevelStartTime{0.0f, 0.0f, 0.0f, 0.0f},
      mMultiplayerInstanceId(0), mLevelInstanceId(0),
      mVTable(TelemetryFunctionWord(&gCTelemetryManagerVTable))
{
}

const CTelemetryManagerVTable* CTelemetryManager::GetVTable() const
{
    return reinterpret_cast<const CTelemetryManagerVTable*>(static_cast<std::uintptr_t>(mVTable));
}

CTelemetryManager::~CTelemetryManager()
{
}

unsigned int CTelemetryManager::Init()
{
    return 0;
}

unsigned int CTelemetryManager::Tick()
{
    return 0;
}

unsigned int CTelemetryManager::Flush()
{
    return 0;
}

unsigned int CTelemetryManager::RecordPlayerSessionStart(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordPlayerSessionExit(int playerId, int disconnectReason)
{
    return 1;
}

unsigned int CTelemetryManager::RecordHeartBeat(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordLevelStart(int playerId, ESen_FriendOrMatch friendOrMatch,
                                                 ESen_CompeteOrCoop competeOrCoop,
                                                 int difficultyLevel, int localPlayerCount,
                                                 int onlinePlayerCount)
{
    if (playerId == gProfile->GetPrimaryPad())
        mLevelStarted = true;

    ++mLevelInstanceId;
    mLevelStartTime[playerId] = g_pApp->getAppTime();
    return 1;
}

unsigned int CTelemetryManager::RecordLevelExit(int playerId, ESen_LevelExitStatus exitStatus)
{
    return 1;
}

unsigned int CTelemetryManager::RecordLevelSaveOrCheckpoint(int playerId, int saveSlot, int saveSize)
{
    return 1;
}

unsigned int CTelemetryManager::RecordLevelResume(int playerId, ESen_FriendOrMatch friendOrMatch,
                                                  ESen_CompeteOrCoop competeOrCoop,
                                                  int difficultyLevel, int localPlayerCount,
                                                  int onlinePlayerCount, int secondsPlayed)
{
    return 1;
}

unsigned int CTelemetryManager::RecordPauseOrInactive(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordUnpauseOrActive(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordMenuShown(int playerId, EUIScene scene, int previousScene)
{
    return 1;
}

unsigned int CTelemetryManager::RecordAchievementUnlocked(int playerId, int achievementId,
                                                          int gamerScore)
{
    return 1;
}

unsigned int CTelemetryManager::RecordMediaShareUpload(int playerId,
                                                       ESen_MediaDestination destination,
                                                       ESen_MediaType mediaType)
{
    return 1;
}

unsigned int CTelemetryManager::RecordUpsellPresented(int playerId, ESen_UpsellID upsellId,
                                                      int upsellContext)
{
    return 1;
}

unsigned int CTelemetryManager::RecordUpsellResponded(int playerId, ESen_UpsellID upsellId,
                                                      int upsellContext,
                                                      ESen_UpsellOutcome outcome)
{
    return 1;
}

unsigned int CTelemetryManager::RecordPlayerDiedOrFailed(int playerId, int locationX, int locationY,
                                                         int locationZ, int enemyTypeId,
                                                         int playerWeaponId, int causeOfDeathId,
                                                         ETelemetryChallenges challenges)
{
    return 1;
}

unsigned int CTelemetryManager::RecordEnemyKilledOrOvercome(int playerId, int locationX,
                                                            int locationY, int locationZ,
                                                            int enemyTypeId, int playerWeaponId,
                                                            int causeOfDeathId,
                                                            ETelemetryChallenges challenges)
{
    return 1;
}

unsigned int CTelemetryManager::RecordTexturePackLoaded(int playerId, int texturePackId,
                                                        bool isDefault)
{
    return 1;
}

unsigned int CTelemetryManager::RecordSkinChanged(int playerId, int skinId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordBanLevel(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordUnBanLevel(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::PrepareMiniGameHostOptions(int playerId,
                                                           const LaunchMoreOptionsMenuInitData* initData,
                                                           TelemetryJoinMethod joinMethod)
{
    return 1;
}

unsigned int CTelemetryManager::RecordMiniGameHostOptions(int playerId, EMiniGameId miniGameId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordPlayerSettings(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordBlockPlaced(boost::shared_ptr<LocalPlayer> player, int blockId,
                                                  int blockData, unsigned long long position)
{
    return 1;
}

unsigned int CTelemetryManager::RecordBlockBroken(boost::shared_ptr<LocalPlayer> player, int blockId,
                                                  int blockData, unsigned long long position)
{
    return 1;
}

unsigned int CTelemetryManager::RecordItemAcquired(boost::shared_ptr<LocalPlayer> player, int itemId,
                                                   int itemData, int count,
                                                   unsigned long long position, const Vec3* location)
{
    return 1;
}

unsigned int CTelemetryManager::RecordItemUsed(boost::shared_ptr<LocalPlayer> player, int itemId,
                                               int itemData, unsigned long long position, int count,
                                               const Vec3* location)
{
    return 1;
}

unsigned int CTelemetryManager::RecordGameModeRoundStart(const std::wstring& gameModeName,
                                                         const std::wstring& roundName)
{
    return 1;
}

unsigned int CTelemetryManager::RecordGameModeRoundFinished()
{
    return 1;
}

unsigned int CTelemetryManager::RecordSavesInformation(int playerId, const SAVE_DETAILS* saves,
                                                       unsigned int saveCount)
{
    return 1;
}

int CTelemetryManager::GetMultiplayerInstanceID()
{
    return mMultiplayerInstanceId;
}

int CTelemetryManager::GenerateMultiplayerInstanceId()
{
    return 0;
}

void CTelemetryManager::SetMultiplayerInstanceId(int instanceId)
{
    mMultiplayerInstanceId = instanceId;
}

void CTelemetryManager::RecordTransitionToLobby()
{
}

unsigned int CTelemetryManager::GetMode(unsigned int playerId)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft == NULL)
        return 0;

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        return 4;

    if (playerId >= 4)
        return 0;

    const boost::shared_ptr<MultiplayerLocalPlayer>& player = minecraft->mLocalPlayers[playerId];
    if (!player)
        return 0;

    const GameType* gameType = NULL;
    if (player->mLevel != NULL && player->mLevel->getLevelData() != NULL)
        gameType = player->mLevel->getLevelData()->getGameType();

    if (gameType == NULL && g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x10)))
        gameType = player->GetGameType();

    if (gameType == GameType::SURVIVAL)
        return 1;
    if (gameType == GameType::CREATIVE)
        return 2;
    if (gameType == GameType::ADVENTURE)
        return 3;

    return 0;
}

unsigned int CTelemetryManager::GetLicense()
{
    if (gProfile->IsFullVersion())
        return 2;

    return 1;
}

unsigned int CTelemetryManager::GetAudioSettings(unsigned int playerId)
{
    if (playerId != (unsigned int)gProfile->GetPrimaryPad())
        return 0;

    int volume = GameSettings::GetGameSettings(playerId, (eGameSetting)1);
    if (volume == 0)
        return 1;
    if (volume == 100)
        return 2;

    return 3;
}
