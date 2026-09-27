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
    friend class CMinecraftApp;
    friend class GameRenderer;
    friend class Gui;

public:
    LocalPlayer(Minecraft* minecraft, Level* level, User* user, int dimension);
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
    virtual boost::shared_ptr<ItemEntity> drop(bool dropAll) override;
    not_null_ptr<ItemInstance> reallyDrop(boost::shared_ptr<ItemEntity> item) override;
    virtual void openTextEdit(boost::shared_ptr<SignBlockEntity> sign) override;
    virtual void openCommandBlock(boost::shared_ptr<CommandBlockEntity> block) override;
    virtual int openTrading(boost::shared_ptr<Merchant> merchant, const std::wstring& name, int level);
    virtual int openContainer(boost::shared_ptr<Container> container);
    virtual int openHorseInventory(boost::shared_ptr<AbstractHorse> horse, boost::shared_ptr<Container> container);
    virtual int startBlockInteraction(InteractionObject& interaction);
    virtual void openItemInstanceGui(not_null_ptr<ItemInstance> item, InteractionHand::EInteractionHand hand);
    virtual void crit(boost::shared_ptr<Entity> entity) override;
    virtual void magicCrit(boost::shared_ptr<Entity> entity) override;
    virtual void serverAiStep();
    virtual float tickHeadTurn(float yRot, float xRot);
    virtual void aiStep();
    virtual MobEffectInstance* removeEffectNoUpdate(MobEffect* effect);
    virtual void move(MoverType type, double xa, double ya, double za, bool collide);
    virtual unsigned long long sendPosition();
    virtual void displayClientMessage(int message, bool force);
    virtual void awardStat(Stat* stat, arrayWithLength<unsigned char> params);
    virtual unsigned int GetThirdPersonView(bool) override;
    virtual void SetThirdPersonView(int) override;
    virtual unsigned int CanChangeThirdPersonView() override;
    virtual void onCrafted(not_null_ptr<ItemInstance> item) override;
    virtual void handleCollectItem(not_null_ptr<ItemInstance> item) override;
    virtual void SpectatePlayerNext() override;
    virtual void SpectatePlayerPrev() override;
    virtual void StopSpectatingPlayer() override;
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
    virtual unsigned int getPermissionLevel();
    unsigned long long chat(const std::wstring&);
    bool swing(InteractionHand::EInteractionHand);
    void respawn() override;
    void actuallyHurt(const DamageSource*, float);
    void clientSideCloseContainer();
    void hurtTo(float, TelemetryDamageSource);
    void onUpdateAbilities() override;
    bool isLocalPlayer();
    void updateDamageIndicator(float, float, bool);
    double getJumpRidingScale();
    void sendRidingJump();
    void sendOpenInventory();
    virtual void setPermissionLevel(int);
    virtual int openFireworks(int x, int y, int z);
    unsigned int freeAtAndAbove(const BlockPos&);
    unsigned int freeAt(const BlockPos&);
    bool checkInBlock(double, double, double);
    void setExperienceValues(float progress, int total, int level);
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
    virtual void setAndBroadcastCustomSkin(unsigned int);
    MultiPlayerGameMode* GetLocalGameMode();
    GameMode* GetGameMode() override;
    virtual void setAndBroadcastCustomCape(unsigned int);
    void OnEquipArmor(not_null_ptr<ItemInstance> item) override;
    void OnTakeFromAnvil(not_null_ptr<ItemInstance> item) override;
    double getUnderwaterVisionClarity();
    void DoAutoAttackOnTouch(boost::shared_ptr<Entity>);
    double GetPercentageTraversed() const;
    void SetPercentageTraversed(double);
    void setDamageNearbyMobs(bool);
    bool getDamageNearbyMobs();
    virtual not_null_ptr<ItemInstance> getCarriedItem();
    virtual void StartSpectating();

    void setHandsBusy(bool);
    unsigned long long getSessionTimer();
    unsigned long long getAndResetChangeDimensionTimer();
    void SetPlayerAdditionalModelParts(std::vector<ModelPart*>);
    int getTicksInWater();

private:
    friend class Boat;
    friend class Minecraft;
    friend class PlayerRenderer;
    friend class ItemInHandRenderer;
    friend class UIScene_HUD;
    friend class Tooltips;
    friend class MultiplayerLocalPlayer;

protected:
    ClientPacketListener* mPacketListener;

private:
    int mPermissionLevel;
    double mLastSentX;
    double mLastSentY;
    double mLastSentZ;
    float mLastSentYRot;
    float mLastSentXRot;
    bool mPreviousOnGround;
    bool mWasSneaking;
    bool mWasSprinting;
    bool mWasSwimming;
    bool mWasDamagingNearbyMobs;
    int mPositionSendCounter;
    bool mHealthInitialized;
    Input* mInput;
    Minecraft* mMinecraft;
    int mSprintDelay;
    bool mSprintPending;
    bool mFlyTogglePending;
    int mRightClickDelay;
    unsigned int mIdleTicks;
    int mSprinting;
    int mThirdPersonViewTimer;
    float mViewYRot;
    float mViewXRot;
    float mViewYRotO;
    float mViewXRotO;
    float mPortalEffectIntensity;
    float mPortalEffectIntensityO;
    float mFallFlyYaw;
    float mFallFlyYawOld;
    float mFallFlyingAlignment;
    float mFallFlyingAlignmentO;
    bool mItemUsePending;
    InteractionHand::EInteractionHand mUsedItemHand;
    bool mHandsBusy;
    bool mAutoJumpEnabled;
    int mAutoJumpTime;
public:
    int mViewport;

private:
    unsigned char mReserved0x90c[4];

public:
    int mButtonsDown;

private:
    unsigned int mButtonsPressed;
    unsigned int mLastSingleFlyButtons[2];
    unsigned int mFlyButtonsDown[2];
    unsigned int mFlyButtons[2];
    int mDestroyTicks;
    int mLastAttackTick;
    int mLastUseTick;
    bool mReserved0x93c;
    int mThirdPersonView;
    int mInWallThirdPersonView;
    bool mDRCTouching;
    float mFlyMotionX;
    float mFlyMotionY;
    float mFlyMotionZ;
    int mRideJumpTicks;
    float mRideJumpScale;

public:
    int mUserIndex;

private:
    bool mPresenceIdleSent;
    SmoothFloat mViewBob;
    SmoothFloat mViewBobPrev;
    SmoothFloat mViewBobAmount;
    float mCreativeMouseStartX;
    float mCreativeMouseStartY;
    float mCreativeMouseStartZ;
    float mCreativeMouseDeltaX;
    float mCreativeMouseDeltaY;
    float mCreativeMouseDeltaZ;
    float mCreativeMouseRepeatDelay;
    int mCreativeRepeatState;
    bool mDamageIndicatorSegments[8];
    bool mDamageIndicatorAllDirections;
    float mDamageTaken;
    bool mHasOpenedInventory;
    bool mWaitingForScoreboard;
    double mSessionTimer;
    std::vector<ModelPart*> mAdditionalModelParts;
    float mWaterVisionClarity;
    float mWaterVisionClarityO;
    float mWaterVisionClarityTarget;
    int mUnderwaterTicks[2];
    unsigned char mAwardedAchievements[0x14];
    double mPercentageTraversed;
    bool mDamageNearbyMobs;
};

#endif
