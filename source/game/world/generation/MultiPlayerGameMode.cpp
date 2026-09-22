#include "game/world/generation/MultiPlayerGameMode.hpp"

#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/minigame/FullTutorial.hpp"
#include "game/minigame/GameModeTutorial.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/TeamInvite.hpp"
#include "game/core/System.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/network/CraftItemPacket.hpp"
#include "game/network/DebugOptionsPacket.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/ServerboundContainerButtonClickPacket.hpp"
#include "game/network/ServerboundContainerClickPacket.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/network/ServerboundInteractPacket.hpp"
#include "game/network/ServerboundPlayerActionPacket.hpp"
#include "game/network/ServerboundUseItemOnPacket.hpp"
#include "game/network/ServerboundSetCarriedItemPacket.hpp"
#include "game/network/ServerboundSetCreativeModeSlotPacket.hpp"
#include "game/network/ServerboundUseItemPacket.hpp"
#include "game/core/Direction.hpp"
#include "game/core/HitResult.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/PlayerTeam.hpp"

MultiPlayerGameMode::MultiPlayerGameMode(int playerId, ClientPacketListener* connection, int userIndex,
                                         bool primaryPlayer, bool splitScreen)
    : GameMode(), mDestroyBlockPos(-1, -1, -1), mDestroyProgress(0.0f), mDestroyTicks(0), mDestroyDelay(0),
      mConnection(connection), mPlayerId(playerId), mGameModeConstraints(new GameModeConstraints(playerId)),
      mTutorial(0), mTeamInvites(), mMissTime(5.0f), mTutorialsEnabled(false), mDestroying(false),
      mSpectateSoundTime(System::processTimeInMilliSecs()), mCarriedItemSlot(0)
{
    mGameType = GameType::SURVIVAL;
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) && !splitScreen)
    {
        if (primaryPlayer)
        {
            mTutorial = new FullTutorial(playerId, mGameModeConstraints, false);
            mMinecraft->playerStartedTutorial(playerId);
        }
        else
        {
            mTutorial = new Tutorial(playerId, mGameModeConstraints, false, false);
        }
    }
    else
    {
        mTutorial = new GameModeTutorial(playerId, mGameModeConstraints, userIndex);
    }
}

MultiPlayerGameMode::~MultiPlayerGameMode()
{
    std::vector<TeamInvite*>::iterator it = mTeamInvites.begin();
    for (; it != mTeamInvites.end(); ++it)
        delete *it;
    delete mTutorial;
    delete mGameModeConstraints;
}

GameModeConstraints* MultiPlayerGameMode::getGameModeConstraints() { return mGameModeConstraints; }
boost::shared_ptr<Player> MultiPlayerGameMode::createPlayer(Level* level)
{
    return boost::shared_ptr<Player>(new MultiplayerLocalPlayer(
        mMinecraft, level, mMinecraft->mUser, mConnection));
}

void MultiPlayerGameMode::initPlayer(boost::shared_ptr<Player> player)
{
    player->mYRot = -10.5f;
}

void MultiPlayerGameMode::adjustPlayer(boost::shared_ptr<Player> player)
{
    mGameType->updatePlayerAbilities(&player->mAbilities);
}

void MultiPlayerGameMode::ensureHasSentCarriedItem()
{
    const int slot = mMinecraft->mPlayer->mInventory->m_selected;
    if (slot == mCarriedItemSlot)
        return;

    mCarriedItemSlot = slot;
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundSetCarriedItemPacket(slot)));
}

