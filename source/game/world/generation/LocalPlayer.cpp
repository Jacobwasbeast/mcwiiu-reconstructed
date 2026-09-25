#include "game/core/UnityClient6.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/PlayerRideableJumping.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerboundChatPacket.hpp"
#include "game/network/ServerboundMovePlayerPacket.hpp"
#include "game/network/ServerboundPlayerAbilitiesPacket.hpp"
#include "game/network/ServerboundSwingPacket.hpp"
#include "game/network/ServerboundUseItemPacket.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameType.hpp"
#include "game/core/User.hpp"
#include "game/core/Input.hpp"
#include "platform/wiiu/CInput.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Gui.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/ui/Tooltips.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ElytraItem.hpp"
#include "game/item/Items.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/HitResult.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/block/BeaconBlockEntity.hpp"
#include "game/world/block/BrewingStandBlockEntity.hpp"
#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/world/block/DropperBlockEntity.hpp"
#include "game/world/block/FlowerPotBlockEntity.hpp"
#include "game/world/block/FurnaceBlockEntity.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/core/MinecartChest.hpp"
#include "game/core/MinecartHopper.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Boat.hpp"
#include "game/core/ArmorStand.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/core/Painting.hpp"
#include "game/entity/mob/LeashFenceKnotEntity.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerboundPlayerCommandPacket.hpp"
#include "game/network/ServerboundClientCommandPacket.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/ui/AnvilMenu.hpp"
#include "game/ui/BrewingStandMenu.hpp"
#include "game/ui/EnchantmentMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"

#include <cmath>
#include <algorithm>
#include <iostream>

LocalPlayer::LocalPlayer(Minecraft* minecraft, Level* level, User* user, int playerIndex)
    : AbstractClientPlayer(level, user != 0 ? user->name : std::wstring()),
      mPacketListener(0), mPermissionLevel(0), mLastSentX(0.0), mLastSentY(0.0),
      mLastSentZ(0.0), mLastSentYRot(0.0f), mLastSentXRot(0.0f),
      mPositionSendCounter(0), mPreviousOnGround(false), mHealthInitialized(false), mInput(0),
      mMinecraft(minecraft), mSprintDelay(0), mSprintPending(false), mFlyTogglePending(false),
      mIdleTicks(0),
      mSprinting(false), mThirdPersonViewTimer(0),
      mViewYRot(0.0f), mViewXRot(0.0f), mViewYRotO(0.0f), mViewXRotO(0.0f),
      mPortalEffectIntensity(0.0f), mPortalEffectIntensityO(0.0f),
      mFallFlyYaw(0.0f), mFallFlyYawOld(0.0f),
      mFallFlyingAlignment(0.0f), mFallFlyingAlignmentO(0.0f),
      mHandsBusy(false), mItemUsePending(false),
      mUsedItemHand(InteractionHand::eInteractionHand_MainHand),
      mAutoJumpEnabled(true), mAutoJumpTime(0), mViewport(0), mButtonsDown(0),
      mButtonsPressed(0), mMoveInputX(0), mMoveInputY(0), mLookInputX(0), mLookInputY(0),
      mReserved0x928(0), mReserved0x92c(0), mDestroyTicks(0), mLastAttackTick(0),
      mLastUseTick(0), mThirdPersonView(0), mGameMode(0), mDRCTouching(false),
      mRideJumpTicks(0), mRideJumpScale(0.0f), mUserIndex(-1), mPresenceIdleSent(false),
      mViewBob(), mViewBobPrev(), mViewBobAmount(), mCreativeMouseStartX(0.0f),
      mCreativeMouseStartY(0.0f), mCreativeMouseStartZ(0.0f), mCreativeMouseDeltaX(0.0f),
      mCreativeMouseDeltaY(0.0f), mCreativeMouseDeltaZ(0.0f), mCreativeMouseRepeatDelay(0.0f),
      mCreativeRepeatState(0), mDamageTaken(0.0f),
      mHasOpenedInventory(true), mWaitingForScoreboard(false),
      mSessionTimer(0.0), mAdditionalModelParts(), mWaterVisionClarity(0.0f),
      mWaterVisionClarityO(0.0f), mWaterVisionClarityTarget(0.0f), mPercentageTraversed(0.0),
      mDamageNearbyMobs(false)
{
}

LocalPlayer::~LocalPlayer()
{
}

bool LocalPlayer::hasPermission(EGameCommand command)
{
    return mPermissionLevel > 0;
}

BlockPos LocalPlayer::getCommandSenderBlockPosition()
{
    return BlockPos(this);
}

bool LocalPlayer::hurt(const DamageSource* source, float damage)
{
    mDamageTaken += damage; return Player::hurt(source, damage);
}

void LocalPlayer::tick()
{
    AbstractClientPlayer::tick();
    sendPosition();
}

Vec3* LocalPlayer::getViewVector(float partialTick)
{
    return Player::getViewVector(partialTick);
}

bool LocalPlayer::isInWall()
{
    if (mField0x6c0 || mNoPhysics)
    {
        return false;
    }

    if (!mTickCache.mCacheChanged &&
        mInWallCacheX == mX && mInWallCacheY == mY && mInWallCacheZ == mZ &&
        mInWallCacheYRot == mYRot && mInWallCacheXRot == mXRot &&
        mGameMode == mThirdPersonView)
    {
        return mInWallCacheResult;
    }

    mGameMode = mThirdPersonView;
    mInWallCacheYRot = mYRot;
    mInWallCacheXRot = mXRot;
    mInWallCacheX = mX;
    mInWallCacheY = mY;
    mInWallCacheZ = mZ;

    double eyeHeight = getEyeHeight();

    for (int i = 0; i < 8; ++i)
    {
        double y = mY + eyeHeight + (double)(((i & 1) != 0 ? 0.5f : -0.5f) * 0.1f);
        double x = mX + (double)(((i & 2) != 0 ? 0.5f : -0.5f) * mBbWidth * 0.8f);
        double z = mZ + (double)(((i & 4) != 0 ? 0.5f : -0.5f) * mBbWidth * 0.8f);

        BlockPos pos(Mth::floor(x), Mth::floor(y), Mth::floor(z));
        const BlockState* state = mLevel->getBlockState(mTickCache, pos);
        if (state->isSolid())
        {
            if (!Minecraft::InMiniGame(eMiniGameId_Any, true) ||
                g_pApp->ShouldCollideWith(state->getBlock()))
            {
                mInWallCacheResult = true;
                return true;
            }
        }
    }

    mInWallCacheResult = false;
    return false;
}

void LocalPlayer::rideTick()
{
    Player::rideTick();
}

bool LocalPlayer::startRiding(boost::shared_ptr<Entity> vehicle, bool force)
{
    return Entity::startRiding(vehicle);
}

void LocalPlayer::stopRiding()
{
    Entity::stopRiding();
}

void LocalPlayer::handleEntityEvent(unsigned char eventId, int data)
{
    LivingEntity::handleEntityEvent(eventId, data);
}

bool LocalPlayer::isSneaking()
{
    return Entity::isSneaking();
}

bool LocalPlayer::isSprinting()
{
    return mSprinting != 0;
}

void LocalPlayer::setSprinting(bool sprinting)
{
    LivingEntity::setSprinting(sprinting);
    mSprinting = sprinting ? 600 : 0;
}

boost::shared_ptr<Entity> LocalPlayer::changeDimension(int dimension)
{
    mGameMode = dimension; return boost::shared_ptr<Entity>();
}

void LocalPlayer::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    Entity::onSyncedDataUpdated(accessor);
}

void LocalPlayer::closeContainer()
{
    Player::closeContainer();
}

boost::shared_ptr<ItemEntity> LocalPlayer::drop(bool dropAll)
{
    return Player::drop(dropAll);
}

not_null_ptr<ItemInstance> LocalPlayer::reallyDrop(boost::shared_ptr<ItemEntity> item)
{
    return ItemInstance::EMPTY;
}

void LocalPlayer::openTextEdit(boost::shared_ptr<SignBlockEntity> sign)
{
    if (g_pApp->LoadSignEntryMenu(mUserIndex, sign) != 0)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
    }
}

void LocalPlayer::openCommandBlock(boost::shared_ptr<CommandBlockEntity> block)
{
}

int LocalPlayer::openTrading(boost::shared_ptr<Merchant> merchant, const std::wstring& name, int level)
{
    const int result = static_cast<int>(g_pApp->LoadTradingMenu(mPlayerIndex, mInventory, merchant, mLevel, name));
    if (result != 0)
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
    return result;
}

