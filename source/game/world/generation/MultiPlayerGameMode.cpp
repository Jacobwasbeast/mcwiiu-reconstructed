#include "game/world/generation/MultiPlayerGameMode.hpp"

#include "game/core/Minecraft.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/TeamInvite.hpp"

MultiPlayerGameMode::MultiPlayerGameMode(int playerId, ClientPacketListener* connection, int,
                                         bool, bool)
    : GameMode(), mDestroyBlockPos(), mDestroyProgress(0.0f), mDestroyTicks(0),
      mConnection(connection), mPlayerId(playerId), mGameModeConstraints(new GameModeConstraints(playerId)),
      mTutorial(0), mTeamInvites(), mMissTime(0.0f), mTutorialsEnabled(false), mDestroying(false)
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
boost::shared_ptr<Player> MultiPlayerGameMode::createPlayer(Level*) { return boost::shared_ptr<Player>(); }
void MultiPlayerGameMode::initPlayer(boost::shared_ptr<Player>) {}
void MultiPlayerGameMode::adjustPlayer(boost::shared_ptr<Player>) {}
void MultiPlayerGameMode::ensureHasSentCarriedItem() {}
bool MultiPlayerGameMode::_useItemOn(boost::shared_ptr<Player>, Level*, const BlockPos&, const Direction*, Vec3*, InteractionHand::EInteractionHand, bool, bool*) { return false; }
bool MultiPlayerGameMode::GetTutorialsEnabled() { return mTutorialsEnabled; }
InteractionResult::EInteractionResult MultiPlayerGameMode::useItemOn(
    boost::shared_ptr<Player>, Level*, const BlockPos&, const Direction*, Vec3*,
    InteractionHand::EInteractionHand, bool, bool*)
{
    return InteractionResult::eInteractionResult_Pass;
}
bool MultiPlayerGameMode::hasMissTime() { return mMissTime > 0.0f; }
void MultiPlayerGameMode::attack(boost::shared_ptr<Player>, boost::shared_ptr<Entity>) {}
InteractionResult::EInteractionResult MultiPlayerGameMode::interact(boost::shared_ptr<Player>, boost::shared_ptr<Entity>, not_null_ptr<ItemInstance>, InteractionHand::EInteractionHand) { return InteractionResult::eInteractionResult_Pass; }
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
void MultiPlayerGameMode::releaseUsingItem(boost::shared_ptr<Player>) {}
bool MultiPlayerGameMode::isServerControlledInventory() { return false; }
boost::shared_ptr<Player> MultiPlayerGameMode::GetLocalPlayer() { return GetPlayer(); }
bool MultiPlayerGameMode::SpectateSoundEnabled() { return false; }
double MultiPlayerGameMode::CalcSpectateSoundAllowance() { return 0.0; }
bool MultiPlayerGameMode::CanMakeSpectateSound() { return SpectateSoundEnabled(); }
void MultiPlayerGameMode::MakeSpectateSound() {}
bool MultiPlayerGameMode::isCutScene() { return false; }
void MultiPlayerGameMode::Follow(bool, bool) {}
void MultiPlayerGameMode::StartSpectating() {}
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
Level* MultiPlayerGameMode::GetLevel() { return 0; }
boost::shared_ptr<Player> MultiPlayerGameMode::GetPlayer() { return boost::shared_ptr<Player>(); }
void MultiPlayerGameMode::OnMouseDown() {}

std::vector<TeamInvite*>::iterator MultiPlayerGameMode::FindTeamInvite(unsigned int teamId)
{
    std::vector<TeamInvite*>::iterator it = mTeamInvites.begin();
    for (; it != mTeamInvites.end(); ++it)
        if (*it != 0 && (*it)->m_uTeamID == teamId) break;
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