bool MultiPlayerGameMode::_useItemOn(boost::shared_ptr<Player> player, Level* level,
                                    const BlockPos& pos, const Direction* direction,
                                    Vec3* hit, InteractionHand::EInteractionHand hand,
                                    bool simulate, bool* used)
{
    if (used != 0)
        *used = false;

    if (!simulate)
        ensureHasSentCarriedItem();
    if (!level->getWorldBorder()->isWithinBounds(pos))
        return false;

    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (isSpectator())
        return false;

    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();

    if (simulate && player->isAllowedToUse(block))
    {
        const bool handled = block->use(
            level, pos, state, player, hand, direction,
            static_cast<float>(hit->x - pos.x), static_cast<float>(hit->y - pos.y),
            static_cast<float>(hit->z - pos.z), true);
        if (used != 0)
            *used = handled;
        return handled;
    }

    if (!simulate && player->isAllowedToUse(block))
    {
        const float hitX = static_cast<float>(hit->x - pos.x);
        const float hitY = static_cast<float>(hit->y - pos.y);
        const float hitZ = static_cast<float>(hit->z - pos.z);
        const bool handled = block->use(level, pos, state, player, hand, direction,
                                        hitX, hitY, hitZ, simulate);
        if (handled)
        {
            if (!simulate && mConnection != 0)
                mConnection->send(boost::shared_ptr<Packet>(new ServerboundUseItemOnPacket(
                    pos, direction, hand, hitX, hitY, hitZ)));
            if (used != 0)
                *used = true;
            return true;
        }
    }

    if (item->isEmpty() || !player->isAllowedToUse(item))
        return false;
    if (player->getCooldowns()->isOnCooldown(item->getItem()))
        return false;

    const int oldCount = item->getCount();
    const int oldAux = item->getAuxValue();
    const int itemResult = item->useOn(
        player, level, pos, hand, direction,
        static_cast<float>(hit->x - pos.x), static_cast<float>(hit->y - pos.y),
        static_cast<float>(hit->z - pos.z), simulate);

    if (isCreative())
    {
        item->setAuxValue(oldAux);
        item->setCount(oldCount);
    }
    if (!simulate && item->getCount() != oldCount)
        player->setItemInHand(hand, item);
    if (!simulate && itemResult != 1 && mConnection != 0)
        mConnection->send(boost::shared_ptr<Packet>(new ServerboundUseItemOnPacket(
            pos, direction, hand, static_cast<float>(hit->x - pos.x),
            static_cast<float>(hit->y - pos.y), static_cast<float>(hit->z - pos.z))));
    if (used != 0)
        *used = itemResult == 0;
    return itemResult == 0;
}

bool MultiPlayerGameMode::GetTutorialsEnabled()
{
    return mTutorialsEnabled;
}
InteractionResult::EInteractionResult MultiPlayerGameMode::useItemOn(
    boost::shared_ptr<Player> player, Level* level, const BlockPos& pos, const Direction* direction,
    Vec3* hit, InteractionHand::EInteractionHand hand, bool simulate, bool* used)
{
    not_null_ptr<ItemInstance> startingItem = player->getItemInHand(hand);
    bool trackStartingCount = false;
    int startingCount = 0;
    if (!mTutorial->tasksChecked && GetTutorialsEnabled())
    {
        mTutorial->useItemOn(level, startingItem, pos, simulate);
        mTutorial->useItemOn(startingItem, simulate);
        if (!simulate && !startingItem->isEmpty())
        {
            trackStartingCount = true;
            startingCount = startingItem->getCount();
        }
    }
    bool handled = _useItemOn(player, level, pos, direction, hit, hand, simulate, used);
    if (!simulate && !mTutorial->tasksChecked)
    {
        if (GetTutorialsEnabled() && handled && trackStartingCount)
        {
            if (startingItem->getCount() < startingCount)
                mTutorial->useItemOn(startingItem, false);
        }
    }
    return handled ? InteractionResult::eInteractionResult_Success
                   : InteractionResult::eInteractionResult_Pass;
}

bool MultiPlayerGameMode::hasMissTime()
{
    return mMissTime > 0.0f;
}

void MultiPlayerGameMode::attack(boost::shared_ptr<Player> player, boost::shared_ptr<Entity> entity)
{
    if (!mTutorial->tasksChecked && GetTutorialsEnabled())
        mTutorial->attack(player, entity);

    ensureHasSentCarriedItem();
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundInteractPacket(entity)));

    if (!isSpectator())
    {
        player->attack(entity);
        player->resetAttackStrengthTicker();
    }
}