int LocalPlayer::openContainer(boost::shared_ptr<Container> container)
{
    if (!container || g_pApp == NULL)
        return 0;
    const int player = mPlayerIndex;
    if (boost::dynamic_pointer_cast<MinecartChest>(container))
        return (int)g_pApp->LoadContainerMenu(player, mInventory, container);
    if (boost::dynamic_pointer_cast<MinecartHopper>(container))
        return (int)g_pApp->LoadHopperMenu(player, mInventory, container);
    if (boost::dynamic_pointer_cast<FurnaceBlockEntity>(container))
        return (int)g_pApp->LoadFurnaceMenu(player, mInventory, container);
    if (boost::dynamic_pointer_cast<BrewingStandBlockEntity>(container))
        return (int)g_pApp->LoadBrewingStandMenu(player, mInventory, container);
    if (boost::dynamic_pointer_cast<BeaconBlockEntity>(container))
        return (int)g_pApp->LoadBeaconMenu(player, mInventory, container);
    if (boost::dynamic_pointer_cast<DispenserBlockEntity>(container) ||
        boost::dynamic_pointer_cast<DropperBlockEntity>(container))
        return (int)g_pApp->LoadTrapMenu(player, container, container);
    if (boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(container))
        return (int)g_pApp->LoadShulkerBoxMenu(player, mInventory, container);
    return (int)g_pApp->LoadContainerMenu(player, container, container);
}

int LocalPlayer::openHorseInventory(boost::shared_ptr<AbstractHorse> horse, boost::shared_ptr<Container> container)
{
    const int result = (int)g_pApp->LoadHorseMenu(mPlayerIndex, mInventory, container, horse);
    if (result != 0)
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
    return result;
}

int LocalPlayer::startBlockInteraction(InteractionObject& interaction)
{
    int result;
    if (interaction.getTypeId(false) == 1)
    {
        boost::shared_ptr<LocalPlayer> self = boost::dynamic_pointer_cast<LocalPlayer>(shared_from_this());
        result = (int)g_pApp->LoadCrafting3x3Menu(mPlayerIndex, self, (int)mX, (int)mY, (int)mZ);
    }
    else if (interaction.getTypeId(false) == 4)
    {
        result = (int)g_pApp->LoadEnchantingMenu(mPlayerIndex, mInventory, (int)mX, (int)mY, (int)mZ, mLevel, interaction.getDisplayName());
    }
    else if (interaction.getTypeId(false) == 0x13)
    {
        result = (int)g_pApp->LoadRepairingMenu(mPlayerIndex, mInventory, mLevel, (int)mX, (int)mY, (int)mZ);
    }
    else
    {
        return 0;
    }
    if (result != 0)
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
    return result;
}

void LocalPlayer::openItemInstanceGui(not_null_ptr<ItemInstance> item, InteractionHand::EInteractionHand hand)
{
    Player::openItemInstanceGui(item, hand);
}

void LocalPlayer::crit(boost::shared_ptr<Entity> entity)
{
    Player::crit(entity);
}

void LocalPlayer::magicCrit(boost::shared_ptr<Entity> entity)
{
    Player::magicCrit(entity);
}

void LocalPlayer::serverAiStep()
{
    Player::serverAiStep();
    if (isControlledCamera())
    {
        const float strafe = mInput->mMoveStrafe;
        const float forward = mInput->mMoveForward;
        const float movement = std::sqrt(strafe * strafe + forward * forward);
        if (mAbilities.mFlying && mAbilities.mMayfly && movement >= 0.1f)
        {
            const float angle = std::atan2(forward, strafe) * 57.2957763671875f;
            const float wrapped = angle + (std::floor((angle + mYRot) / 360.0f + 0.5f)
                                           * 360.0f - (angle + mYRot));
            const float radians = wrapped * 0.01745329238474369f;
            mXxa = std::cos(radians) * movement;
            mZza = std::sin(radians) * movement;
        }
        else
        {
            mXxa = strafe;
            mZza = forward;
        }
        mJumping = mInput->mJumping;
        mViewYRotO = mViewYRot;
        mViewXRotO = mViewXRot;
        mViewYRot += (mYRot - mViewYRot) * 0.5f;
        mViewXRot += (mXRot - mViewXRot) * 0.5f;
    }
}

float LocalPlayer::tickHeadTurn(float yRot, float xRot)
{
    return Player::tickHeadTurn(yRot, xRot);
}

