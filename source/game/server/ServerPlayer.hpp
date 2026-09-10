
#ifndef GAME_SERVER_SERVERPLAYER_HPP
#define GAME_SERVER_SERVERPLAYER_HPP

#include "game/world/generation/Player.hpp"
#include <cstdint>
#include <list>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"
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
class Merchant;
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
struct ServerPlayer : Player, net_minecraft_world_inventory::ContainerListener {
    
       
    boost::shared_ptr<ServerGamePacketListenerImpl> mConnection;
    MinecraftServer* mServer;
    ServerPlayerGameMode* mGameMode;
    uint8_t mPadding1[4];
    double mLastSentX;
    double mLastSentZ;
    std::list<ChunkPos> mLoadedChunks;
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
    double* mLevitationSourceY;
    uint8_t mChangingDimension;
    uint8_t mPadding4[4];
    int32_t mSpawnInvulnerableTime;
    uint8_t mPvpState;
    uint8_t mPadding5[4];
    int32_t mViewDistanceModifier;
    bool mIsChangingQuantityOnly;
    uint8_t mPadding6[4];
    int32_t latency;
    uint8_t mFallFlying;
    bool mRespawnState;
    uint8_t mChangedDimension;
    uint8_t mPadding7[4];
    int32_t mTeleportPending;
    int32_t mCameraEntity;
    int32_t mType;
    bool mDisconnected;
    uint8_t mPadding8[3];
    int32_t mContainerCounter;

    
    ServerPlayer(MinecraftServer* parameterOne, ServerLevel* parameterTwo, const std::wstring& parameterThree, ServerPlayerGameMode* parameterFour);   
    ~ServerPlayer();   
       
    void sendMessage(uint32_t parameterOne, int32_t* parameterTwo, uint32_t parameterThree, std::wstring* parameterFour, uint32_t parameterFive);   
    uint32_t hasPermission(uint32_t parameterOne);   
    BlockPos getCommandSenderBlockPosition();   
    virtual void refreshContainer(AbstractContainerMenu* menu, std::vector<not_null_ptr<ItemInstance> >* items);
    virtual void slotChanged(AbstractContainerMenu* menu, int32_t slot, not_null_ptr<ItemInstance> item);
    virtual void setContainerData(AbstractContainerMenu* menu, int32_t dataIndex, int32_t value);
    virtual void setAllContainerData(AbstractContainerMenu* menu, boost::shared_ptr<Container> container);
    uint64_t flagEntitiesToBeRemoved(uint32_t* parameterOne, bool* parameterTwo);   
    static int32_t getFlagIndexForChunk(ChunkPos* parameterOne, int32_t parameterTwo);   
    void disconnect();   
    ServerLevel* getLevel();
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
    virtual void addEffect(MobEffectInstance* parameterOne, const boost::shared_ptr<Entity>& parameterTwo);   
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
    void openTextEdit(boost::shared_ptr<SignBlockEntity> parameterOne);
    virtual int startBlockInteraction(InteractionObject& interaction);
    virtual int openFireworks(int32_t x, int32_t y, int32_t z);
    virtual int openContainer(boost::shared_ptr<Container> container);
    virtual int openTrading(boost::shared_ptr<Merchant> merchant, const std::wstring& name, int entityId);
    virtual int openHorseInventory(boost::shared_ptr<AbstractHorse> horse, boost::shared_ptr<Container> container);
    virtual void openItemInstanceGui(not_null_ptr<ItemInstance> item, InteractionHand::EInteractionHand hand);
    void doCloseContainer();
    virtual void closeContainer();
    void broadcastCarriedItem();   
    void setPlayerInput(float parameterOne, float parameterTwo, bool parameterThree, bool parameterFour);   
    void awardStat(Stat* stat, arrayWithLength<unsigned char> parameters);   
    void resetStat(Stat* parameterOne);   
    uint8_t hasDisconnected();   
    void displayClientMessage(int32_t parameterOne, bool parameterTwo);   
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
    uint64_t attack(boost::shared_ptr<Entity> parameterOne);
    virtual bool swing(InteractionHand::EInteractionHand parameterOne);   
    void hasChangedDimension();   
    void startFallFlying();
    void stopFallFlying();
    uint32_t getPlayerViewDistanceModifier();   
    void handleCollectItem(not_null_ptr<ItemInstance> parameterOne);
    void SetCamera(boost::shared_ptr<Entity> parameterOne);   
    const GameType* GetGameType();
    ServerPlayerGameMode* GetGameMode();   
    uint64_t OnEquipArmor(not_null_ptr_ItemInstance* parameterOne);   
    int32_t checkTotemDeathProtection(DamageSource* parameterOne);   
    uint64_t GetType();   
    void getStats();
    void getLastActionTime();
};

#endif