InteractionResult::EInteractionResult MultiPlayerGameMode::interact(
    boost::shared_ptr<Player> player, boost::shared_ptr<Entity> entity, not_null_ptr<ItemInstance>,
    InteractionHand::EInteractionHand hand)
{
    ensureHasSentCarriedItem();
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundInteractPacket(entity, hand)));
    if (!isSpectator())
        return static_cast<InteractionResult::EInteractionResult>(player->interactOn(entity, hand));
    return InteractionResult::eInteractionResult_Pass;
}

InteractionResult::EInteractionResult MultiPlayerGameMode::interactAt(
    boost::shared_ptr<Player> player, boost::shared_ptr<Entity> entity, const HitResult* hit,
    not_null_ptr<ItemInstance>, InteractionHand::EInteractionHand hand)
{
    ensureHasSentCarriedItem();
    Vec3* relative = Vec3::newTemp(hit->mLocation->x - entity->mX, hit->mLocation->y - entity->mY,
                                    hit->mLocation->z - entity->mZ);
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundInteractPacket(entity, hand, relative)));
    if (!isSpectator())
        return static_cast<InteractionResult::EInteractionResult>(entity->interactAt(player, relative, hand));
    return InteractionResult::eInteractionResult_Pass;
}
bool MultiPlayerGameMode::hasInfiniteItems() { return isCreative(); }
void MultiPlayerGameMode::handlePickItem(const boost::shared_ptr<ItemInstance>& item)
{
    ByteArrayOutputStream bytes;
    DataOutputStream output(&bytes);
    output.writeItem(not_null_ptr<ItemInstance>(item));
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundCustomPayloadPacket(
        std::wstring(L"MC|PickItem"), bytes.toByteArray())));
}
bool MultiPlayerGameMode::isInputAllowed(int mapping)
{
    bool allowed = true;
    if (GetTutorialsEnabled() && !mTutorial->tasksChecked
        && !mGameModeConstraints->isInputAllowed(mapping))
        allowed = false;

    bool positionOk = true;
    switch (mapping)
    {
    case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x20:
        if (mMinecraft->mPlayer->PositionLocked())
        {
            positionOk = false;
        }
        else
        {
            if (mMinecraft->mPlayer->IsSpectatingOtherPlayer())
                positionOk = false;
            if (mMinecraft->mPlayer->GetThirdPersonView(false) == 3)
                positionOk = false;
        }
        break;

    case 0x1c: case 0x1d: case 0x1e: case 0x1f:
        if (mMinecraft->mPlayer->GetThirdPersonView(false) == 3)
            positionOk = false;
        break;

    case 0x21:
        if (Minecraft::InMiniGame(eMiniGameId_Any, true)
            && mMinecraft->mPlayer->PositionLocked())
            positionOk = false;
        break;

    case 0x22:
        break;

    case 0x23: case 0x24: case 0x25: case 0x26: case 0x28:
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        {
            const bool positionLocked = mMinecraft->mPlayer->PositionLocked();
            const bool lockedInventory = Minecraft::GetInstance()->GetMiniGame()->LockedInventory();
            const bool inScoringState = Minecraft::GetInstance()->GetClientMasterGameMode()->IsInScoringState();
            if (positionLocked || lockedInventory || inScoringState)
                positionOk = false;
        }
        break;

    case 0x27: case 0x29:
        if (mMinecraft->mPlayer->IsSpectatingOtherPlayer())
            positionOk = false;
        if (mMinecraft->mPlayer->GetThirdPersonView(false) == 3)
            positionOk = false;
        break;

    default:
        break;
    }

    return allowed && positionOk;
}
bool MultiPlayerGameMode::destroyBlock(const BlockPos& pos)
{
    if (!mTutorial->tasksChecked && GetTutorialsEnabled())
    {
        const BlockState* state = mMinecraft->mLevel->getBlockState(pos);
        mTutorial->destroyBlock(state->getBlock());
    }

    not_null_ptr<ItemInstance> heldItem = mMinecraft->mPlayer->getMainHandItem();
    int oldDamage = 0;
    if (!heldItem->isEmpty())
        oldDamage = heldItem->getDamageValue();

    bool result = _destroyBlock(pos);

    if (!mTutorial->tasksChecked && GetTutorialsEnabled())
    {
        if (heldItem->isDamageableItem())
        {
            const int maxDamage = heldItem->getMaxDamage();
            const int newDamage = heldItem->getDamageValue();
            if (oldDamage < newDamage && maxDamage / 2 < newDamage)
                mTutorial->itemDamaged(heldItem);
        }
    }

    return result;
}
void MultiPlayerGameMode::creativeDestroyBlock(Minecraft*, MultiPlayerGameMode* mode, const BlockPos& pos, const Direction*) { if (mode != 0) mode->_destroyBlock(pos); }
bool MultiPlayerGameMode::sameDestroyTarget(const BlockPos& pos)
{
    not_null_ptr<ItemInstance> heldItem = mMinecraft->mPlayer->getMainHandItem();
    bool result = false;
    if (pos.equals(mDestroyBlockPos))
    {
        if (heldItem->getItem() == mDestroyItem->getItem() &&
            ItemInstance::tagMatches(heldItem, mDestroyItem))
        {
            if (heldItem->isDamageableItem() || heldItem->getAuxValue() == mDestroyItem->getAuxValue())
                result = true;
        }
    }
    return result;
}
bool MultiPlayerGameMode::continueDestroyBlock(const BlockPos& pos, const Direction* direction)
{
    if (!mMinecraft->mPlayer->isAllowedToMine())
        return false;

    ensureHasSentCarriedItem();

    if (mDestroyDelay > 0)
    {
        --mDestroyDelay;
        return true;
    }

    if (isCreative() && mMinecraft->mLevel->getWorldBorder()->isWithinBounds(pos))
    {
        mDestroyDelay = 5;
        mConnection->send(boost::shared_ptr<Packet>(new ServerboundPlayerActionPacket(
            ServerboundPlayerActionPacket::START_DESTROY_BLOCK, pos, direction, 0)));
        creativeDestroyBlock(mMinecraft, this, pos, direction);
        return true;
    }

    if (!sameDestroyTarget(pos))
        return startDestroyBlock(pos, direction);

    const BlockState* state = mMinecraft->mLevel->getBlockState(pos);
    if (state->getMaterial() == Material::AIR)
    {
        mDestroying = false;
        return false;
    }

    if (isSpectator() && Minecraft::GetInstance()->GetMiniGame()->HasSpectatorParticipation())
        mDestroyProgress += 0.02f;
    else
        mDestroyProgress += state->getDestroyProgress(mMinecraft->mPlayer, mMinecraft->mPlayer->mLevel, pos);
    ++mDestroyTicks;

    if (mDestroyProgress >= 1.0f)
    {
        mDestroying = false;
        mConnection->send(boost::shared_ptr<Packet>(new ServerboundPlayerActionPacket(
            ServerboundPlayerActionPacket::STOP_DESTROY_BLOCK, pos, direction, 0)));
        destroyBlock(pos);
        mDestroyTicks = 0;
        mDestroyDelay = 5;
        mDestroyProgress = 0.0f;
    }

    mMinecraft->mLevel->destroyBlockProgress(mMinecraft->mPlayer->getId(), mDestroyBlockPos,
        static_cast<int>(mDestroyProgress * 10.0f) - 1);
    return true;
}
void MultiPlayerGameMode::stopDestroyBlock()
{
    if (!mDestroying)
        return;

    mConnection->send(boost::shared_ptr<Packet>(new ServerboundPlayerActionPacket(
        ServerboundPlayerActionPacket::ABORT_DESTROY_BLOCK, mDestroyBlockPos, Direction::DOWN, 0)));

    mDestroyProgress = 0.0f;
    mDestroying = false;

    mMinecraft->mLevel->destroyBlockProgress(mMinecraft->mPlayer->getId(), mDestroyBlockPos, -1);
    mMinecraft->mPlayer->resetAttackStrengthTicker();
}
Tutorial* MultiPlayerGameMode::getTutorial() { return mTutorial; }
void MultiPlayerGameMode::releaseUsingItem(boost::shared_ptr<Player> player)
{
    ensureHasSentCarriedItem();
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundPlayerActionPacket(
        ServerboundPlayerActionPacket::RELEASE_USE_ITEM, BlockPos::ZERO,
        Direction::DOWN, player->getUseItemRemainingTicks())));
    player->releaseUsingItem();
}
bool MultiPlayerGameMode::isServerControlledInventory()
{
    bool result = false;
    if (mMinecraft->mPlayer->isPassenger())
    {
        boost::shared_ptr<Entity> vehicle = mMinecraft->mPlayer->getVehicle();
        unsigned long long type = vehicle->GetType();
        if ((type & 0x0D690000ULL) == 0x0D690000ULL)
            result = true;
    }
    return result;
}
boost::shared_ptr<Player> MultiPlayerGameMode::GetLocalPlayer()
{
    return boost::static_pointer_cast<Player>(mMinecraft->mLocalPlayers[mPlayerId]);
}
bool MultiPlayerGameMode::SpectateSoundEnabled()
{
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true))
        return false;
    boost::shared_ptr<Player> player = GetLocalPlayer();
    if (!player || !player->SpectateAsOtherEntity())
        return false;
    if (mMinecraft == NULL || mMinecraft->GetMiniGame() == NULL)
        return false;
    return !mMinecraft->GetMiniGame()->IsBatSqueakDisabled();
}
double MultiPlayerGameMode::CalcSpectateSoundAllowance()
{
    const uint64_t now = System::processTimeInMilliSecs();
    const double elapsed = static_cast<double>(now - mSpectateSoundTime);
    double allowance = static_cast<double>(mMissTime) + elapsed * 10.0 / 8000.0;
    if (allowance > 10.0)
        allowance = 10.0;
    return allowance;
}
bool MultiPlayerGameMode::CanMakeSpectateSound()
{
    if (!SpectateSoundEnabled())
        return false;
    const uint64_t now = System::processTimeInMilliSecs();
    return CalcSpectateSoundAllowance() >= 1.0 && now - mSpectateSoundTime > 300;
}
void MultiPlayerGameMode::MakeSpectateSound()
{
    boost::shared_ptr<Player> player = GetLocalPlayer();
    const unsigned long long entityType = player->GetSpectateEntity();
    const SoundEvent* sound = 0;
    if (entityType == 0x5D540048ULL)
        sound = SoundEvents::VEX_CHARGE;
    else if (entityType == 0x2D6C0003ULL)
        sound = SoundEvents::PARROT_AMBIENT;
    else if (entityType == 0x1D000101ULL)
        sound = SoundEvents::BAT_AMBIENT;
    if (!sound || !mConnection)
        return;
    mMissTime = static_cast<float>(CalcSpectateSoundAllowance() - 1.0);
    mSpectateSoundTime = System::processTimeInMilliSecs();
    mConnection->send(boost::shared_ptr<Packet>(new ClientboundSoundPacket(
        sound, SoundSource::eSoundSource_Master, player->mX, player->mY, player->mZ, 0.5f, 0.0f)));
}
bool MultiPlayerGameMode::isCutScene() { return false; }




