#include "game/world/generation/LocalPlayer.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameType.hpp"
#include "game/core/User.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Gui.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/ui/Tooltips.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"

#include <cmath>
#include <iostream>

LocalPlayer::LocalPlayer(Minecraft* minecraft, Level* level, User* user, int playerIndex)
    : AbstractClientPlayer(level, user != 0 ? user->name : std::wstring()),
      mPacketListener(0), mPermissionLevel(0), mInput(0), mMinecraft(minecraft), mIdleTicks(0),
      mSprinting(false),
      mHandsBusy(false), mAutoJumpEnabled(true), mAutoJumpTime(0), mViewport(0), mButtonsDown(0),
      mButtonsPressed(0), mMoveInputX(0), mMoveInputY(0), mLookInputX(0), mLookInputY(0),
      mReserved0x928(0), mReserved0x92c(0), mDestroyTicks(0), mLastAttackTick(0),
      mLastUseTick(0), mThirdPersonView(0), mGameMode(0), mDRCTouching(false), mUserIndex(-1),
      mViewBob(), mViewBobPrev(), mViewBobAmount(), mCreativeRepeatState(0), mDamageTaken(0.0f),
      mHasOpenedInventory(true), mWaitingForScoreboard(false),
      mSessionTimer(0.0), mAdditionalModelParts(), mPercentageTraversed(0.0),
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
    return 0;
}

int LocalPlayer::openContainer(boost::shared_ptr<Container> container)
{
    return 0;
}

int LocalPlayer::openHorseInventory(boost::shared_ptr<AbstractHorse> horse, boost::shared_ptr<Container> container)
{
    return 0;
}

int LocalPlayer::startBlockInteraction(InteractionObject& interaction)
{
    return 0;
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
    return 0;
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
    return 0;
}

void LocalPlayer::SetGameType(const GameType* gameType)
{
    mGameMode = gameType == 0 ? 0 : gameType->getId();
    Tooltips::ResetAllTooltips(mUserIndex);
}

unsigned long long LocalPlayer::GetType()
{
    return 0x2DULL;
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
    return true;
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
    return (unsigned long long)(button + (pressed ? 1 : 0));
}

void LocalPlayer::SetWaitingForScoreboard(bool waiting)
{
    mWaitingForScoreboard = waiting;
}

void LocalPlayer::updateRichPresence()
{
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
    return true;
}

int LocalPlayer::getIndicatorSegment(float value, float maxValue)
{
    return maxValue == 0.0f ? 0 : (int)(value * 10.0f / maxValue);
}

void LocalPlayer::updateProximityIndicator()
{
}

bool LocalPlayer::isControlledCamera()
{
    return true;
}

unsigned long long LocalPlayer::sendPosition()
{
    return 0;
}

unsigned long long LocalPlayer::chat(const std::wstring& message)
{
    return (unsigned long long)message.size();
}

bool LocalPlayer::swing(InteractionHand::EInteractionHand hand)
{
    return Player::swing(hand);
}

void LocalPlayer::respawn()
{
}

void LocalPlayer::actuallyHurt(const DamageSource* source, float amount)
{
    mDamageTaken += amount; Player::actuallyHurt(source, amount);
}

void LocalPlayer::clientSideCloseContainer()
{
}

void LocalPlayer::hurtTo(float amount, TelemetryDamageSource source)
{
    mDamageTaken = amount;
}

void LocalPlayer::onUpdateAbilities()
{
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
    return 0;
}

unsigned long long LocalPlayer::sendOpenInventory()
{
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
    return 1;
}

unsigned int LocalPlayer::freeAt(const BlockPos& pos)
{
    return 1;
}

bool LocalPlayer::checkInBlock(double x, double y, double z)
{
    return false;
}

void LocalPlayer::setExperienceValues(float progress, int level, int total)
{
    mGameMode = level;
}

void LocalPlayer::playSound(const SoundEvent* sound, float volume, float pitch)
{
}

bool LocalPlayer::isEffectiveAi()
{
    return true;
}

void LocalPlayer::startUsingItem(InteractionHand::EInteractionHand hand)
{
}

bool LocalPlayer::isUsingItem()
{
    return false;
}

void LocalPlayer::stopUsingItem()
{
}

InteractionHand::EInteractionHand LocalPlayer::getUsedItemHand()
{
    return InteractionHand::EInteractionHand(0);
}

bool LocalPlayer::isRidingJumpable()
{
    return false;
}

bool LocalPlayer::isAutoJumpEnabled()
{
    return mAutoJumpEnabled;
}

void LocalPlayer::updateAutoJump(float xMovement, float zMovement)
{
    if (!isAutoJumpEnabled() || mAutoJumpTime > 0 || !mOnGround || isSneaking() ||
        isPassenger() || isInWater() || isInLava() || isFallFlying())
    {
        return;
    }

    double movementLengthSquared = xMovement * xMovement + zMovement * zMovement;
    if (movementLengthSquared <= 1.0e-7)
    {
        return;
    }

    double movementLength = std::sqrt(movementLengthSquared);
    double directionX = xMovement / movementLength;
    double directionZ = zMovement / movementLength;
    Vec3* view = getViewVector(1.0f);
    if (view == 0 || view->x * directionX + view->z * directionZ < -0.15)
    {
        return;
    }

    double lookAhead = getSpeed() * 7.0;
    if (lookAhead < 1.0 / movementLength)
    {
        lookAhead = 1.0 / movementLength;
    }

    int targetX = (int)std::floor(mX + directionX * lookAhead);
    int targetY = (int)std::floor(mY);
    int targetZ = (int)std::floor(mZ + directionZ * lookAhead);
    if (isSolidBlock(targetX, targetY, targetZ) &&
        !isSolidBlock(targetX, targetY + 1, targetZ) &&
        !isSolidBlock(targetX, targetY + 2, targetZ))
    {
        mAutoJumpTime = 1;
    }
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

unsigned int LocalPlayer::GetGameMode()
{
    return (unsigned int)GetLocalGameMode();
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
    return 1.0;
}

void LocalPlayer::DoAutoAttackOnTouch(boost::shared_ptr<Entity> entity)
{
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