void LocalPlayer::aiStep()
{
    if (isSprinting() && --mSprinting < 1)
        mSprinting = 1;

    if (mSprintDelay > 0)
        --mSprintDelay;

    mPortalEffectIntensityO = mPortalEffectIntensity;
    if (!mIsInsidePortal)
    {
        bool confusionIsLong = false;
        if (hasEffect(MobEffects::CONFUSION))
        {
            MobEffectInstance* confusion = getEffect(MobEffects::CONFUSION);
            confusionIsLong = confusion->getDuration() > 60;
        }
        if (confusionIsLong)
            mPortalEffectIntensity = std::min(mPortalEffectIntensity + 0.0066666668f, 1.0f);
        else
            mPortalEffectIntensity = std::max(mPortalEffectIntensity - 0.05f, 0.0f);
    }
    else
    {
        if (!mLevel->mIsClientSide && isPassenger())
            stopRiding();
        if (mMinecraft->mScreen != NULL)
            mMinecraft->setScreen(NULL);
        if (mPortalEffectIntensity == 0.0f)
            mMinecraft->mSoundEngine->playUI(SoundEvents::PORTAL_TRIGGER, 1.0f,
                                             mRandom->nextFloat() * 0.4f + 0.8f);
        mPortalEffectIntensity = std::min(mPortalEffectIntensity + 0.0125f, 1.0f);
        mIsInsidePortal = false;
    }

    if (mPortalTime > 0)
        --mPortalTime;

    const bool wasJumping = mInput->mJumping;
    const bool wasSneaking = mInput->mSneaking;
    const float previousForward = mInput->mMoveForward;
    mInput->tick(this);

    if (mInput->mMoveStrafe > -0.8f && mInput->mMoveStrafe < 0.8f &&
        mInput->mMoveForward > -0.8f && mInput->mMoveForward < 0.8f)
    {
        mThirdPersonViewTimer = 0;
    }
    else
    {
        ++mThirdPersonViewTimer;
    }

    if (isUsingItem() && !isPassenger())
    {
        mSprintDelay = 0;
        mInput->mMoveStrafe *= 0.2f;
        mInput->mMoveForward *= 0.2f;
    }

    const int autoJumpTime = mAutoJumpTime;
    if (mAutoJumpTime > 0)
    {
        --mAutoJumpTime;
        mInput->mJumping = true;
    }

    const double edgeOffset = static_cast<double>(mBbWidth) * 0.35;
    const double edgeY = mBoundingBox->mMinY + 1.75;
    checkInBlock(mX - edgeOffset, edgeY, mZ + edgeOffset);
    checkInBlock(mX - edgeOffset, edgeY, mZ - edgeOffset);
    checkInBlock(mX + edgeOffset, edgeY, mZ - edgeOffset);
    checkInBlock(mX + edgeOffset, edgeY, mZ + edgeOffset);

    const bool canFly = mAbilities.mMayfly || isAllowedToFly();
    const bool foodAllowsSprinting = mFoodData.getFoodLevel() > FoodConstants::SprintLevel() || canFly;
    bool canStartSprinting = foodAllowsSprinting && !isUsingItem() &&
        !hasEffect(MobEffects::BLINDNESS) && !isPassenger();

    if ((!mOnGround && !isInWater()) || isSprinting())
        canStartSprinting = false;

    bool forwardSprintInput = false;
    if (canStartSprinting)
    {
        forwardSprintInput = g_pInput->ButtonDown(mUserIndex, 0x2e);
        const float forward = mInput->mMoveForward;
        if (!forwardSprintInput || forward < 0.8f)
        {
            if (wasSneaking || previousForward >= 0.8f || forward < 0.8f)
            {
                if (mSprintDelay > 0 && forward == 0.0f)
                    mSprintPending = true;
            }
            else if (mSprintDelay == 0)
            {
                mSprintDelay = 9;
                mSprintPending = false;
            }
            else if (mSprintPending)
            {
                setSprinting(true);
                mSprintDelay = 0;
                mSprintPending = false;
            }
        }
        else
        {
            setSprinting(true);
            mSprintDelay = 0;
            mSprintPending = false;
        }
    }

    bool stopForDirection = true;
    if (mInput->mMoveForward >= 0.8f &&
        std::fabs(mInput->mMoveStrafe) <= 0.8f && foodAllowsSprinting)
        stopForDirection = false;

    bool keepSprintForInput = mInput->mMoveForward >= 0.8f && foodAllowsSprinting;
    if (isSprinting() && mOnGround && stopForDirection)
        setSprinting(false);

    Vec3* position = getPos(0.0f);
    const BlockPos currentBlock(position);
    const BlockPos aboveBlock(position->x, position->y + 1.0, position->z);
    const BlockPos belowBlock(position->x, position->y - 1.1, position->z);
    Material* currentMaterial = mLevel->getBlockState(currentBlock)->getMaterial();
    Material* aboveMaterial = mLevel->getBlockState(aboveBlock)->getMaterial();
    Material* belowMaterial = mLevel->getBlockState(belowBlock)->getMaterial();
    Vec3* viewVector = getViewVector(0.0f);
    const float viewAngle = std::acos(static_cast<float>(viewVector->x * viewVector->x +
                                                          viewVector->z * viewVector->z)) *
                            57.29578f;

    if (isInWater() && !mAbilities.mFlying && !isSwimming() &&
        ((belowMaterial != Material::AIR && aboveMaterial != Material::AIR) ||
         viewVector->y < -0.15) &&
        mLevel->mIsClientSide && isSprinting() && !stopForDirection && !isPassenger())
    {
        startSwimming();
    }

    const bool stopSwimmingForBlocks = !keepSprintForInput &&
        !aboveMaterial->blocksMotion() && !currentMaterial->blocksMotion();
    const bool stopSwimmingForView = belowMaterial == Material::AIR && viewAngle > 45.0f;
    if ((!isInWater() || stopSwimmingForBlocks || stopSwimmingForView) &&
        mLevel->mIsClientSide && isSwimming())
    {
        stopSwimming();
    }

    if (isSprinting() && (stopForDirection || !foodAllowsSprinting) && !isSwimming())
    {
        setSprinting(false);
    }

    if (!isSleeping() && (mAbilities.mMayfly || isAllowedToFly()))
    {
        if (!mInput->mJumping)
        {
            if (mField0x61c > 0 && mFlyTogglePending && !isSpectator())
            {
                mAbilities.mFlying = !mAbilities.mFlying;
                onUpdateAbilities();
                mField0x61c = 0;
                mFlyTogglePending = false;
                if (mAbilities.mFlying)
                    mInput->mSneaking = false;
            }
        }
        else if (!wasJumping && autoJumpTime <= 0)
        {
            if (mField0x61c == 0)
            {
                mFlyTogglePending = false;
                mField0x61c = 10;
            }
            else
            {
                mFlyTogglePending = true;
            }
        }
    }
    else if (mAbilities.mFlying)
    {
        mAbilities.mFlying = false;
        onUpdateAbilities();
    }

    bool autoElytraRequested = true;
    if (!mInput->mJumping)
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        autoElytraRequested = miniGame->HasFeatureAutoElytraFly() && mFallDistance > 1.25f;
    }
    if (!isSleeping() && autoElytraRequested && !wasJumping && !mOnGround &&
        mYd < 0.0 &&
        !isFallFlying() && !mAbilities.mFlying)
    {
        not_null_ptr<ItemInstance> chestItem = getItemSlot(EquipmentSlot::CHEST);
        if (chestItem->getItem() == Items::elytra && ElytraItem::isFlyEnabled(chestItem))
        {
            boost::shared_ptr<Entity> self = shared_from_this();
            boost::shared_ptr<Packet> packet(new ServerboundPlayerCommandPacket(
                self, ServerboundPlayerCommandPacket::ACTION_8));
            mPacketListener->send(packet);
        }
    }

    const bool inWaterForInput = isInWater();
    const bool waterControlHeld = (mButtonsDown & 0x80) != 0;
    if (inWaterForInput && waterControlHeld && mInput->mSwimming && !mOnGround)
    {
        goDownInWater();
    }
    else if (inWaterForInput && !mInput->mSwimming && !mOnGround)
    {
        mInput->mSwimming = false;
        mInput->mDown = false;
    }

    if (mAbilities.mFlying && isControlledCamera())
    {
        if ((mButtonsDown & 0x80) != 0)
            mYd -= 0.15;
        if (mInput->mJumping)
        {
            mYd += 0.15;
            mNoJumpDelay = 0;
        }

        const float yawQuadrant = std::floor(mYRot / 90.0f + 0.5f);
        if ((mReserved0x928 & 0x800) != 0)
        {
            const float radians = yawQuadrant * 90.0f * 3.1415927410125732f / 180.0f;
            mXd = -0.15 * std::cos(radians);
            mZd = -0.15 * std::sin(radians);
        }
        else if ((mReserved0x928 & 0x400) != 0)
        {
            const float radians = yawQuadrant * 90.0f * 3.1415927410125732f / 180.0f;
            mXd = 0.15 * std::cos(radians);
            mZd = 0.15 * std::sin(radians);
        }
    }

    if (!isFallFlying() || !isCreative() || !mInput->mJumping)
    {
        mGlideDistance = 0.0;
    }
    else
    {
        mGlideDistance = 0.5699999928474426;
        mNoJumpDelay = 0;
    }

    if (isRidingJumpable())
    {
        boost::shared_ptr<Entity> vehicle = getVehicle();
        boost::shared_ptr<PlayerRideableJumping> rideable =
            boost::dynamic_pointer_cast<PlayerRideableJumping>(vehicle);

        if (mRideJumpTicks < 0 && ++mRideJumpTicks == 0)
            mRideJumpScale = 0.0f;

        if (wasJumping)
        {
            if (!mInput->mJumping)
            {
                mRideJumpTicks = -10;
                rideable->onPlayerJump(static_cast<int>(std::floor(getJumpRidingScale() * 100.0)));
                sendRidingJump();
            }
            else
            {
                ++mRideJumpTicks;
                if (mRideJumpTicks > 9)
                    mRideJumpScale = 0.2f / static_cast<float>(mRideJumpTicks - 8) + 0.8f;
                else
                    mRideJumpScale = static_cast<float>(mRideJumpTicks) * 0.1f;
            }
        }
        else if (mInput->mJumping)
        {
            mRideJumpTicks = 0;
            mRideJumpScale = 0.0f;
        }
    }
    else
    {
        mRideJumpScale = 0.0f;
    }

    Player::aiStep();

    if (mAbilities.mMayfly || isAllowedToFly() || !mOnGround)
    {
        if (mAbilities.mFlying)
        {
            Vec3* look = getViewVector(0.0f);
            if (!isSprinting() && !isSpectator())
            {
                mXd = 0.0;
                mZd = 0.0;
                double verticalSpeed = 0.0;
                if ((mReserved0x928 & 0x1000) != 0)
                    verticalSpeed = static_cast<double>(0.1f);
                if ((mReserved0x928 & 0x2000) != 0)
                    verticalSpeed = static_cast<double>(-0.1f);
                mYd = verticalSpeed;
                move(eMoverType_Self, 0.0, verticalSpeed, 0.0, false);
                mYd = 0.0;
            }
            else
            {
                double speed = static_cast<double>(600 - mSprinting) / 10.0;
                if (!isSpectator())
                {
                    speed *= speed;
                    if (speed > 1.0)
                        speed = 1.0;
                }
                else if (speed > 0.3)
                {
                    speed = 0.3;
                }
                const float forward = mInput->mMoveForward;
                mXd = look->x * forward * speed;
                mYd = look->y * forward * speed;
                mZd = look->z * forward * speed;
                move(eMoverType_Self, mXd, mYd, mZd, false);
                mYd = 0.0;
            }

            mFallDistance = 0.0f;
            mYo = mY;
            mOnGround = true;
        }
    }
    else if (mAbilities.mFlying)
    {
        mAbilities.mFlying = false;
    }

    if (!mPresenceIdleSent)
    {
        if (g_pInput->GetIdleSeconds(mUserIndex) > 300.0)
        {
            g_pApp->SetRichPresenceContext(mUserIndex, 0);
            mPresenceIdleSent = true;
        }
    }
    else if (g_pInput->GetIdleSeconds(mUserIndex) < 300.0)
    {
        const int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
        const bool localGame = CGameNetworkManager::sInstance.IsLocalGame();
        if (playerCount < 2)
            g_pApp->SetRichPresenceContext(mUserIndex, localGame ? 5 : 4);
        else
            g_pApp->SetRichPresenceContext(mUserIndex, localGame ? 3 : 2);
        updateRichPresence();
        mPresenceIdleSent = false;
    }

    mFallFlyYawOld = mFallFlyYaw;
    mFallFlyingAlignmentO = mFallFlyingAlignment;
    if (isFallFlying())
    {
        Vec3* look = getViewVector(0.0f);
        const double horizontalMotion = mXd * mXd + mZd * mZd;
        const double horizontalLook = look->x * look->x + look->z * look->z;
        if (horizontalMotion > 0.01 && horizontalLook > 0.01)
        {
            const double motionLength = Mth::sqrt(horizontalMotion);
            const double lookLength = Mth::sqrt(horizontalLook);
            const double lengthProduct = static_cast<double>(
                static_cast<float>(motionLength * lookLength));
            const double dot = (mXd * look->x + mZd * look->z) / lengthProduct;
            const double aligned = Mth::clamp(dot, 0.0, 1.0);
            const double cosine = Mth::clamp(dot, -1.0, 1.0);
            const double cross = mXd * look->z - mZd * look->x;
            const double sign = (0.0 < cross) - (cross < 0.0);
            mFallFlyingAlignment = static_cast<float>(aligned);
            const float signedAngle = static_cast<float>(sign * std::acos(cosine));
            mFallFlyYaw = signedAngle * 180.0f / 3.141592741f;
        }
        else
        {
            mFallFlyYaw = 0.0f;
        }
    }
    else
    {
        mFallFlyYaw = 0.0f;
    }
}

