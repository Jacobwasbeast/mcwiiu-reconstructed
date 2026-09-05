
#ifndef GAME_SERVER_SERVERPLAYER_HPP
#define GAME_SERVER_SERVERPLAYER_HPP

#include "game/world/generation/Player.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/item/not_null_ptr.hpp"

class AbstractContainerMenu;
class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class GameType;
class InteractionObject;
class Level;
class MinecraftServer;
class MobEffectInstance;
class ServerGamePacketListenerImpl;
class ServerLevel;
class ServerPlayerGameMode;
class Stat;
class BlockEntity;
class Container;

struct AbstractContainerMenu;
struct Block;
struct BlockPos;
struct ChunkPos;
struct CompoundTag;
struct DamageSource;
struct DataFixerUpper;
struct GameType;
struct InteractionObject;
struct Level;
struct MinecraftServer;
struct MobEffectInstance;
struct ServerGamePacketListenerImpl;
struct ServerLevel;
struct ServerPlayerGameMode;
struct Stat;
struct ServerPlayer : Player {
    
       
    ServerGamePacketListenerImpl* mConnection;
    uint32_t mPlayerId;   
    MinecraftServer* mServer;
    ServerPlayerGameMode* mGameMode;
    uint8_t mPadding1[4];
    double mLastSentX;
    double mLastSentZ;
    uint8_t mPadding2[4];
    uint32_t mContainer;   
    int32_t mContainerCounter;
    std::vector<int32_t> mSentEntityIds;
    boost::unordered_set<ChunkPos, ChunkPosKeyHash, ChunkPosKeyEq> mTrackedChunks;   
    float mViewDistance;
    int32_t mChunkX;
    int32_t mChunkZ;
    int32_t mChunkRadius;
    int32_t mChunkIndex;
    int32_t mChunkFlags;
    float mLastActionTime;
    int32_t mDimensionChangeDelay;
    uint8_t mSpectatorState;
    uint8_t mPadding3[4];
    int32_t mExperienceLevel;
    int32_t mExperienceTotal;
    int64_t mLastTick;
    int32_t mRemoveEntityCount;
    int32_t mRespawnDimension;
    uint32_t mEntitiesToRemove;   
    uint8_t mChangingDimension;
    uint8_t mPadding4[4];
    int32_t mSpawnInvulnerableTime;
    uint8_t mPvpState;
    uint8_t mPadding5[4];
    int32_t mViewDistanceModifier;
    uint8_t mCameraState;
    uint8_t mPadding6[4];
    int32_t latency;
    uint8_t mFallFlying;
    uint8_t mUsingItem;
    uint8_t mChangedDimension;
    uint8_t mPadding7[4];
    int32_t mTeleportPending;
    int32_t mCameraEntity;
    int32_t mType;
    uint8_t mPadding8[4];

    
    ServerPlayer(MinecraftServer* parameterOne, ServerLevel* parameterTwo, const std::wstring& parameterThree, ServerPlayerGameMode* parameterFour);   
    ~ServerPlayer();   
       
