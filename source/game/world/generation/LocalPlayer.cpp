#include "game/world/generation/LocalPlayer.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerboundChatPacket.hpp"
#include "game/network/ServerboundMovePlayerPacket.hpp"
#include "game/network/ServerboundPlayerAbilitiesPacket.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameType.hpp"
#include "game/core/User.hpp"
#include "game/core/Input.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Gui.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/ui/Tooltips.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/HitResult.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/block/BeaconBlockEntity.hpp"
#include "game/world/block/BrewingStandBlockEntity.hpp"
#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/world/block/DropperBlockEntity.hpp"
#include "game/world/block/FurnaceBlockEntity.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/core/MinecartChest.hpp"
#include "game/core/MinecartHopper.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
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
#include <iostream>

LocalPlayer::LocalPlayer(Minecraft* minecraft, Level* level, User* user, int playerIndex)
    : AbstractClientPlayer(level, user != 0 ? user->name : std::wstring()),
      mPacketListener(0), mPermissionLevel(0), mLastSentX(0.0), mLastSentY(0.0),
      mLastSentZ(0.0), mLastSentYRot(0.0f), mLastSentXRot(0.0f),
      mPositionSendCounter(0), mPreviousOnGround(false), mHealthInitialized(false), mInput(0), mMinecraft(minecraft), mIdleTicks(0),
      mSprinting(false), mThirdPersonViewTimer(0),
      mFallFlyYaw(0.0f), mFallFlyYawOld(0.0f),
      mHandsBusy(false), mItemUsePending(false),
      mUsedItemHand(InteractionHand::eInteractionHand_MainHand),
      mAutoJumpEnabled(true), mAutoJumpTime(0), mViewport(0), mButtonsDown(0),
      mButtonsPressed(0), mMoveInputX(0), mMoveInputY(0), mLookInputX(0), mLookInputY(0),
      mReserved0x928(0), mReserved0x92c(0), mDestroyTicks(0), mLastAttackTick(0),
      mLastUseTick(0), mThirdPersonView(0), mGameMode(0), mDRCTouching(false), mUserIndex(-1),
      mViewBob(), mViewBobPrev(), mViewBobAmount(), mCreativeRepeatState(0), mDamageTaken(0.0f),
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
    return mSprinting;
}

void LocalPlayer::setSprinting(bool sprinting)
{
    mSprinting = sprinting; Entity::setSprinting(sprinting);
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

void LocalPlayer::reallyDrop(boost::shared_ptr<ItemEntity> item)
{
    Player::reallyDrop(item);
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
}

float LocalPlayer::tickHeadTurn(float yRot, float xRot)
{
    return Player::tickHeadTurn(yRot, xRot);
}

void LocalPlayer::aiStep()
{
    if (mInput != NULL)
    {
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
    }

    Player::aiStep();
}

MobEffectInstance* LocalPlayer::removeEffectNoUpdate(MobEffect* effect)
{
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
    return (unsigned int)mThirdPersonView;
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
    return select ? 1U : 0U;
}

unsigned char LocalPlayer::handleMouseClick(int button)
{
    return (unsigned char)(button != 0);
}

unsigned int LocalPlayer::creativeModeHandleMouseClick(int button, bool pressed)
{
    return (unsigned int)(button >= 0 && pressed);
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
    return Player::swing(hand);
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
            getHealth();
            setHealth(amount);
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
        gTelemetryManager.RecordPlayerDiedOrFailed(mUserIndex, (int)mX, (int)mY, (int)mZ, 0,
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
    return 0.0;
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
    return freeAt(pos) && !mLevel->getBlockState(pos.above())->isSolidRender();
}

unsigned int LocalPlayer::freeAt(const BlockPos& pos)
{
    return !mLevel->getBlockState(pos)->isSolidRender();
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
    boost::shared_ptr<AbstractHorse> horse = boost::dynamic_pointer_cast<AbstractHorse>(vehicle);
    return horse && horse->canJump();
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
