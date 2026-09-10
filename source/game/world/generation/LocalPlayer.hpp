#ifndef GAME_WORLD_GENERATION_LOCALPLAYER_HPP
#define GAME_WORLD_GENERATION_LOCALPLAYER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/core/SmoothFloat.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/item/not_null_ptr.hpp"

class Minecraft;
class Level;
class User;
class Input;
class DamageSource;
class GameType;
class InteractionObject;
class MobEffect;
class Stat;
class SoundEvent;
class ClientPacketListener;
class ItemEntity;
class Container;
class AbstractHorse;
class Merchant;
class SignBlockEntity;
class CommandBlockEntity;
class EntityDataAccessor_Base;
class ModelPart;
class ItemInstance;
class MobEffectInstance;
class MultiPlayerGameMode;
struct TelemetryDamageSource;
class PlayerRenderer;

class LocalPlayer : public AbstractClientPlayer
{
public:
    LocalPlayer(Minecraft* minecraft, Level* level, User* user, int playerIndex);
    virtual ~LocalPlayer();

    virtual bool hasPermission(EGameCommand command);
    virtual BlockPos getCommandSenderBlockPosition();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void tick();
    virtual Vec3* getViewVector(float partialTick);
    virtual bool isInWall();
    virtual void rideTick();
    virtual bool startRiding(boost::shared_ptr<Entity> vehicle, bool force);
    virtual void stopRiding();
    virtual void handleEntityEvent(unsigned char eventId, int data);
    virtual bool isSneaking();
    virtual bool isSprinting();
    virtual void setSprinting(bool sprinting);
    virtual boost::shared_ptr<Entity> changeDimension(int dimension);
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual void closeContainer();
    virtual boost::shared_ptr<ItemEntity> drop(bool dropAll);
    virtual void reallyDrop(boost::shared_ptr<ItemEntity> item);
    virtual void openTextEdit(boost::shared_ptr<SignBlockEntity> sign);
    virtual void openCommandBlock(boost::shared_ptr<CommandBlockEntity> block);
    virtual int openTrading(boost::shared_ptr<Merchant> merchant, const std::wstring& name, int level);
    virtual int openContainer(boost::shared_ptr<Container> container);
    virtual int openHorseInventory(boost::shared_ptr<AbstractHorse> horse, boost::shared_ptr<Container> container);
    virtual int startBlockInteraction(InteractionObject& interaction);
    virtual void openItemInstanceGui(not_null_ptr<ItemInstance> item, InteractionHand::EInteractionHand hand);
    virtual void crit(boost::shared_ptr<Entity> entity);
    virtual void magicCrit(boost::shared_ptr<Entity> entity);
    virtual void serverAiStep();
    virtual float tickHeadTurn(float yRot, float xRot);
    virtual void aiStep();
    virtual MobEffectInstance* removeEffectNoUpdate(MobEffect* effect);
    virtual void move(MoverType type, double xa, double ya, double za, bool collide);
    virtual int openFireworks(int slot, int x, int y);
    virtual void displayClientMessage(int message, bool force);
    virtual void awardStat(Stat* stat, arrayWithLength<unsigned char> params);
    virtual unsigned int GetThirdPersonView(bool);
    virtual void SetThirdPersonView(int);
    virtual unsigned int CanChangeThirdPersonView();
    virtual void onCrafted(not_null_ptr<ItemInstance> item);
    virtual void handleCollectItem(not_null_ptr<ItemInstance> item);
    virtual void SpectatePlayerNext();
    virtual void SpectatePlayerPrev();
    virtual void StopSpectatingPlayer();
    virtual const GameType* GetGameType();
    virtual void SetGameType(const GameType* gameType);
    virtual unsigned long long GetType();