MobEffectInstance* LocalPlayer::removeEffectNoUpdate(MobEffect* effect)
{
    if (effect == MobEffects::CONFUSION)
    {
        mPortalEffectIntensity = 0.0f;
        mPortalEffectIntensityO = 0.0f;
    }
    return LivingEntity::removeEffectNoUpdate(effect);
}

void LocalPlayer::move(MoverType type, double xa, double ya, double za, bool collide)
{
    Entity::move(type, xa, ya, za, collide);
}

int LocalPlayer::openFireworks(int slot, int x, int y)
{
    boost::shared_ptr<LocalPlayer> self = boost::static_pointer_cast<LocalPlayer>(shared_from_this());
    const int result = static_cast<int>(g_pApp->LoadFireworksMenu(mPlayerIndex, self, slot, x, y));
    if (result != 0)
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
    return result;
}

void LocalPlayer::displayClientMessage(int message, bool force)
{
    mWaitingForScoreboard = force;
}

void LocalPlayer::awardStat(Stat* stat, arrayWithLength<unsigned char> params)
{
    Player::awardStat(stat, params);
}

unsigned int LocalPlayer::GetThirdPersonView(bool firstPerson)
{
    if (IsSpectatingOtherPlayer() && !firstPerson) {
        return 1;
    }
    if (mDRCTouching && isAlive()) {
        return 2;
    }
    return static_cast<unsigned int>(mThirdPersonView);
}

void LocalPlayer::SetThirdPersonView(int view)
{
    mThirdPersonView = view;
}

unsigned int LocalPlayer::CanChangeThirdPersonView()
{
    if (mThirdPersonViewTimer < 7 && mThirdPersonViewTimer != 0)
    {
        return 0;
    }
    return 1;
}

void LocalPlayer::onCrafted(not_null_ptr<ItemInstance> item)
{
    Player::onCrafted(item);
}

void LocalPlayer::handleCollectItem(not_null_ptr<ItemInstance> item)
{
    Player::handleCollectItem(item);
}

void LocalPlayer::SpectatePlayerNext()
{
    Player::SpectatePlayerNext();
}

void LocalPlayer::SpectatePlayerPrev()
{
    Player::SpectatePlayerPrev();
}

void LocalPlayer::StopSpectatingPlayer()
{
    Player::StopSpectatingPlayer();
}

const GameType* LocalPlayer::GetGameType()
{
    MultiPlayerGameMode* gameMode = GetLocalGameMode();
    return gameMode == 0 ? 0 : gameMode->GetGameType();
}

void LocalPlayer::SetGameType(const GameType* gameType)
{
    mGameMode = gameType == 0 ? 0 : gameType->getId();
    Tooltips::ResetAllTooltips(mUserIndex);
}

unsigned long long LocalPlayer::GetType()
{
    return 0x000000000C800003ULL;
}

bool LocalPlayer::isHandsBusy()
{
    return mHandsBusy;
}

unsigned long long LocalPlayer::SetSessionTimerStart()
{
    mSessionTimer = 0.0; return (unsigned long long)mSessionTimer;
}

bool LocalPlayer::IsWaitingForScoreboard()
{
    return mWaitingForScoreboard;
}

bool LocalPlayer::doesLevelAllowBlockToBreak(const BlockPos& pos)
{
    MultiPlayerGameMode* gameMode = mMinecraft->mGameModes[mUserIndex];
    if (gameMode == NULL)
        return false;
    Level* level = gameMode->GetLevel();
    if (level == NULL)
        return false;
    Block* block = level->getBlockState(pos)->getBlock();
    return (block == Blocks::bedrock || pos.y > 0) &&
           (block != Blocks::AIR || pos.y < 127);
}

unsigned int LocalPlayer::pickBlock(bool select)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mHitResult == NULL)
        return 0;

    HitResult* hitResult = minecraft->mHitResult;
    not_null_ptr<ItemInstance> pickedItem(new ItemInstance());
    boost::shared_ptr<BlockEntity> blockEntity;

    if (hitResult->mType == HitResult::BLOCK)
    {
        const BlockPos pos = hitResult->getBlockPos();
        const BlockState* state = mLevel->getBlockState(pos);
        if (state->getMaterial() == Material::AIR)
            return 0;

        Block* block = state->getBlock();
        pickedItem = block->getCloneItemInstance(mLevel, pos, state);
        blockEntity = mLevel->getBlockEntity(pos);
    }
    else if (hitResult->mType == HitResult::ENTITY)
    {
        const unsigned long long entityType = hitResult->mEntity->GetType();
        if (entityType == 0x000000000A080001ULL)
        {
            pickedItem.reset(new ItemInstance(Items::painting));
        }
        else if (entityType == 0x000000000A080003ULL)
        {
            pickedItem.reset(new ItemInstance(Items::lead));
        }
        else if (entityType == 0x000000000A080002ULL)
        {
            boost::shared_ptr<ItemFrame> itemFrame =
                boost::static_pointer_cast<ItemFrame>(hitResult->mEntity);
            not_null_ptr<ItemInstance> frameItem = itemFrame->getItem();
            if (!frameItem->isEmpty())
                pickedItem = frameItem->copy();
        }
        else if ((entityType & 0x000000000A800000ULL) == 0x000000000A800000ULL)
        {
            boost::shared_ptr<Minecart> minecart =
                boost::static_pointer_cast<Minecart>(hitResult->mEntity);
            const int type = minecart->getType()->mId;
            Item* item = Items::minecart;
            if (type == 1)
                item = Items::chestMinecart;
            else if (type == 2)
                item = Items::furnaceMinecart;
            else if (type == 3)
                item = Items::tntMinecart;
            else if (type == 5)
                item = Items::hopperMinecart;
            else if (type == 6)
                item = Items::commandBlockMinecart;
            pickedItem.reset(new ItemInstance(item));
        }
        else if (eTYPE_DERIVED_FROM__FULT1(0, static_cast<unsigned int>(ENTITY_TYPE_BOAT),
                                            static_cast<unsigned int>(entityType >> 32),
                                            static_cast<unsigned int>(entityType)))
        {
            boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(hitResult->mEntity);
            pickedItem.reset(new ItemInstance(boat->getDropItem()));
        }
        else if (eTYPE_DERIVED_FROM__FULT1(0, static_cast<unsigned int>(ENTITY_TYPE_ARMOR_STAND),
                                            static_cast<unsigned int>(entityType >> 32),
                                            static_cast<unsigned int>(entityType)))
        {
            pickedItem.reset(new ItemInstance(Items::armorStand));
        }
        else if (eTYPE_DERIVED_FROM__FULT1(0, 0x0800000EU,
                                            static_cast<unsigned int>(entityType >> 32),
                                            static_cast<unsigned int>(entityType)))
        {
            pickedItem.reset(new ItemInstance(Items::endCrystal));
        }
        else
        {
            const ResourceLocation key = EntityIO::getKey(hitResult->mEntity);
            if (EntityIO::sSpawnInfo.find(key) == EntityIO::sSpawnInfo.end())
                return 0;
            pickedItem.reset(new ItemInstance(Items::spawnEgg));
            SpawnEggItem::addSpawnEntity(pickedItem, key);
        }
    }
    else
    {
        return 0;
    }

    if (blockEntity && boost::dynamic_pointer_cast<FlowerPotBlockEntity>(blockEntity))
        blockEntity.reset();
    if (blockEntity)
        pickedItem = Minecraft::addCustomNbtData(pickedItem, blockEntity);

    if (pickedItem->isEmpty())
        return 0;

    Inventory* inventory = mInventory.get();
    if (!select)
    {
        inventory->setPickedItem(pickedItem);
        minecraft->mGameMode->handlePickItem(pickedItem);
        return 1;
    }

    if (!mAbilities.mInstabuild)
        return inventory->findSlotMatchingItem(pickedItem, false) != -1;
    return 1;
}

