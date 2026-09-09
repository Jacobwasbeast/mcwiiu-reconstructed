#include "game/server/ServerPlayerGameMode.hpp"

#include "game/core/GameRulesInstance.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/AABB.hpp"
#include "game/server/ServerLevel.hpp"

ServerPlayerGameMode::ServerPlayerGameMode(Level* level, MasterGameMode* masterGameMode)
    : GameMode(), mLevel(level), mPlayer(NULL), mDestroying(false), mDestroyProgress(0),
      mDestroyPos(), mLastSentState(0), mIsDestroying(false), mPreviousDestroyPos(),
      mLastSentState2(0), mGameModeTicks(0), mGameRules(NULL),
      mMasterGameMode(masterGameMode), mDeaths(0), mKills(0), mUnknown58(0),
      mLastCheckpoint(0)
{
}

ServerPlayerGameMode::~ServerPlayerGameMode()
{
}

void ServerPlayerGameMode::setLevel(ServerLevel* level) { mLevel = level; }
void ServerPlayerGameMode::updateGameMode(const GameType* gameType)
{
    if (mGameType == GameType::NOT_SET)
        SetGameType(gameType);
}
void ServerPlayerGameMode::setGameRules(GameRulesInstance* gameRules) { mGameRules = gameRules; }
void ServerPlayerGameMode::nullifyGameRules() { mGameRules = NULL; }
void ServerPlayerGameMode::setLastCheckpoint(int checkpoint) { mLastCheckpoint = checkpoint; }
unsigned int ServerPlayerGameMode::getLastCheckpoint() { return mLastCheckpoint; }
void ServerPlayerGameMode::RestoreFromOld(ServerPlayerGameMode* oldMode)
{
    SetDeaths(oldMode->GetDeaths());
    setLastCheckpoint(oldMode->getLastCheckpoint());
}
boost::shared_ptr<ServerPlayer> ServerPlayerGameMode::getServerPlayer() const
{
    return boost::static_pointer_cast<ServerPlayer>(mPlayer);
}
int ServerPlayerGameMode::superDestroyBlock(const BlockPos& pos) { mPreviousDestroyPos = pos; return 0; }
int ServerPlayerGameMode::destroyBlock(const BlockPos& pos) { return superDestroyBlock(pos); }
void ServerPlayerGameMode::stopDestroyBlock(const BlockPos& pos) { mPreviousDestroyPos = pos; mDestroying = false; }
void ServerPlayerGameMode::abortDestroyBlock() { mDestroying = false; mDestroyProgress = 0; }
bool ServerPlayerGameMode::useItemOn(boost::shared_ptr<Player> player, Level* level,
                                     not_null_ptr<ItemInstance> item,
                                     InteractionHand::EInteractionHand hand, const BlockPos& pos,
                                     const Direction* direction, float hitX, float hitY, float hitZ,
                                     bool unknown, bool* result)
{
    mPreviousDestroyPos = pos;
    return false;
}
bool ServerPlayerGameMode::startDestroyBlock(const BlockPos& pos, const Direction* direction)
{
    mDestroyPos = pos; mDestroying = true; return true;
}
InteractionResult::EInteractionResult ServerPlayerGameMode::useItem(boost::shared_ptr<Player> player,
    Level* level, InteractionHand::EInteractionHand hand, bool simulate)
{
    return InteractionResult::eInteractionResult_Pass;
}
void ServerPlayerGameMode::SetGameType(const GameType* gameType) { mGameType = gameType; }
const GameType* ServerPlayerGameMode::GetGameType() { return mGameType; }
bool ServerPlayerGameMode::IsNearCentreOf(const AABB* box, ServerPlayer* player, double x, double y, double z)
{
    return box != NULL && x >= box->mMinX && x <= box->mMaxX && z >= box->mMinZ && z <= box->mMaxZ;
}
void ServerPlayerGameMode::tick() { ++mGameModeTicks; }
void ServerPlayerGameMode::OnDeath(boost::shared_ptr<ServerPlayer> player) { ++mDeaths; }
unsigned int ServerPlayerGameMode::GetDeaths() { return mDeaths; }
void ServerPlayerGameMode::SetDeaths(int deaths) { mDeaths = deaths; }
unsigned int ServerPlayerGameMode::GetKills() { return 0; }
void ServerPlayerGameMode::SetKills(int) {}