    bool isHandsBusy();
    unsigned long long SetSessionTimerStart();
    bool IsWaitingForScoreboard();
    bool doesLevelAllowBlockToBreak(const BlockPos& pos);
    unsigned int pickBlock(bool);
    unsigned char handleMouseClick(int);
    unsigned int creativeModeHandleMouseClick(int, bool);
    unsigned long long handleMouseDown(int, bool);
    void SetWaitingForScoreboard(bool);
    void updateRichPresence();
    void ResetPercentageTraversed();
    void heal(float);
    bool isInvisible();
    bool canMoveOrBeMovedFromTo(double, double, double, double, double, double, Vec3*);
    int getIndicatorSegment(float, float);
    void updateProximityIndicator();
    bool isControlledCamera();
    unsigned long long sendPosition();
    unsigned long long chat(const std::wstring&);
    bool swing(InteractionHand::EInteractionHand);
    void respawn();
    void actuallyHurt(const DamageSource*, float);
    void clientSideCloseContainer();
    void hurtTo(float, TelemetryDamageSource);
    void onUpdateAbilities();
    bool isLocalPlayer();
    void updateDamageIndicator(float, float, bool);
    double getJumpRidingScale();
    unsigned long long sendRidingJump();
    unsigned long long sendOpenInventory();
    unsigned int getPermissionLevel();
    void setPermissionLevel(int);
    unsigned int freeAtAndAbove(const BlockPos&);
    unsigned int freeAt(const BlockPos&);
    bool checkInBlock(double, double, double);
    void setExperienceValues(float, int, int);
    void playSound(const SoundEvent*, float, float);
    bool isEffectiveAi();
    void startUsingItem(InteractionHand::EInteractionHand);
    bool isUsingItem();
    void stopUsingItem();
    InteractionHand::EInteractionHand getUsedItemHand();
    bool isRidingJumpable();
    bool isAutoJumpEnabled();
    void updateAutoJump(float xMovement, float zMovement);
    void mapPlayerChunk(unsigned int targetBlockId);
    double getDamageTaken();
    bool isSolidBlock(int x, int y, int z);
    not_null_ptr<ItemInstance> getCarriedItem();
    MultiPlayerGameMode* GetLocalGameMode();
    unsigned int GetGameMode();
    void StartSpectating();
    void OnEquipArmor(not_null_ptr<ItemInstance> item);
    void OnTakeFromAnvil(not_null_ptr<ItemInstance> item);
    double getUnderwaterVisionClarity();
    void DoAutoAttackOnTouch(boost::shared_ptr<Entity>);
    double GetPercentageTraversed() const;
    void SetPercentageTraversed(double);
    void setDamageNearbyMobs(bool);
    bool getDamageNearbyMobs();
    void setAndBroadcastCustomSkin(unsigned int);
    void setAndBroadcastCustomCape(unsigned int);

    void setHandsBusy(bool);
    unsigned long long getSessionTimer();
    unsigned long long getAndResetChangeDimensionTimer();
    void SetPlayerAdditionalModelParts(std::vector<ModelPart*>);
    int getTicksInWater();

private:
    friend class Minecraft;
    friend class PlayerRenderer;

    unsigned char mReserved0x850[0x28];
    ClientPacketListener* mPacketListener;
    int mPermissionLevel;
    double mLastSentX;
    double mLastSentY;
    double mLastSentZ;
    float mLastSentYRot;
    float mLastSentXRot;
    int mPositionSendCounter;
    bool mPreviousOnGround;
    unsigned char mReserved0x8a1[0x0f];
    Input* mInput;
    Minecraft* mMinecraft;
    unsigned char mReserved0x8b8[0xc];
    unsigned int mIdleTicks;
    unsigned char mReserved0x8c8[4];
    int mSprinting;
    int mThirdPersonViewTimer;
    unsigned char mReserved0x8d0[0x18];
    float mFallFlyYaw;
    float mFallFlyYawOld;
    unsigned char mReserved0x8f0[0x08];
    bool mItemUsePending;
    unsigned char mReserved0x8f9[3];
    InteractionHand::EInteractionHand mUsedItemHand;
    bool mHandsBusy;
    bool mAutoJumpEnabled;
    unsigned char mReserved0x902[2];
    int mAutoJumpTime;
public:
    int mViewport;

private:
    unsigned char mReserved0x90c[4];

public:
    int mButtonsDown;

private:
    unsigned int mButtonsPressed;
    unsigned int mMoveInputX;
    unsigned int mMoveInputY;
    unsigned int mLookInputX;
    unsigned int mLookInputY;
    unsigned int mReserved0x928;
    unsigned int mReserved0x92c;
    int mDestroyTicks;
    int mLastAttackTick;
    int mLastUseTick;
    unsigned char mReserved0x93c[4];
    int mThirdPersonView;
    int mGameMode;
    bool mDRCTouching;
    unsigned char mReserved0x949[0x17];

public:
    int mUserIndex;

private:
    unsigned char mReserved0x964[4];
    SmoothFloat mViewBob;
    SmoothFloat mViewBobPrev;
    SmoothFloat mViewBobAmount;
    unsigned char mReserved0x98c[0x1c];
    int mCreativeRepeatState;
    unsigned char mReserved0x9ac[0xc];
    float mDamageTaken;
    bool mHasOpenedInventory;
    bool mWaitingForScoreboard;
    unsigned char mReserved0x9be[2];
    double mSessionTimer;
    std::vector<ModelPart*> mAdditionalModelParts;
    unsigned char mReserved0x9d4[4];
    float mWaterVisionClarity;
    float mWaterVisionClarityO;
    float mWaterVisionClarityTarget;
    unsigned char mReserved0x9e0[0x1c];
    double mPercentageTraversed;
    bool mDamageNearbyMobs;
    unsigned char mReserved0xa09[7];
};

#endif
