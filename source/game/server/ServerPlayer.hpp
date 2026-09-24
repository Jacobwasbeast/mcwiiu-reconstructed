
#ifndef GAME_SERVER_SERVERPLAYER_HPP
#define GAME_SERVER_SERVERPLAYER_HPP

#include "game/world/generation/Player.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include <cstdint>
#include <cstddef>
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
class Biome;
class BlockEntity;
class Container;

class AbstractContainerMenu;
class Block;
class BlockPos;
class ChunkPos;
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
class ServerPlayer : public Player, public net_minecraft_world_inventory::ContainerListener {
public:
    
       
    boost::shared_ptr<ServerGamePacketListenerImpl> mConnection;
    MinecraftServer* mServer;
    ServerPlayerGameMode* mGameMode;
    uint8_t mPadding1[4];
    double mLastSentX;
    double mLastSentZ;
    std::list<ChunkPos> mLoadedChunks;
    std::vector<int32_t> mSentEntityIds;
    boost::unordered_set<ChunkPos, ChunkPosKeyHash, ChunkPosKeyEq> mTrackedChunks;   
    int32_t mState;
    Biome* mLastBiome;
    int64_t mChunkRequestTime;
    double* mLevitationSourceY;
    float mLastRecordedHealth;
    int32_t mLastRecordedFood;
    int32_t mLastRecordedAir;
    int32_t mLastRecordedArmor;
    int32_t mLastRecordedExperienceLevel;
    int32_t mLastRecordedTotalExperience;
    float mLastSentHealth;
    int32_t mLastSentFood;
    bool mLastSentSaturationWasZero;
    int32_t mLastSentTotalExperience;
    int32_t mSpawnInvulnerableTime;
    int64_t mLastActionTime;
    int32_t mLastChunkSendingTick;
    boost::shared_ptr<void> mSharedState;
    uint8_t mChangingDimension;
    BlockPos* mFallStartPosition;
    bool mDisconnected;
    int32_t mContainerCounter;
    bool mIsChangingQuantityOnly;
    int32_t latency;
    bool mRespawnState;
    uint8_t mFallFlying;
    uint8_t mChangedDimension;
    int32_t mPvpState;
    int32_t mCauseOfDeathId;
    int32_t mPlayerWeaponId;
    int32_t mChunkRadius;

    
    ServerPlayer(MinecraftServer* parameterOne, ServerLevel* parameterTwo, const std::wstring& parameterThree, ServerPlayerGameMode* parameterFour);   
    ~ServerPlayer();   
       
    void sendMessage(ClientboundChatPacket::EChatPacketMessage message, int32_t* intParams, uint32_t numIntParams, std::wstring* stringParams, uint32_t numStringParams) override;
    bool hasPermission(EGameCommand command) override;
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
    void giveExperienceLevels(int32_t parameterOne) override;
    void onEnchantmentPerformed(int32_t parameterOne) override;
    void onEnterCombat();   
    void onLeaveCombat();   
    void ResetSpawnInvulnerableTime();   
    void tick();   
    void die(const DamageSource* source) override;
    bool hurt(const DamageSource* source, float damage) override;
    bool isPvpAllowed();
    uint32_t canHarmPlayer(boost::shared_ptr<Player> parameterOne) override;
    bool canHarmPlayer(std::wstring parameterOne) override;
    virtual void addEffect(MobEffectInstance* parameterOne, const boost::shared_ptr<Entity>& parameterTwo);   
    virtual boost::shared_ptr<Entity> changeDimension(int dimension) override;
    bool broadcastToPlayer(boost::shared_ptr<ServerPlayer> player) override;
    void take(boost::shared_ptr<Entity> entity, int32_t count) override;
    uint32_t startSleepInBed(const BlockPos& position, bool force) override;
    void stopSleepInBed(bool parameterOne, bool parameterTwo, bool parameterThree) override;
    virtual bool startRiding(boost::shared_ptr<Entity> vehicle);
    void stopRiding();   
    void stopRidingBeforeTeleport();   
    uint8_t isChangingDimension();
    bool hasChangedDimensionState() const { return mChangedDimension != 0; }
    virtual bool isInvulnerableTo(const DamageSource* parameterOne);
    void checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos) override;
    uint64_t doCheckFallDamage(double parameterOne, bool parameterTwo);   
    void nextContainerCounter();   
    void openTextEdit(boost::shared_ptr<SignBlockEntity> parameterOne) override;
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
    void restoreFrom(boost::shared_ptr<Player> oldPlayer, bool keepEverything);
    void onEffectAdded(MobEffectInstance* parameterOne);   
    void onEffectUpdated(MobEffectInstance* parameterOne, bool parameterTwo);   
    void onEffectRemoved(MobEffectInstance* parameterOne);   
    void teleportTo(double parameterOne, double parameterTwo, double parameterThree);   
    virtual void teleportTo(double parameterOne, double parameterTwo, double parameterThree, float parameterFour, float parameterFive);
    void crit(boost::shared_ptr<Entity> entity) override;
    void magicCrit(boost::shared_ptr<Entity> entity) override;
    void onUpdateAbilities() override;
    void setGameMode(const GameType* gameType) override;
    bool isSpectator();   
    bool isCreative();   
    void resetLastActionTime();   
    uint64_t sendRemoveEntity(boost::shared_ptr<Entity>* parameterOne);   
    uint64_t cancelRemoveEntity(boost::shared_ptr<Entity>* parameterOne);   
    void updateInvisibilityStatus();   
    void processDimensionDelay();   
    uint64_t attack(boost::shared_ptr<Entity> parameterOne) override;
    virtual bool swing(InteractionHand::EInteractionHand parameterOne);   
    void hasChangedDimension();   
    void startFallFlying();
    void stopFallFlying();
    uint32_t getPlayerViewDistanceModifier();   
    void handleCollectItem(not_null_ptr<ItemInstance> parameterOne) override;
    void SetCamera(boost::shared_ptr<Entity> parameterOne) override;
    const GameType* GetGameType();
    ServerPlayerGameMode* GetGameMode() override;
    void OnEquipArmor(not_null_ptr<ItemInstance> item) override;
    bool checkTotemDeathProtection(const DamageSource* source) override;
    uint64_t GetType();   
    void getStats();
    void getLastActionTime();
};

#if UINTPTR_MAX == 0xffffffffu && !defined(RECON_NATIVE)
static_assert(offsetof(ServerPlayer, mRespawnState) == 0x934,
              "ServerPlayer respawn state must retain the Wii U +0x934 byte");
static_assert(offsetof(ServerPlayer, mFallFlying) == 0x935,
              "ServerPlayer fall-flying state must retain the Wii U +0x935 byte");
#endif

#endif
