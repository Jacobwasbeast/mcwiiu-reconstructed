#include "game/world/generation/MultiPlayerGameMode.hpp"

#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/minigame/Tutorial.hpp"
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
#include "game/network/ServerboundSetCarriedItemPacket.hpp"
#include "game/network/ServerboundSetCreativeModeSlotPacket.hpp"
#include "game/core/Direction.hpp"
#include "game/core/HitResult.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/util/Vec3.hpp"

MultiPlayerGameMode::MultiPlayerGameMode(int playerId, ClientPacketListener* connection, int,
                                         bool, bool)
    : GameMode(), mDestroyBlockPos(), mDestroyProgress(0.0f), mDestroyTicks(0),
      mConnection(connection), mPlayerId(playerId), mGameModeConstraints(new GameModeConstraints(playerId)),
      mTutorial(0), mTeamInvites(), mMissTime(0.0f), mTutorialsEnabled(false), mDestroying(false),
      mSpectateSoundTime(0), mCarriedItemSlot(0)
{
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
    boost::shared_ptr<Player> player = GetLocalPlayer();
    const int slot = player->mInventory->m_selected;
    if (slot == mCarriedItemSlot)
        return;

    mCarriedItemSlot = slot;
    boost::shared_ptr<Packet> packet(new ServerboundSetCarriedItemPacket(slot));
    mConnection->send(packet);
}

bool MultiPlayerGameMode::_useItemOn(boost::shared_ptr<Player>, Level*, const BlockPos&,
                                    const Direction*, Vec3*, InteractionHand::EInteractionHand,
                                    bool, bool*)
{
    return false;
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
    return static_cast<InteractionResult::EInteractionResult>(handled);
}

bool MultiPlayerGameMode::hasMissTime()
{
    return mMissTime > 0.0f;
}

void MultiPlayerGameMode::attack(boost::shared_ptr<Player> player, boost::shared_ptr<Entity> entity)
{
    if (!player || !entity)
        return;
    if (mTutorialsEnabled && mTutorial != NULL)
        mTutorial->attack(player, entity);
    player->attack(entity);
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
    if (!item || !mConnection)
        return;
    ByteArrayOutputStream bytes;
    DataOutputStream output(&bytes);
    output.writeItem(not_null_ptr<ItemInstance>(item));
    mConnection->send(boost::shared_ptr<Packet>(new ServerboundCustomPayloadPacket(
        std::wstring(L"MC|PickItem"), bytes.toByteArray())));
}
bool MultiPlayerGameMode::isInputAllowed(int mapping) { return mGameModeConstraints == 0 || mGameModeConstraints->isInputAllowed(mapping); }
bool MultiPlayerGameMode::destroyBlock(const BlockPos& pos) { return _destroyBlock(pos); }
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
bool MultiPlayerGameMode::continueDestroyBlock(const BlockPos& pos, const Direction*) { mDestroyBlockPos = pos; mDestroying = true; return true; }
void MultiPlayerGameMode::stopDestroyBlock() { mDestroying = false; mDestroyProgress = 0.0f; }
Tutorial* MultiPlayerGameMode::getTutorial() { return mTutorial; }
void MultiPlayerGameMode::releaseUsingItem(boost::shared_ptr<Player> player)
{
    ensureHasSentCarriedItem();
    boost::shared_ptr<Packet> packet(new ServerboundPlayerActionPacket(
        ServerboundPlayerActionPacket::RELEASE_USE_ITEM, BlockPos::ZERO,
        Direction::DOWN, player->getUseItemRemainingTicks()));
    mConnection->send(packet);
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
    if (!Minecraft::InMiniGame(static_cast<EMiniGameId>(15), true))
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
    if (!player)
        return;
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
    mConnection->send(GameModePacket::Create(static_cast<GameModePacket::EMessage>(0), data));
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

    if (!simulate)
    {
        if (player->getCooldowns()->isOnCooldown(item->getItem()))
            return InteractionResult::eInteractionResult_Pass;

        int savedCount = item->getCount();
        item->use(level, player, hand);

        if (item->getCount() != savedCount)
            player->setItemInHand(hand, item);

        return InteractionResult::eInteractionResult_Success;
    }

    bool canUse = item->TestUse(level, player, hand);
    return canUse ? InteractionResult::eInteractionResult_Success : InteractionResult::eInteractionResult_Pass;
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
bool MultiPlayerGameMode::_destroyBlock(const BlockPos&) { mDestroying = false; return true; }
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
    if (!isCreative() || !mConnection)
        return;
    mConnection->send(boost::shared_ptr<Packet>(
        new ServerboundSetCreativeModeSlotPacket(slot, item)));
}

void MultiPlayerGameMode::handleCreativeModeItemDrop(not_null_ptr<ItemInstance> item)
{
    if (!isCreative() || !mConnection || !item || item->isEmpty())
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
    if (!player || !mConnection)
        return;
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
bool MultiPlayerGameMode::CanSendInvite(std::wstring, PlayerUID) { return !isSpectator(); }
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
    if (isSpectator() || !mConnection)
        return;
    boost::shared_ptr<Player> player = GetLocalPlayer();
    if (!player || !player->getTeam())
        return;
    GameModePacket::EmptyData data;
    mConnection->send(GameModePacket::Create(static_cast<GameModePacket::EMessage>(0x10), data));
}
bool MultiPlayerGameMode::RecordLastCheckpoint(int checkpoint, int, bool, bool, bool)
{
    boost::shared_ptr<Player> player = GetPlayer();
    return mMinecraft->GetClientMasterGameMode()->SetLastCheckpointID(player->getSUID(), checkpoint, false);
}