void ServerPlayerGameMode::RecordDamage(boost::shared_ptr<Entity> entity, CombatEntry* entry,
                                        boost::shared_ptr<ClientboundChatPacket> packet)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordDamage(entity, entry, packet);
}
void ServerPlayerGameMode::RecordDeath(boost::shared_ptr<Entity> entity) { ++mDeaths; }
void ServerPlayerGameMode::RecordItem(not_null_ptr<ItemInstance> item) { ++mGameModeTicks; }
void ServerPlayerGameMode::RecordContainer(int type, int slot, int count, int data) { mGameModeTicks += count; }
void ServerPlayerGameMode::RecordWalk(int distance, bool sprinting, bool sneaking) { mGameModeTicks += distance; }
void ServerPlayerGameMode::RecordSwim(int distance, bool sprinting) { mGameModeTicks += distance; }
void ServerPlayerGameMode::RecordClimb(int distance) { mGameModeTicks += distance; }
void ServerPlayerGameMode::RecordUseItem(not_null_ptr<ItemInstance> item) { ++mGameModeTicks; }
void ServerPlayerGameMode::RecordJump() { ++mGameModeTicks; }
void ServerPlayerGameMode::RecordGainPoints(int points) { mGameModeTicks += points; }
void ServerPlayerGameMode::ResetTempPoints() { mGameModeTicks = 0; }
bool ServerPlayerGameMode::RecordSecondaryTargetReached(const std::wstring& name, int target,
                                                        GlideRingGenerator::eGlideRingSize size)
{
    if (gMinecraftServer == NULL || isSpectator())
        return false;
    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return false;
    return gMinecraftServer->mMasterGameMode->mGameStats.RecordSecondaryTargetReached(
        player->getSUID(), name, target, size) != 0;
}
bool ServerPlayerGameMode::RecordLastCheckpoint(int checkpoint, int lap, bool first, bool second, bool third)
{ mLastCheckpoint = checkpoint; return true; }
bool ServerPlayerGameMode::RecordLapCompleted()
{
    if (gMinecraftServer == NULL || isSpectator())
        return false;
    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return false;
    gMinecraftServer->mMasterGameMode->mGameStats.RecordLapCompleted(player->getSUID());
    return true;
}
void ServerPlayerGameMode::RecordThermalEntered() { ++mGameModeTicks; }
bool ServerPlayerGameMode::RecordProgress(double first, double second, double third, double fourth)
{
    if (gMinecraftServer == NULL || isSpectator())
        return false;
    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return false;
    gMinecraftServer->mMasterGameMode->mGameStats.RecordProgress(
        player->getSUID(), first, second, third, fourth);
    return true;
}
void ServerPlayerGameMode::RecordSpawnedAtCheckpoint(int checkpoint) { mLastCheckpoint = checkpoint; }
void ServerPlayerGameMode::RecordBlockPlaced(const BlockPos& pos, int blockId) { mPreviousDestroyPos = pos; }
void ServerPlayerGameMode::RecordBlockDestroyed(const BlockPos& pos, int blockId) { mPreviousDestroyPos = pos; }
void ServerPlayerGameMode::RecordBlockPresent(const BlockPos& pos, int blockId) { mPreviousDestroyPos = pos; }
void ServerPlayerGameMode::RecordLayerTime() { ++mGameModeTicks; }
void ServerPlayerGameMode::RecordSnowballHit() { ++mGameModeTicks; }
void ServerPlayerGameMode::RecordHitBySnowball() { ++mGameModeTicks; }
void ServerPlayerGameMode::RecordLanding(const BlockPos& pos)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
    {
        double y = static_cast<double>(pos.y);
        int layer = masterGameMode->GetLayer(y);
        if (layer == static_cast<int>(masterGameMode->GetLayerCount()) - 1 &&
            masterGameMode->IsOnGeneratedLayer(layer, y))
        {
            masterGameMode->mGameStats.RecordLanding(mPlayer->getSUID());
        }
    }
}
const MiniGameDef* ServerPlayerGameMode::GetMiniGame() { return NULL; }
boost::shared_ptr<Player> ServerPlayerGameMode::GetPlayer() { return mPlayer; }
Level* ServerPlayerGameMode::GetLevel() { return mLevel; }