    void sendMessage(uint32_t parameterOne, int32_t* parameterTwo, uint32_t parameterThree, std::wstring* parameterFour, uint32_t parameterFive);   
    uint32_t hasPermission(uint32_t parameterOne);   
    BlockPos getCommandSenderBlockPosition();   
    uint64_t refreshContainer(AbstractContainerMenu* parameterOne, std::vector<not_null_ptr<ItemInstance> >* parameterTwo);   
    uint64_t slotChanged(AbstractContainerMenu* parameterOne, int32_t parameterTwo, not_null_ptr_ItemInstance* parameterThree);   
    uint64_t setContainerData(AbstractContainerMenu* parameterOne, int32_t parameterTwo, int32_t parameterThree);   
    uint64_t setAllContainerData(AbstractContainerMenu* parameterOne, boost::shared_ptr<Container>* parameterTwo);   
    uint64_t flagEntitiesToBeRemoved(uint32_t* parameterOne, bool* parameterTwo);   
    static int32_t getFlagIndexForChunk(ChunkPos* parameterOne, int32_t parameterTwo);   
    void disconnect();   
    Level* getLevel();   
    void doTickA();   
    uint64_t broadcast(boost::shared_ptr<BlockEntity>* parameterOne, bool parameterTwo);   
    void doChunkSendingTick(bool parameterOne, bool parameterTwo);   
    uint64_t doTickB();   
    void doTick(bool parameterOne, bool parameterTwo, bool parameterThree);   
    void initMenu();   
    uint64_t SpectatePlayerChange(uint32_t parameterOne, int32_t parameterTwo);   
    void updateClientsCameras();   
    uint64_t notifyPlayerRespawn(boost::shared_ptr<ServerPlayer>* parameterOne, boost::shared_ptr<ServerPlayer>* parameterTwo);   
    void flushEntitiesToRemove();   
    uint64_t markAttributesDirty();   
    void refreshContainer(AbstractContainerMenu* parameterOne);   
    void resetSentInfo();   
    static int32_t getFlagIndexForChunk(int32_t parameterOne, int32_t parameterTwo, int32_t parameterThree);   
    void resetChunkFlagsOutsideArea(int32_t parameterOne, int32_t parameterTwo, int32_t parameterThree, int32_t parameterFour, int32_t parameterFive);   
    void readAdditionalSaveData(CompoundTag* parameterOne);   
    static void addWalkers(DataFixerUpper* parameterOne);   
    void addAdditonalSaveData(CompoundTag* parameterOne);   
    void giveExperienceLevels(int32_t parameterOne);   
    void onEnchantmentPerformed(int32_t parameterOne);   
    void onEnterCombat();   
    void onLeaveCombat();   
    void ResetSpawnInvulnerableTime();   
    void tick();   
    uint64_t die(DamageSource* parameterOne);   
    int32_t hurt(uint64_t parameterOne, float parameterTwo);   
    void isPvpAllowed();
    uint32_t canHarmPlayer(boost::shared_ptr<Player>* parameterOne);   
    int32_t canHarmPlayer(std::wstring* parameterOne);   
    void addEffect(MobEffectInstance* parameterOne, boost::shared_ptr<Entity>* parameterTwo);   
    uint64_t changeDimension(uint32_t parameterOne, int32_t parameterTwo);   
    uint32_t broadcastToPlayer(boost::shared_ptr<ServerPlayer>* parameterOne);   
    uint64_t take(boost::shared_ptr<Entity>* parameterOne, int32_t parameterTwo);   
    int32_t startSleepInBed(BlockPos* parameterOne, bool parameterTwo);   
    uint64_t stopSleepInBed(bool parameterOne, bool parameterTwo, bool parameterThree);   
    uint32_t startRiding(boost::shared_ptr<Entity>* parameterOne);   
    void stopRiding();   
    void stopRidingBeforeTeleport();   
    uint8_t isChangingDimension();   
    uint32_t isInvulnerableTo(DamageSource* parameterOne);   
    void checkFallDamage(double parameterOne, bool parameterTwo, Block* parameterThree, BlockPos* parameterFour);   
    uint64_t doCheckFallDamage(double parameterOne, bool parameterTwo);   
    void nextContainerCounter();   
    void openTextEdit(uint32_t parameterOne);   
    uint32_t startBlockInteraction(InteractionObject* parameterOne);   
    uint32_t openFireworks(int32_t parameterOne, int32_t parameterTwo, int32_t parameterThree);   
    uint32_t openContainer(boost::shared_ptr<Container>* parameterOne);   
    uint32_t openTrading(uint32_t parameterOne, std::wstring* parameterTwo, int32_t parameterThree);   
    uint32_t openHorseInventory(uint32_t parameterOne, boost::shared_ptr<Container>* parameterTwo);   
    void openItemInstanceGui(not_null_ptr_ItemInstance* parameterOne, uint32_t parameterTwo);   
    void doCloseContainer();   
    void closeContainer();   
    void broadcastCarriedItem();   
    void setPlayerInput(float parameterOne, float parameterTwo, bool parameterThree, bool parameterFour);   
    void awardStat(Stat* parameterOne, arrayWithLength<unsigned char>* parameterTwo);   
    void resetStat(Stat* parameterOne);   
    uint8_t hasDisconnected();   
    uint64_t displayClientMessage(int32_t parameterOne, bool parameterTwo);   
    void completeUsingItem();   
    uint64_t restoreFrom(boost::shared_ptr<Player>* parameterOne, bool parameterTwo);   
    void onEffectAdded(MobEffectInstance* parameterOne);   
    void onEffectUpdated(MobEffectInstance* parameterOne, bool parameterTwo);   
    void onEffectRemoved(MobEffectInstance* parameterOne);   
    void teleportTo(double parameterOne, double parameterTwo, double parameterThree);   
    void teleportTo(double parameterOne, double parameterTwo, double parameterThree, float parameterFour, float parameterFive);   
    uint64_t crit(boost::shared_ptr<Entity>* parameterOne);   
    uint64_t magicCrit(boost::shared_ptr<Entity>* parameterOne);   
    void onUpdateAbilities();   
    void setGameMode(GameType* parameterOne);   
    bool isSpectator();   
    bool isCreative();   
    void resetLastActionTime();   
    uint64_t sendRemoveEntity(boost::shared_ptr<Entity>* parameterOne);   
    uint64_t cancelRemoveEntity(boost::shared_ptr<Entity>* parameterOne);   
    void updateInvisibilityStatus();   
    void processDimensionDelay();   
    uint64_t attack(boost::shared_ptr<Entity>* parameterOne);   
    uint32_t swing(uint32_t parameterOne);   
    void hasChangedDimension();   
    void startFallFlying();
    void stopFallFlying();
    uint32_t getPlayerViewDistanceModifier();   
    void handleCollectItem(not_null_ptr_ItemInstance* parameterOne);   
    uint64_t SetCamera(boost::shared_ptr<Entity>* parameterOne);   
    const GameType* GetGameType();
    ServerPlayerGameMode* GetGameMode();   
    uint64_t OnEquipArmor(not_null_ptr_ItemInstance* parameterOne);   
    int32_t checkTotemDeathProtection(DamageSource* parameterOne);   
    uint64_t GetType();   
    void getStats();
    void getLastActionTime();
};

#endif