void MultiPlayerGameMode::Follow(bool follow, bool immediate)
{
    if (!mConnection)
        return;
    GameModePacket::FollowData data(follow, immediate);
    mConnection->send(GameModePacket::Create(GameModePacket::MessageUnknown, data));
}
void MultiPlayerGameMode::StartSpectating()
{
    if (!mConnection)
        return;
    GameModePacket::EmptyData data;
    mConnection->send(GameModePacket::Create(static_cast<GameModePacket::EMessage>(1), data));


}
bool MultiPlayerGameMode::startDestroyBlock(const BlockPos& pos, const Direction*) { mDestroyBlockPos = pos; mDestroying = true; return true; }
bool MultiPlayerGameMode::canHurtPlayer() { return true; }
void MultiPlayerGameMode::tick() { if (mMissTime > 0.0f) mMissTime -= 1.0f; }
InteractionResult::EInteractionResult MultiPlayerGameMode::useItem(
    boost::shared_ptr<Player> player, Level* level, InteractionHand::EInteractionHand hand, bool simulate)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);

    if (isSpectator())
        return InteractionResult::eInteractionResult_Pass;

    if (!player->isAllowedToUse(item))
        return InteractionResult::eInteractionResult_Pass;

    InteractionResult::EInteractionResult result;
    if (!simulate)
    {
        ensureHasSentCarriedItem();

        if (player->getCooldowns()->isOnCooldown(item->getItem()))
            return InteractionResult::eInteractionResult_Pass;

        int savedCount = item->getCount();
        item->use(level, player, hand);

        if (item->getCount() != savedCount)
            player->setItemInHand(hand, item);

        result = InteractionResult::eInteractionResult_Success;
    }
    else
    {
        bool canUse = item->TestUse(level, player, hand);
        result = canUse ? InteractionResult::eInteractionResult_Success : InteractionResult::eInteractionResult_Pass;
    }

    if (!simulate && result != InteractionResult::eInteractionResult_Pass)
        mConnection->send(boost::shared_ptr<Packet>(new ServerboundUseItemPacket(hand)));

    return result;
}
void MultiPlayerGameMode::SetGameType(const GameType* gameType)
{
    boost::shared_ptr<Player> player = GetLocalPlayer();
    if (!mMinecraft->GetClientMasterGameMode()->IsRoundEnding() && gameType != GameType::LOBBY &&
        gameType == GameType::SPECTATOR && getTutorial() != 0)
    {
        eTutorial_GameMode mode = static_cast<eTutorial_GameMode>(7);
        if (player->SpectateAsInvisible())
            mode = static_cast<eTutorial_GameMode>(6);
        getTutorial()->onGameModeChange(mode);
    }
    mGameType = gameType;
    if (!player->isPassenger())
        player->UpdateSize();
}
const GameType* MultiPlayerGameMode::GetGameType() { return mGameType; }
void MultiPlayerGameMode::setLocalMode(const GameType* gameType) { mGameType = gameType; }
bool MultiPlayerGameMode::_destroyBlock(const BlockPos& pos)
{
    if (!MayDestroy(pos))
        return false;

    Level* level = mMinecraft->mLevel;
    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    if (state->getMaterial() == Material::AIR)
        return false;

    level->levelEvent(2001, pos, Block::getIdWithData(state));

    boost::shared_ptr<Player> player = GetLocalPlayer();
    bool handled = block->playerWillDestroy(level, pos, state, player);
    if (!handled)
    {
        handled = level->setBlock(pos, Blocks::AIR->defaultBlockState(), 11, false);
        if (handled)
            block->destroy(level, pos, state);
    }

    mDestroyBlockPos = BlockPos(mDestroyBlockPos.x, -1, mDestroyBlockPos.z);

    if (!isCreative())
    {
        not_null_ptr<ItemInstance> heldItem = mMinecraft->mPlayer->getMainHandItem();
        if (!heldItem->isEmpty())
        {
            heldItem->mineBlock(level, state, pos, player);
            if (heldItem->isEmpty())
                mMinecraft->mPlayer->setItemInHand(InteractionHand::eInteractionHand_MainHand, heldItem);
        }
    }

    return handled;
}
double MultiPlayerGameMode::getPickRange() { return hasFarPickRange() ? 5.0 : 4.5; }
boost::shared_ptr<ItemInstance> MultiPlayerGameMode::handleTakeAllInvenetory(
    int containerId, int button, int start, int count, boost::shared_ptr<Player> player)
{
    int uid = player->mContainerMenu->backup(player->mInventory);
    AbstractContainerMenu::ItemList items = player->mContainerMenu->getAllItems(start, count);
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundContainerClickPacket(
        containerId, start, button, ClickType::eClickType_QuickMove, items, static_cast<short>(uid), count)));
    return boost::shared_ptr<ItemInstance>();
}
boost::shared_ptr<ItemInstance> MultiPlayerGameMode::handleInventoryMouseClick(
    int containerId, int slot, int button, ClickType::eClickType clickType, boost::shared_ptr<Player> player)
{
    int uid = player->mContainerMenu->backup(player->mInventory);
    not_null_ptr<ItemInstance> item = player->mContainerMenu->clicked(slot, button, clickType, player);
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundContainerClickPacket(
        containerId, slot, button, clickType, item, static_cast<short>(uid))));
    return item;
}
void MultiPlayerGameMode::handleInventoryButtonClick(int containerId, int button)
{
    if (!mConnection)
        return;
    mConnection->send(boost::shared_ptr<Packet>(
        new ServerboundContainerButtonClickPacket(containerId, button)));
}