unsigned char LocalPlayer::handleMouseClick(int button)
{
    MultiPlayerGameMode* gameMode = mMinecraft->mGameModes[mUserIndex];
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(shared_from_this());
    HitResult* hitResult = mMinecraft->mHitResult;
    const InteractionHand::EInteractionHand hand = InteractionHand::eInteractionHand_MainHand;

    if (isUsingItem())
        return 0;

    if (button == 0)
    {
        if (mLastAttackTick > 0)
            return 0;

        if (hitResult == NULL)
        {
            if (gameMode->hasMissTime())
                mLastAttackTick = 10;
            resetAttackStrengthTicker();
            if (swing(hand))
                playSound(SoundEvents::PLAYER_ATTACK_WEAK, 0.6f, 1.0f);
            return 0;
        }

        if (hitResult->mType == HitResult::BLOCK)
        {
            const BlockPos pos = hitResult->getBlockPos();
            if (!doesLevelAllowBlockToBreak(pos))
            {
                if (gameMode->hasMissTime())
                    mLastAttackTick = 10;
                resetAttackStrengthTicker();
            }
            else
            {
                gameMode->startDestroyBlock(pos, hitResult->mDirection);
            }
            swing(hand);
        }
        else if (hitResult->mType == HitResult::ENTITY)
        {
            gameMode->attack(player, hitResult->mEntity);
            swing(hand);
        }
        return 0;
    }

    if (button == 1)
    {
        mIdleTicks = 4;
        if (isHandsBusy())
            return 0;

        if (isSleeping() && mLevel != NULL && mLevel->mIsClientSide)
        {
            if (mCreativeRepeatState == 4)
                return 0;
            MultiplayerLocalPlayer* multiplayerPlayer =
                dynamic_cast<MultiplayerLocalPlayer*>(this);
            if (multiplayerPlayer != NULL && mPacketListener != NULL)
                multiplayerPlayer->StopSleeping();
            return 0;
        }

        if (Minecraft::InMiniGame(eMiniGameId_Glide, false))
        {
            const MiniGameDef* miniGame = mMinecraft->GetMiniGame();
            if (miniGame->ArePowerupsActive())
            {
                not_null_ptr<ItemInstance> item = getItemInHand(hand);
                if (!item->isEmpty())
                {
                    boost::shared_ptr<MultiplayerLocalPlayer> multiplayerPlayer =
                        boost::dynamic_pointer_cast<MultiplayerLocalPlayer>(player);
                    if (multiplayerPlayer)
                    {
                        PowerupItems::UsePowerupItem(player, Item::getId(item->getItem()));
                        multiplayerPlayer->mPacketListener->send(
                            boost::shared_ptr<Packet>(new ServerboundUseItemPacket(hand)));
                        item->shrink(1);
                        if (item->getCount() == 0)
                            setItemInHand(hand, ItemInstance::EMPTY);
                        return 1;
                    }
                }
            }
            return 0;
        }

        if (hitResult != NULL && hitResult->mType == HitResult::BLOCK)
        {
            const BlockPos pos = hitResult->getBlockPos();
            if (mLevel->getBlockState(pos)->getMaterial() != Material::AIR)
            {
                not_null_ptr<ItemInstance> item = getItemInHand(hand);
                const int oldCount = item->isEmpty() ? 0 : item->getCount();
                bool used = false;
                const InteractionResult::EInteractionResult result = gameMode->useItemOn(
                    player, mLevel, pos, hitResult->mDirection, hitResult->mLocation,
                    hand, false, &used);
                if (result == InteractionResult::eInteractionResult_Success)
                {
                    swing(hand);
                    if (!item->isEmpty())
                    {
                        const int newCount = item->getCount();
                        if (newCount == 0)
                            setItemInHand(hand, ItemInstance::EMPTY);
                        else if (newCount != oldCount || gameMode->hasInfiniteItems())
                            mMinecraft->mLocalItemInHandRenderers[mUserIndex]->itemUsed(hand);
                    }
                    return used;
                }
            }
        }

        if (hitResult != NULL && hitResult->mType == HitResult::ENTITY)
        {
            not_null_ptr<ItemInstance> item = getItemInHand(hand);
            if (gameMode->interact(player, hitResult->mEntity, item, hand) ==
                InteractionResult::eInteractionResult_Success)
                return 0;
            if (gameMode->interactAt(player, hitResult->mEntity, hitResult, item, hand) ==
                InteractionResult::eInteractionResult_Success)
                return 0;
        }

        not_null_ptr<ItemInstance> item = getItemInHand(hand);
        if (!item->isEmpty())
        {
            const InteractionResult::EInteractionResult result =
                gameMode->useItem(player, mLevel, hand, false);
            if (result == InteractionResult::eInteractionResult_Success)
            {
                mMinecraft->mLocalItemInHandRenderers[mUserIndex]->itemUsed(hand);
                return 0;
            }
        }

        return 0;
    }

    if (button == 2 && !Minecraft::InMiniGame(eMiniGameId_Any, true))
        pickBlock(false);
    return 0;
}

unsigned int LocalPlayer::creativeModeHandleMouseClick(int button, bool pressed)
{
    if (!pressed)
    {
        mCreativeRepeatState = 0;
        return 0;
    }

    if (mCreativeRepeatState == 4 || mCreativeRepeatState == 5)
        return 0;

    if (mCreativeRepeatState == 0)
    {
        mCreativeMouseStartX = static_cast<float>(mX);
        mCreativeMouseStartY = static_cast<float>(mY);
        mCreativeMouseStartZ = static_cast<float>(mZ);

        const unsigned char handled = handleMouseClick(button);
        if (isSprinting() || isPassenger() || isSleeping())
        {
            mCreativeRepeatState = 5;
            return 1;
        }

        if (handled == 0)
        {
            mCreativeRepeatState = 4;
            return 1;
        }

        mCreativeMouseRepeatDelay = 0.0f;
        mCreativeRepeatState = 1;
        return 1;
    }

    if (isSprinting())
    {
        mCreativeRepeatState = 5;
        return 0;
    }

    const float deltaX = static_cast<float>(mX) - mCreativeMouseStartX;
    const float deltaY = static_cast<float>(mY) - mCreativeMouseStartY;
    const float deltaZ = static_cast<float>(mZ) - mCreativeMouseStartZ;
    const float changeX = deltaX - mCreativeMouseDeltaX;
    const float changeY = deltaY - mCreativeMouseDeltaY;
    const float changeZ = deltaZ - mCreativeMouseDeltaZ;
    const float changeLength = std::sqrt(changeX * changeX + changeY * changeY + changeZ * changeZ);

    if (mCreativeRepeatState == 2)
    {
        if (changeLength < 0.01f)
        {
            mCreativeMouseRepeatDelay = 0.0f;
            mCreativeRepeatState = 3;
        }
    }
    else if (mCreativeRepeatState == 3)
    {
        if (changeLength >= 0.01f)
        {
            mCreativeMouseRepeatDelay = 0.0f;
            mCreativeRepeatState = 2;
        }
        else
        {
            mCreativeMouseRepeatDelay += 0.1f;
            if (mCreativeMouseRepeatDelay > 0.7f)
            {
                mCreativeMouseRepeatDelay = 0.0f;
                mCreativeRepeatState = 1;
            }
        }
    }

    mCreativeMouseDeltaX = deltaX;
    mCreativeMouseDeltaY = deltaY;
    mCreativeMouseDeltaZ = deltaZ;

    const float movement[3] = {deltaX, deltaY, deltaZ};
    int axis = -1;
    bool delayedRepeat = false;

    for (int candidate = 0; candidate < 3; ++candidate)
    {
        if (std::fabs(movement[candidate]) >= 1.0f)
        {
            axis = candidate;
            break;
        }
    }

    if (axis < 0)
    {
        if (mCreativeMouseRepeatDelay <= 0.0f)
            return 0;

        const float remaining = 1.0f - mCreativeMouseRepeatDelay;
        if (std::fabs(deltaX) < remaining && std::fabs(deltaY) < remaining &&
            std::fabs(deltaZ) < remaining)
            return 0;
        delayedRepeat = true;
        for (int candidate = 0; candidate < 3; ++candidate)
        {
            if (std::fabs(movement[candidate]) >= remaining)
            {
                axis = candidate;
                break;
            }
        }
    }

    if (axis < 0)
        return 0;

    const double savedX = mX;
    const double savedY = mY;
    const double savedZ = mZ;
    float rayX = mCreativeMouseStartX + deltaX;
    float rayY = mCreativeMouseStartY + deltaY;
    float rayZ = mCreativeMouseStartZ + deltaZ;
    if (!delayedRepeat)
    {
        const float snapped = movement[axis] < 0.0f ? std::ceil(movement[axis]) :
                                                       std::floor(movement[axis]);
        if (axis == 0)
            rayX = mCreativeMouseStartX + snapped;
        else if (axis == 1)
            rayY = mCreativeMouseStartY + snapped;
        else
            rayZ = mCreativeMouseStartZ + snapped;
    }
    mCreativeMouseStartX = rayX;
    mCreativeMouseStartY = rayY;
    mCreativeMouseStartZ = rayZ;
    mX = rayX;
    mY = rayY;
    mZ = rayZ;
    mMinecraft->mGameRenderer->pick(1.0f);
    mX = savedX;
    mY = savedY;
    mZ = savedZ;
    handleMouseClick(button);
    mCreativeMouseRepeatDelay = 0.0f;

    mCreativeRepeatState = mCreativeRepeatState == 3 ? 1 : 2;
    return 0;
}

