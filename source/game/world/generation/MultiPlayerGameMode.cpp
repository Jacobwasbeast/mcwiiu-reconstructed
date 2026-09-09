#include "game/world/generation/MultiPlayerGameMode.hpp"

#include "game/core/Minecraft.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/Inventory.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/TeamInvite.hpp"
#include "game/core/System.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/ServerboundPlayerActionPacket.hpp"
#include "game/network/ServerboundSetCarriedItemPacket.hpp"
#include "game/core/Direction.hpp"

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
    boost::shared_ptr<Player>, Level*, const BlockPos&, const Direction*, Vec3*,
    InteractionHand::EInteractionHand, bool, bool*)
{
    return InteractionResult::eInteractionResult_Pass;
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
    boost::shared_ptr<Player>, boost::shared_ptr<Entity>, not_null_ptr<ItemInstance>,
    InteractionHand::EInteractionHand)
{
    return InteractionResult::eInteractionResult_Pass;
}

InteractionResult::EInteractionResult MultiPlayerGameMode::interactAt(
    boost::shared_ptr<Player>, boost::shared_ptr<Entity>, const HitResult*,
    not_null_ptr<ItemInstance>, InteractionHand::EInteractionHand)
{
    return InteractionResult::eInteractionResult_Pass;
}
bool MultiPlayerGameMode::hasInfiniteItems() { return isCreative(); }
void MultiPlayerGameMode::handlePickItem(const boost::shared_ptr<ItemInstance>&) {}
bool MultiPlayerGameMode::isInputAllowed(int mapping) { return mGameModeConstraints == 0 || mGameModeConstraints->isInputAllowed(mapping); }
bool MultiPlayerGameMode::destroyBlock(const BlockPos& pos) { return _destroyBlock(pos); }
void MultiPlayerGameMode::creativeDestroyBlock(Minecraft*, MultiPlayerGameMode* mode, const BlockPos& pos, const Direction*) { if (mode != 0) mode->_destroyBlock(pos); }
bool MultiPlayerGameMode::sameDestroyTarget(const BlockPos& pos) { return mDestroyBlockPos == pos; }
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
bool MultiPlayerGameMode::isServerControlledInventory() { return false; }
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
void MultiPlayerGameMode::Follow(bool, bool) {}
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
InteractionResult::EInteractionResult MultiPlayerGameMode::useItem(boost::shared_ptr<Player>, Level*, InteractionHand::EInteractionHand, bool) { return InteractionResult::eInteractionResult_Pass; }
void MultiPlayerGameMode::SetGameType(const GameType* gameType) { mGameType = gameType; }
const GameType* MultiPlayerGameMode::GetGameType() { return mGameType; }
void MultiPlayerGameMode::setLocalMode(const GameType* gameType) { mGameType = gameType; }
bool MultiPlayerGameMode::_destroyBlock(const BlockPos&) { mDestroying = false; return true; }
double MultiPlayerGameMode::getPickRange() { return hasFarPickRange() ? 5.0 : 4.5; }
boost::shared_ptr<ItemInstance> MultiPlayerGameMode::handleTakeAllInvenetory(int, int, int, int, boost::shared_ptr<Player>) { return boost::shared_ptr<ItemInstance>(); }
boost::shared_ptr<ItemInstance> MultiPlayerGameMode::handleInventoryMouseClick(int, int, int, ClickType::eClickType, boost::shared_ptr<Player>) { return boost::shared_ptr<ItemInstance>(); }
void MultiPlayerGameMode::handleInventoryButtonClick(int, int) {}
void MultiPlayerGameMode::handleCreativeModeItemAdd(not_null_ptr<ItemInstance>, int) {}
void MultiPlayerGameMode::handleCreativeModeItemDrop(not_null_ptr<ItemInstance>) {}
bool MultiPlayerGameMode::hasExperience() { return false; }
bool MultiPlayerGameMode::hasFarPickRange() { return isCreative(); }
boost::shared_ptr<ItemInstance> MultiPlayerGameMode::handleCraftItem(int, int, boost::shared_ptr<Player>) { return boost::shared_ptr<ItemInstance>(); }
boost::shared_ptr<ItemInstance> MultiPlayerGameMode::handleStackedCraftItem(std::vector<std::pair<int, int> >&, int, boost::shared_ptr<Player>) { return boost::shared_ptr<ItemInstance>(); }
boost::shared_ptr<ItemInstance> MultiPlayerGameMode::handleCustomCraftItem(std::vector<int>&, boost::shared_ptr<Player>) { return boost::shared_ptr<ItemInstance>(); }
void MultiPlayerGameMode::handleDebugOptions(unsigned int, boost::shared_ptr<Player>) {}
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
void MultiPlayerGameMode::SendTeamInvite(std::wstring) {}
std::vector<TeamInvite*>::iterator MultiPlayerGameMode::FindTeamInviteTo(std::wstring playerName)
{
    std::vector<TeamInvite*>::iterator it = mTeamInvites.begin();
    for (; it != mTeamInvites.end(); ++it)
        if (*it != 0 && (*it)->m_sToPlayerName == playerName) break;
    return it;
}
bool MultiPlayerGameMode::CanSendInvite(std::wstring, PlayerUID) { return !isSpectator(); }
bool MultiPlayerGameMode::CanLeaveTeam(PlayerUID) { return !isSpectator(); }
void MultiPlayerGameMode::AcceptTeamInvite(std::wstring) {}
void MultiPlayerGameMode::LeaveTeam() {}
bool MultiPlayerGameMode::RecordLastCheckpoint(int, int, bool, bool, bool) { return false; }