void MultiPlayerGameMode::handleCreativeModeItemAdd(not_null_ptr<ItemInstance> item, int slot)
{
    if (!isCreative())
        return;
    mConnection->send(boost::shared_ptr<Packet>(
        new ServerboundSetCreativeModeSlotPacket(slot, item)));
}

void MultiPlayerGameMode::handleCreativeModeItemDrop(not_null_ptr<ItemInstance> item)
{
    if (!isCreative())
        return;
    if (item->isEmpty())
        return;
    mConnection->send(boost::shared_ptr<Packet>(
        new ServerboundSetCreativeModeSlotPacket(-1, item)));
}
bool MultiPlayerGameMode::hasExperience() { return false; }
bool MultiPlayerGameMode::hasFarPickRange() { return isCreative(); }
boost::shared_ptr<ItemInstance> MultiPlayerGameMode::handleCraftItem(
    int category, int recipe, boost::shared_ptr<Player> player)
{
    int uid = player->mContainerMenu->backup(player->mInventory);
    mConnection->send(boost::shared_ptr<Packet>(new CraftItemPacket(category, recipe, static_cast<short>(uid))));
    return GameMode::handleCraftItem(category, recipe, player);
}
boost::shared_ptr<ItemInstance> MultiPlayerGameMode::handleStackedCraftItem(
    std::vector<std::pair<int, int> >& recipes, int category, boost::shared_ptr<Player> player)
{
    int uid = player->mContainerMenu->backup(player->mInventory);
    mConnection->send(boost::shared_ptr<Packet>(new CraftItemPacket(recipes, category, static_cast<short>(uid))));
    return GameMode::handleStackedCraftItem(recipes, category, player);
}
boost::shared_ptr<ItemInstance> MultiPlayerGameMode::handleCustomCraftItem(
    std::vector<int>& recipes, boost::shared_ptr<Player> player)
{
    int uid = player->mContainerMenu->backup(player->mInventory);
    mConnection->send(boost::shared_ptr<Packet>(new CraftItemPacket(recipes, static_cast<short>(uid))));
    return GameMode::handleCustomCraftItem(recipes, player);
}
void MultiPlayerGameMode::handleDebugOptions(unsigned int option, boost::shared_ptr<Player> player)
{
    player->mDebugOptions = option;
    mConnection->send(boost::shared_ptr<Packet>(new DebugOptionsPacket(option)));
}
const MiniGameDef* MultiPlayerGameMode::GetMiniGame() { return mMinecraft == 0 ? 0 : mMinecraft->GetMiniGame(); }
Level* MultiPlayerGameMode::GetLevel()
{
    return mMinecraft->mLevel;
}