unsigned long long LocalPlayer::handleMouseDown(int button, bool pressed)
{
    if (button == 0 && pressed)
    {
        GetLocalGameMode()->OnMouseDown();
    }

    if (isSpectator())
    {
        if (!Minecraft::GetInstance()->GetMiniGame()->HasSpectatorParticipation())
        {
            return 0;
        }
    }

    if (!isSleeping() || mLevel == 0 || !mLevel->mIsClientSide)
    {
        if (!pressed)
        {
            mDestroyTicks = 0;
        }

        if (button != 0 || mDestroyTicks < 1)
        {
            if (pressed)
            {
                HitResult* hitResult = mMinecraft->mHitResult;
                if (hitResult == 0 || hitResult->mType != HitResult::BLOCK || button != 0)
                {
                    mMinecraft->mGameMode->stopDestroyBlock();
                    return 0;
                }

                if (!isUsingItem())
                {
                    BlockPos pos = hitResult->getBlockPos();
                    if (!doesLevelAllowBlockToBreak(pos))
                    {
                        return 0;
                    }
                    if (!mMinecraft->mGameMode->continueDestroyBlock(pos, hitResult->mDirection))
                    {
                        return 0;
                    }
                    mMinecraft->mParticleEngine->crack(pos, hitResult->mDirection);
                    swing(InteractionHand::eInteractionHand_MainHand);
                    return 0;
                }
            }
            mMinecraft->mGameMode->stopDestroyBlock();
        }
    }
    return 0;
}

void LocalPlayer::SetWaitingForScoreboard(bool waiting)
{
    mWaitingForScoreboard = waiting;
}

void LocalPlayer::updateRichPresence()
{
    if (mUserIndex == -1)
        return;

    not_null_ptr<ItemInstance> selected = mInventory->getSelected();
    if (!selected->isEmpty())
    {
        Item* item = selected->getItem();
        if (item != NULL)
        {
            if (Item::getId(item) == 346)
            {
                g_pApp->SetRichPresenceContext(mUserIndex, 4);
                return;
            }
            if (Item::getId(item) == 358)
            {
                g_pApp->SetRichPresenceContext(mUserIndex, 9);
                return;
            }
        }
    }

    if (mContainerMenu != NULL && mContainerMenu != mInventoryMenu)
    {
        if (dynamic_cast<EnchantmentMenu*>(mContainerMenu) != NULL)
        {
            g_pApp->SetRichPresenceContext(mUserIndex, 2);
            return;
        }
        if (dynamic_cast<BrewingStandMenu*>(mContainerMenu) != NULL)
        {
            g_pApp->SetRichPresenceContext(mUserIndex, 3);
            return;
        }
        if (dynamic_cast<AnvilMenu*>(mContainerMenu) != NULL)
        {
            g_pApp->SetRichPresenceContext(mUserIndex, 1);
            return;
        }
    }

    if (mDimension == -1)
    {
        g_pApp->SetRichPresenceContext(mUserIndex, 7);
        return;
    }

    int context = 0;
    if (mMinecraft != NULL && mMinecraft->isTutorial())
        context = 8;
    g_pApp->SetRichPresenceContext(mUserIndex, context);
}

void LocalPlayer::ResetPercentageTraversed()
{
    mPercentageTraversed = 0.0;
}

void LocalPlayer::heal(float amount)
{
    Player::heal(amount);
}

bool LocalPlayer::isInvisible()
{
    return Player::isInvisible();
}

bool LocalPlayer::canMoveOrBeMovedFromTo(double a, double b, double c, double d, double e, double f, Vec3* result)
{
    MultiPlayerGameMode* gameMode = mMinecraft->mGameModes[mUserIndex];
    GameModeConstraints* constraints = gameMode->getGameModeConstraints();
    if (!constraints->canMoveToPosition(a, b, c, d, e, f, result))
        return false;
    if (!g_pApp->DebugDevToolsOn() && mLevel->mIsClientSide)
    {
        if (!mLevel->reallyHasChunk((int)std::floor(d * 0.0625), (int)std::floor(f * 0.0625)))
            return false;
    }
    return true;
}

int LocalPlayer::getIndicatorSegment(float value, float maxValue)
{
    return maxValue == 0.0f ? 0 : (int)(value * 10.0f / maxValue);
}

void LocalPlayer::updateProximityIndicator()
{
    if (gApp.GetFirstMiniGameType() != eMiniGameId_Glide)
        return;

    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options == NULL)
        return;

    LevelRuleset* rules = options->getRequiredGameRules();
    if (rules == NULL)
        return;

    for (int segment = 0; segment < 12; ++segment)
        mReserved0x9ac[segment] = 0;

    if (mBoundingBox == NULL)
        return;

    Vec3* position = getPos(1.0f);
    AABB* probe = mBoundingBox->grow(2.0);
    boost::shared_ptr<Entity> self = boost::static_pointer_cast<Entity>(shared_from_this());
    std::vector<AABB*>* collisions =
        mLevel->getCollisionAABBs(self, probe, true, false, false);
    if (collisions != NULL && !collisions->empty() && position != NULL)
    {
        AABB* closestBox = NULL;
        double closestDistance = 0.0;
        for (size_t index = 0; index < collisions->size(); ++index)
        {
            AABB* box = (*collisions)[index];
            if (box == NULL)
                continue;
            const double distance = position->distanceTo(static_cast<AABB*>(box));
            if (closestBox == NULL || distance <= closestDistance)
            {
                closestBox = box;
                closestDistance = distance;
            }
        }
        if (closestBox != NULL && position != NULL)
        {
            Vec3* center = closestBox->getCenter();
            Vec3* direction = Vec3::newTemp(position->x - center->x, 0.0,
                                            position->z - center->z);
            direction->normalize();
            const int segment = getIndicatorSegment(static_cast<float>(direction->x),
                                                    static_cast<float>(direction->z));
            if (segment >= 0 && segment < 12)
                mReserved0x9ac[segment] = 1;
        }
    }

    if (CGameNetworkManager::sInstance.GetPlayerCount() > 1 && position != NULL)
    {
        std::vector<boost::shared_ptr<Player> >* nearby =
            mLevel->getNearbyPlayers(self, 3.0);
        if (nearby != NULL)
        {
            for (size_t index = 0; index < nearby->size(); ++index)
            {
                boost::shared_ptr<Player> player = (*nearby)[index];
                if (!player)
                    continue;
                Vec3* direction = Vec3::newTemp(position->x - player->mX, 0.0,
                                                position->z - player->mZ);
                direction->normalize();
                const int segment = getIndicatorSegment(static_cast<float>(direction->x),
                                                        static_cast<float>(direction->z));
                if (segment >= 0 && segment < 12)
                    mReserved0x9ac[segment] = 1;
            }
            delete nearby;
        }
    }
}

bool LocalPlayer::isControlledCamera()
{
    return true;
}

unsigned long long LocalPlayer::sendPosition()
{
    if (!isControlledCamera())
        return 0;

    const double dx = mX - mLastSentX;
    const double dy = mY - mLastSentY;
    const double dz = mZ - mLastSentZ;
    const bool positionDue = dx * dx + dy * dy + dz * dz > 0.0009 ||
                             ++mPositionSendCounter >= 20 ||
                             mOnGround != mPreviousOnGround;
    const bool rotationChanged = mYRot != mLastSentYRot || mXRot != mLastSentXRot;

    if (mPacketListener != NULL)
    {
        boost::shared_ptr<Packet> packet;
        if (positionDue && rotationChanged)
            packet.reset(new ServerboundMovePlayerPacket::PosRot(mX, mY, mZ, mYRot, mXRot, mOnGround, false));
        else if (positionDue)
            packet.reset(new ServerboundMovePlayerPacket::Pos(mX, mY, mY, mZ, mOnGround, false));
        else if (rotationChanged)
            packet.reset(new ServerboundMovePlayerPacket::Rot(mYRot, mXRot, mOnGround, false));
        else if (mOnGround != mPreviousOnGround)
            packet.reset(new ServerboundMovePlayerPacket(mOnGround, false));
        if (packet)
            mPacketListener->send(packet);
    }

    if (positionDue)
    {
        mLastSentX = mX;
        mLastSentY = mY;
        mLastSentZ = mZ;
        mPositionSendCounter = 0;
    }
    if (rotationChanged)
    {
        mLastSentYRot = mYRot;
        mLastSentXRot = mXRot;
    }
    mPreviousOnGround = mOnGround;
    return 0;
}

unsigned long long LocalPlayer::chat(const std::wstring& message)
{
    boost::shared_ptr<Packet> packet(new ServerboundChatPacket(message));
    mPacketListener->send(packet);
    return 0;
}

bool LocalPlayer::swing(InteractionHand::EInteractionHand hand)
{
    bool result = LivingEntity::swing(hand);
    mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundSwingPacket(hand)));
    return result;
}

void LocalPlayer::respawn()
{
    if (mPacketListener == NULL)
        return;
    boost::shared_ptr<Packet> packet(new ServerboundClientCommandPacket(
        ServerboundClientCommandPacket::PERFORM_RESPAWN));
    mPacketListener->send(packet);
}

void LocalPlayer::actuallyHurt(const DamageSource* source, float amount)
{
    mDamageTaken += amount; Player::actuallyHurt(source, amount);
}

void LocalPlayer::clientSideCloseContainer()
{
    not_null_ptr<ItemInstance> empty(new ItemInstance());
    mInventory->setCarried(empty);
    Player::closeContainer();
    mMinecraft->setScreen(NULL);
    if (gConsoleUIController.IsContainerMenuDisplayed(mPlayerIndex))
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
        gConsoleUIController.CloseUIScenes(mPlayerIndex, false);
    }
}

void LocalPlayer::hurtTo(float amount, TelemetryDamageSource source)
{
    float health;

    if (!mHealthInitialized)
    {
        setHealth(amount);
        mHealthInitialized = true;
        health = getHealth();
    }
    else
    {
        float oldHealth = getHealth();
        float delta = oldHealth - amount;

        if (delta > 0.0f)
        {
            mLastHurt = delta;
            setHealth(getHealth());
            mInvulnerableTime = mInvulnerableDuration;
            actuallyHurt(DamageSource::GENERIC, delta);
            mHurtDuration = 10;
            mHurtTime = 10;
        }
        else
        {
            setHealth(amount);
            if (delta < 0.0f)
            {
                mInvulnerableTime = mInvulnerableDuration / 2;
            }
        }

        health = getHealth();
    }

    if (health <= 0.0f)
    {
        telemetryManagerInstance->RecordPlayerDiedOrFailed(mUserIndex, (int)mX, (int)mY, (int)mZ, 0,
                                                    source.playerWeaponId, source.causeOfDeathId,
                                                    source.challenges);
        if (gConsoleUIController.GetMenuDisplayed(mUserIndex))
        {
            gConsoleUIController.CloseUIScenes(mUserIndex, false);
        }
    }
}

void LocalPlayer::onUpdateAbilities()
{
    boost::shared_ptr<Packet> packet(new ServerboundPlayerAbilitiesPacket(&mAbilities));
    mPacketListener->send(packet);
}

bool LocalPlayer::isLocalPlayer()
{
    return true;
}

void LocalPlayer::updateDamageIndicator(float amount, float previous, bool critical)
{
    mDamageTaken = amount;
}

double LocalPlayer::getJumpRidingScale()
{
    return mRideJumpScale;
}

unsigned long long LocalPlayer::sendRidingJump()
{
    boost::shared_ptr<Entity> entity = shared_from_this();
    boost::shared_ptr<Packet> packet(new ServerboundPlayerCommandPacket(
        entity, ServerboundPlayerCommandPacket::RIDING_JUMP,
        static_cast<int>(getJumpRidingScale() * 100.0)));
    if (mPacketListener != 0)
    {
        mPacketListener->send(packet);
    }
    return 0;
}

unsigned long long LocalPlayer::sendOpenInventory()
{
    boost::shared_ptr<Entity> entity = shared_from_this();
    boost::shared_ptr<Packet> packet(new ServerboundPlayerCommandPacket(
        entity, ServerboundPlayerCommandPacket::OPEN_INVENTORY));
    mPacketListener->send(packet);
    return 0;
}

unsigned int LocalPlayer::getPermissionLevel()
{
    return (unsigned int)mPermissionLevel;
}

void LocalPlayer::setPermissionLevel(int level)
{
    mPermissionLevel = level;
}

unsigned int LocalPlayer::freeAtAndAbove(const BlockPos& pos)
{
    const BlockState* state = mLevel->getBlockState(pos);
    const BlockState* aboveState = mLevel->getBlockState(pos.above());
    bool freeAtPos = !state->isSolidRender();
    bool freeAbove = !aboveState->isSolidRender();

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        if (!freeAtPos)
            freeAtPos = !g_pApp->ShouldCollideWith(state->getBlock());
        if (!freeAbove)
            freeAbove = !g_pApp->ShouldCollideWith(aboveState->getBlock());
    }

    return freeAtPos && freeAbove;
}

unsigned int LocalPlayer::freeAt(const BlockPos& pos)
{
    const BlockState* state = mLevel->getBlockState(pos);
    if (!state->isSolidRender())
        return true;

    if (!Minecraft::InMiniGame(eMiniGameId_Any, true))
        return false;

    return !g_pApp->ShouldCollideWith(state->getBlock());
}

bool LocalPlayer::checkInBlock(double x, double y, double z)
{
    if (!mNoPhysics)
    {
        BlockPos pos(x, y, z);
        x -= pos.x;
        z -= pos.z;
        unsigned int free;
        if (std::fabs(mBoundingBox->mMinY - mBoundingBox->mMaxY) <= 1.0)
            free = freeAt(pos);
        else
            free = freeAtAndAbove(pos);
        if (!free)
        {
            int direction = -1;
            double best = 9999.0;
            if (freeAtAndAbove(pos.west()) && x < best)
            {
                direction = 0;
                best = x;
            }
            if (freeAtAndAbove(pos.east()) && (1.0 - x) < best)
            {
                direction = 1;
                best = 1.0 - x;
            }
            if (freeAtAndAbove(pos.north()) && z < best)
            {
                direction = 4;
                best = z;
            }
            if (freeAtAndAbove(pos.south()) && best > 1.0 - z)
            {
                mZd = 0.1;
            }
            else if (direction == 0)
            {
                mXd = -0.1;
            }
            else if (direction == 1)
            {
                mXd = 0.1;
            }
            else if (direction == 4)
            {
                mZd = -0.1;
            }
        }
    }
    return false;
}

void LocalPlayer::setExperienceValues(float progress, int level, int total)
{
    mGameMode = level;
}

void LocalPlayer::playSound(const SoundEvent* sound, float volume, float pitch)
{
    Entity::playSound(sound, volume, pitch);
}

bool LocalPlayer::isEffectiveAi()
{
    return true;
}

void LocalPlayer::startUsingItem(InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = getItemInHand(hand);
    if (!item->isEmpty() && !mItemUsePending)
    {
        LivingEntity::startUsingItem(hand);
        mUsedItemHand = hand;
        mItemUsePending = true;
    }
}

bool LocalPlayer::isUsingItem()
{
    return mItemUsePending;
}

void LocalPlayer::stopUsingItem()
{
    LivingEntity::stopUsingItem();
    mItemUsePending = false;
}

InteractionHand::EInteractionHand LocalPlayer::getUsedItemHand()
{
    return mUsedItemHand;
}

bool LocalPlayer::isRidingJumpable()
{
    boost::shared_ptr<Entity> vehicle = getVehicle();
    if (!isPassenger() || !vehicle)
        return false;
    boost::shared_ptr<PlayerRideableJumping> rideable =
        boost::dynamic_pointer_cast<PlayerRideableJumping>(vehicle);
    return rideable && rideable->canJump();
}

bool LocalPlayer::isAutoJumpEnabled()
{
    return mAutoJumpEnabled;
}