boost::shared_ptr<Player> MultiPlayerGameMode::GetPlayer()
{
    return GetLocalPlayer();
}
void MultiPlayerGameMode::OnMouseDown() {}

std::vector<TeamInvite*>::iterator MultiPlayerGameMode::FindTeamInvite(unsigned int teamId)
{
    std::vector<TeamInvite*>::iterator it = mTeamInvites.begin();
    for (; it != mTeamInvites.end(); ++it)
        if ((*it)->m_uTeamID == teamId) break;
    return it;
}

void MultiPlayerGameMode::SyncTeamInvite(TeamInvite* invite)
{
    if (invite == 0) return;
    std::vector<TeamInvite*>::iterator it = FindTeamInvite(invite->m_uTeamID);
    if (it == mTeamInvites.end()) mTeamInvites.push_back(invite);
    else *it = invite;
}

void MultiPlayerGameMode::RemoveTeamInvite(unsigned int teamId)
{
    std::vector<TeamInvite*>::iterator it = FindTeamInvite(teamId);
    if (it != mTeamInvites.end()) { delete *it; mTeamInvites.erase(it); }
}

std::vector<TeamInvite*>::iterator MultiPlayerGameMode::FindTeamInviteFrom(std::wstring playerName)
{
    std::vector<TeamInvite*>::iterator it = mTeamInvites.begin();
    for (; it != mTeamInvites.end(); ++it)
        if (*it != 0 && (*it)->m_sFromPlayerName == playerName) break;
    return it;
}