void LocalPlayer::updateAutoJump(float xMovement, float zMovement)
{
    if (!isAutoJumpEnabled() || mAutoJumpTime > 0)
        return;

    if ((!mOnGround || isPassenger()) &&
        (((isSneaking() || !isPassenger()) && !isInLava()) || mNoPhysics))
        return;

    Vec3* movement = Vec3::newTemp(xMovement, 0.0, zMovement);
    double lengthSquared = movement->lengthSqr();
    double speed = getSpeed();
    if (lengthSquared <= 1.0e-7)
    {
        Vec3* view = getViewVector(1.0f);
        if (view == NULL)
            return;
        double angle = static_cast<double>(mYRot) * 0.01745329238474369;
        double sine = Mth::sin(angle);
        double cosine = Mth::cos(angle);
        double viewX = view->x * speed;
        double viewZ = view->z * speed;
        movement = Vec3::newTemp(viewX * cosine - viewZ * sine, 0.0,
                                 viewZ * cosine + viewX * sine);
        lengthSquared = movement->lengthSqr();
    }
    if (lengthSquared <= 1.0e-7)
        return;

    double inputLength = std::sqrt(lengthSquared);
    movement = movement->scale(Mth::fastInvSqrt(lengthSquared));
    Vec3* view = getViewVector(1.0f);
    if (view == NULL || (view->x == 0.0 && view->z == 0.0) ||
        view->x * movement->x + view->z * movement->z < -0.15)
        return;
    if (isFallFlying())
        return;

    double distance = speed * 0.9;
    if (distance < 1.0 / inputLength)
        distance = 1.0 / inputLength;

    Vec3* start = Vec3::newTemp(mX, mBoundingBox->mMinY, mZ);
    Vec3* target = Vec3::newTemp(mX + xMovement, mBoundingBox->mMinY, mZ + zMovement);
    target->add(movement->x * distance, 0.0, movement->z * distance);
    BlockPos startBlock(start);
    BlockPos targetBlock(target);

    const BlockState* startState = mLevel->getBlockState(startBlock);
    const BlockState* startAboveState = mLevel->getBlockState(startBlock.above());
    if ((startState != NULL && startState->getShape(mLevel, startBlock) != NULL) ||
        (startAboveState != NULL && startAboveState->getShape(mLevel, startBlock.above()) != NULL))
        return;

    double jumpHeight = 1.2;
    MobEffectInstance* jump = getEffect(MobEffects::JUMP);
    if (jump != NULL)
        jumpHeight += static_cast<double>(jump->getAmplifier() + 1) * 0.75;

    AABB* probe = AABB::newTemp(startBlock, targetBlock)->grow(mBbWidth, 0.0, mBbWidth);
    std::vector<AABB*>* collisions = mLevel->getCollisionAABBs(shared_from_this(), probe,
                                                                false, false, false);
    if (collisions == NULL)
        return;

    Vec3* up = Vec3::newTemp(0.0, 1.0, 0.0);
    Vec3* side = Vec3::newTemp(movement)->cross(up)->scale(mBbWidth * 0.5);
    start->add(0.51, 0.0, 0.51);
    target->add(0.51, 0.0, 0.51);
    Vec3* fromLeft = Vec3::newTemp(start)->subtract(side);
    Vec3* toLeft = Vec3::newTemp(target)->subtract(side);
    Vec3* fromRight = Vec3::newTemp(start)->add(side);
    Vec3* toRight = Vec3::newTemp(target)->add(side);

    const double sentinel = static_cast<double>(1.1754943508222875e-38f);
    double highest = sentinel;
    for (std::vector<AABB*>::const_iterator it = collisions->begin();
         it != collisions->end(); ++it)
    {
        if (*it != NULL && ((*it)->intersects(fromLeft, toLeft) ||
                            (*it)->intersects(fromRight, toRight)))
        {
            double candidateHeight = static_cast<double>(static_cast<float>((*it)->mMaxY));
            BlockPos obstacle((*it)->getCenter());
            unsigned int step = 1;
            while (true)
            {
                if (step > 1)
                {
                    BlockPos immediateAbove = obstacle.above();
                    const BlockState* immediateState = mLevel->getBlockState(immediateAbove);
                    const AABB* immediateShape = immediateState == NULL ? NULL :
                        immediateState->getShape(mLevel, immediateAbove);
                    if (immediateShape != NULL)
                        return;
                }
                ++step;
                if (jumpHeight <= static_cast<double>(step) - 1.0)
                    break;
                BlockPos above = obstacle.above(static_cast<int>(step));
                const BlockState* aboveState = mLevel->getBlockState(above);
                const AABB* aboveShape = aboveState == NULL ? NULL :
                    aboveState->getShape(mLevel, above);
                if (aboveShape != NULL)
                {
                    float top = static_cast<float>(aboveShape->mMaxY) +
                        static_cast<float>(static_cast<double>(step) - 1.0);
                    candidateHeight = static_cast<double>(top);
                    if (jumpHeight < candidateHeight - mBoundingBox->mMinY)
                        return;
                }
            }
            highest = candidateHeight;
            break;
        }
    }

    double height = highest - mBoundingBox->mMinY;
    if (highest != sentinel && height > distance && !(jumpHeight < height))
        mAutoJumpTime = 1;
}

void LocalPlayer::mapPlayerChunk(unsigned int targetBlockId)
{
    int localX = (int)std::floor(mX) % 16;
    int localZ = (int)std::floor(mZ) % 16;
    if (localX < 0)
    {
        localX += 16;
    }
    if (localZ < 0)
    {
        localZ += 16;
    }

    std::cout << "player in chunk (" << mChunkX << "," << mChunkZ << ") at ("
              << mX << "," << mY << "," << mZ << ")\n";

    for (int chunkZ = -1; chunkZ <= 1; ++chunkZ)
    {
        for (int z = 0; z < 16; ++z)
        {
            for (int chunkX = -1; chunkX <= 1; ++chunkX)
            {
                LevelChunk* chunk = mLevel->getChunk(mChunkX + chunkX, mChunkZ + chunkZ);
                for (int x = 0; x < 16; ++x)
                {
                    if (chunkX == 0 && chunkZ == 0 && x == localX && z == localZ)
                    {
                        std::cout << 'O';
                        continue;
                    }

                    int blockId = 0;
                    for (int y = 127; y > 0; --y)
                    {
                        blockId = Block::getId(chunk->getBlock(BlockPos(x, y, z)));
                        if (blockId != 0)
                        {
                            break;
                        }
                    }

                    if (targetBlockId != 0 && blockId == (int)targetBlockId)
                    {
                        std::cout << '@';
                    }
                    else if (blockId == 0)
                    {
                        std::cout << ' ';
                    }
                    else if (blockId < 10)
                    {
                        std::cout << blockId;
                    }
                    else
                    {
                        std::cout << '#';
                    }
                }
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }
}

double LocalPlayer::getDamageTaken()
{
    return mDamageTaken;
}

bool LocalPlayer::isSolidBlock(int x, int y, int z)
{
    return mLevel->getBlockState(BlockPos(x, y, z))->isSolid();
}

not_null_ptr<ItemInstance> LocalPlayer::getCarriedItem()
{
    return mInventory->getSelected();
}

MultiPlayerGameMode* LocalPlayer::GetLocalGameMode()
{
    if ((unsigned int)mUserIndex < 4)
    {
        return mMinecraft->mGameModes[mUserIndex];
    }

    for (int index = 0; index < 4; ++index)
    {
        if (static_cast<const void*>(mMinecraft->mLocalPlayers[index].get()) ==
            static_cast<const void*>(this))
        {
            return mMinecraft->mGameModes[index];
        }
    }
    return 0;
}

GameMode* LocalPlayer::GetGameMode()
{
    return GetLocalGameMode();
}

void LocalPlayer::StartSpectating()
{
    GetLocalGameMode()->StartSpectating();
}

void LocalPlayer::OnEquipArmor(not_null_ptr<ItemInstance> item)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    MultiPlayerGameMode* gameMode = minecraft->mGameModes[mUserIndex];
    if (gameMode != 0 && gameMode->getTutorial() != 0)
    {
        gameMode->getTutorial()->onEquip(item);
    }

    minecraft->mGui->ShowCharacterDisplay(mUserIndex);

    Item* ironArmor[4] = {
        Items::ironBoots,
        Items::ironLeggings,
        Items::ironChestplate,
        Items::ironHelmet
    };
    for (int slot = 0; slot < 4; ++slot)
    {
        not_null_ptr<ItemInstance> armor = mInventory->getArmor(slot);
        if (armor->isEmpty() || armor->getItem() != ironArmor[slot])
        {
            return;
        }
    }

    awardStat(GenericStats::ironMan(), GenericStats::param_ironMan());
}

void LocalPlayer::OnTakeFromAnvil(not_null_ptr<ItemInstance> item)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    MultiPlayerGameMode* gameMode = minecraft->mGameModes[mUserIndex];
    if (gameMode != 0 && gameMode->getTutorial() != 0)
    {
        gameMode->getTutorial()->onTakeFromAnvil(item);
    }
}

double LocalPlayer::getUnderwaterVisionClarity()
{
    const float current = mWaterVisionClarity;
    if (current == mWaterVisionClarityO)
        return mWaterVisionClarityTarget;
    double value = 1.0;
    if (current >= 5.0f)
    {
        if (current < 10.0f)
            value = Mth::Lerp(0.6, 1.0, static_cast<double>((current - 5.0f) / 5.0f));
    }
    else
    {
        value = Mth::Lerp(3.0, 0.6, static_cast<double>(current / 5.0f));
    }
    mWaterVisionClarityTarget = static_cast<float>(value);
    mWaterVisionClarityO = current;
    return value;
}

void LocalPlayer::DoAutoAttackOnTouch(boost::shared_ptr<Entity> entity)
{
    boost::shared_ptr<Player> self = boost::static_pointer_cast<Player>(shared_from_this());
    mMinecraft->mGameModes[mUserIndex]->attack(self, entity);
}

double LocalPlayer::GetPercentageTraversed() const
{
    return mPercentageTraversed;
}

void LocalPlayer::SetPercentageTraversed(double percentage)
{
    mPercentageTraversed = percentage;
}

void LocalPlayer::setDamageNearbyMobs(bool damage)
{
    mAutoSpinAttackTicks = 0;
    mDamageNearbyMobs = damage;
}

bool LocalPlayer::getDamageNearbyMobs()
{
    return mDamageNearbyMobs;
}

void LocalPlayer::setAndBroadcastCustomSkin(unsigned int skin)
{
    Player::setCustomSkin(skin);
}

void LocalPlayer::setAndBroadcastCustomCape(unsigned int cape)
{
    Player::setCustomCape(cape);
}