bool MultiPlayerGameMode::IsTeamInvite(std::wstring playerName) { return FindTeamInviteFrom(playerName) != mTeamInvites.end(); }
void MultiPlayerGameMode::SendTeamInvite(std::wstring playerName)
{
    if (isSpectator() || !mConnection)
        return;
    GameModePacket::SendTeamInviteData data;
    data.playerName = playerName;
        mConnection->send(GameModePacket::Create(static_cast<GameModePacket::EMessage>(8), data));
}
std::vector<TeamInvite*>::iterator MultiPlayerGameMode::FindTeamInviteTo(std::wstring playerName)
{
    std::vector<TeamInvite*>::iterator it = mTeamInvites.begin();
    for (; it != mTeamInvites.end(); ++it)
        if (*it != 0 && (*it)->m_sToPlayerName == playerName) break;
    return it;
}
bool MultiPlayerGameMode::CanSendInvite(std::wstring playerName, PlayerUID playerUid)
{
    if (isSpectator())
        return false;

    Scoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetScoreboard();
    boost::shared_ptr<Player> player = GetPlayer();
    PlayerTeam* myTeam = scoreboard->getPlayersTeam(player->getSUID());
    PlayerTeam* theirTeam = scoreboard->getPlayersTeam(playerUid);

    if (myTeam != 0 && theirTeam != 0 && myTeam->getPlayerCount() < 2)
    {
        boost::shared_ptr<Player> self = GetPlayer();
        if (!(self->getSUID() == playerUid) && myTeam != theirTeam)
            return FindTeamInviteTo(playerName) == mTeamInvites.end();
    }
    return false;
}
bool MultiPlayerGameMode::CanLeaveTeam(PlayerUID) { return !isSpectator(); }
void MultiPlayerGameMode::AcceptTeamInvite(std::wstring playerName)
{
    if (!mConnection)
        return;
    std::vector<TeamInvite*>::iterator it = FindTeamInviteFrom(playerName);
    if (it == mTeamInvites.end() || *it == NULL)
        return;
    GameModePacket::TeamIDData data((*it)->m_uTeamID);
    mConnection->send(GameModePacket::Create(static_cast<GameModePacket::EMessage>(0xB), data));
}

void MultiPlayerGameMode::LeaveTeam()
{
    if (isSpectator())
        return;

    Scoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetScoreboard();
    boost::shared_ptr<Player> player = GetPlayer();
    PlayerTeam* team = scoreboard->getPlayersTeam(player->getSUID());
    if (team != 0)
    {
        GameModePacket::EmptyData data;
        mConnection->send(GameModePacket::Create(static_cast<GameModePacket::EMessage>(0x10), data));
    }
}
bool MultiPlayerGameMode::RecordLastCheckpoint(int checkpoint, int, bool, bool, bool)
{
    boost::shared_ptr<Player> player = GetPlayer();
    return mMinecraft->GetClientMasterGameMode()->SetLastCheckpointID(player->getSUID(), checkpoint, false);
}
