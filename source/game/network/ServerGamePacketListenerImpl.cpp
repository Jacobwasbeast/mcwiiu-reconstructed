#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/core/System.hpp"
#include "game/core/Socket.hpp"
#include "game/item/Inventory.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Vec3.hpp"
#include "game/network/ServerboundPlayerInputPacket.hpp"
#include "game/network/ServerboundMoveVehiclePacket.hpp"
#include "game/network/ServerboundAcceptTeleportationPacket.hpp"
#include "game/network/ServerboundSwingPacket.hpp"
#include "game/network/ServerboundSetCarriedItemPacket.hpp"
#include "game/network/ServerboundContainerClosePacket.hpp"
#include "game/network/ClientboundPlayerPositionPacket.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PlayerInfoPacket.hpp"
#include "game/network/DebugOptionsPacket.hpp"
#include "game/network/KickPlayerPacket.hpp"
#include "game/network/PlayerReadyPacket.hpp"
#include "game/network/ServerboundKeepAlivePacket.hpp"
#include "game/network/ServerboundPaddleBoatPacket.hpp"
#include "game/network/ServerboundPlayerAbilitiesPacket.hpp"
#include "game/network/ServerboundSetCarriedItemPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Boat.hpp"
#include "game/core/Voteable.hpp"
#include "game/core/Voteables.hpp"
#include "game/command/CommandDispatcher.hpp"
#include "game/command/CommandSender.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/VotePacket.hpp"
#include "game/network/ServerboundContainerButtonClickPacket.hpp"
#include "game/ui/AbstractContainerMenu.hpp"

ServerGamePacketListenerImpl::ServerGamePacketListenerImpl(
    MinecraftServer* server, Connection* connection,
    boost::shared_ptr<ServerPlayer> player)
    : mServer(server), mConnection(connection), mPlayer(player),
      mDisconnected(false), mKeepAliveTime(0), mKeepAliveId(0), mTeleportId(0),
      mPendingTeleportPosition(0), mLastX(0.0), mLastY(0.0), mLastZ(0.0),
      mLastYRot(0.0f), mLastXRot(0.0f), mPlayerUID(0), mOldPlayerUID(0)
{
    if (mConnection != 0)
        mConnection->setListener(this);
}

ServerGamePacketListenerImpl::~ServerGamePacketListenerImpl()
{
    delete mPendingTeleportPosition;
}

void ServerGamePacketListenerImpl::send(boost::shared_ptr<Packet> packet)
{
    if (mConnection->m_socket != 0 &&
        (mServer->getPlayers()->canReceiveAllPackets(mPlayer) || Packet::canSendToAnyClient(packet)))
        mConnection->send(packet);
}

INetworkPlayer* ServerGamePacketListenerImpl::getNetworkPlayer()
{
    if (mConnection != 0 && mConnection->m_socket != 0)
        return mConnection->m_socket->getPlayer();
    return 0;
}

void ServerGamePacketListenerImpl::info(const std::wstring& message)
{
}

void ServerGamePacketListenerImpl::warn(const std::wstring& message)
{
}

std::wstring ServerGamePacketListenerImpl::getConsoleName()
{
    return mPlayer->getName();
}

void ServerGamePacketListenerImpl::disconnect(DisconnectPacket::eDisconnectReason reason)
{
    (void)reason;
    mDisconnected = true;
    if (mConnection != 0)
        mConnection->close(DisconnectPacket::eDisconnectReason_Quit);
}

void ServerGamePacketListenerImpl::setShowOnMaps(bool show)
{
    mPlayer->mShowOnMaps = show;
}

bool ServerGamePacketListenerImpl::isLocal()
{
    return mConnection->m_socket != 0 && mConnection->m_socket->mLocal;
}

void ServerGamePacketListenerImpl::countDelayedPackets()
{
    if (mConnection != 0)
        mConnection->countDelayedPackets();
}

void ServerGamePacketListenerImpl::queueSend(boost::shared_ptr<Packet> packet)
{
    if (mConnection->m_socket != 0 &&
        (mServer->getPlayers()->canReceiveAllPackets(mPlayer) || Packet::canSendToAnyClient(packet)))
        mConnection->queueSend(packet);
}

void ServerGamePacketListenerImpl::resetPosition()
{
    mLastX = mPlayer->mX;
    mLastY = mPlayer->mY;
    mLastZ = mPlayer->mZ;
}

void ServerGamePacketListenerImpl::teleport(
    double x, double y, double z, float yRot, float xRot,
    const boost::unordered_set<const ClientboundPlayerPositionPacket::RelativeArgument*>& relativeArguments,
    bool dismount, bool retainPassenger)
{
    (void)dismount;
    (void)retainPassenger;
    delete mPendingTeleportPosition;
    mPendingTeleportPosition = new Vec3(x, y, z);
    ++mTeleportId;
    send(boost::shared_ptr<Packet>(new ClientboundPlayerPositionPacket(
        x, y, z, yRot, xRot, relativeArguments, mTeleportId)));
}

void ServerGamePacketListenerImpl::teleport(
    double x, double y, double z, float yRot, float xRot,
    bool dismount, bool retainPassenger)
{
    boost::unordered_set<const ClientboundPlayerPositionPacket::RelativeArgument*> relativeArguments;
    teleport(x, y, z, yRot, xRot, relativeArguments, dismount, retainPassenger);
}

void ServerGamePacketListenerImpl::tick()
{
}

bool ServerGamePacketListenerImpl::containsInvalidValues(boost::shared_ptr<ServerboundMoveVehiclePacket> packet)
{
    if (packet)
    {
        packet->getX();
        packet->getY();
        packet->getZ();
        packet->getXRot();
        packet->getYRot();
    }
    return false;
}

void ServerGamePacketListenerImpl::handlePlayerInput(boost::shared_ptr<ServerboundPlayerInputPacket> packet)
{
    if (packet && mPlayer)
        mPlayer->setPlayerInput(packet->getXxa(), packet->getZza(), packet->isJumping(), packet->isSneaking());
}
void ServerGamePacketListenerImpl::handleMoveVehicle(boost::shared_ptr<ServerboundMoveVehiclePacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleAcceptTeleportPacket(boost::shared_ptr<ServerboundAcceptTeleportationPacket> packet)
{
    if (packet && packet->getId() == mTeleportId && mPendingTeleportPosition != 0 && mPlayer)
    {
        mPlayer->absMoveTo(mPendingTeleportPosition->x, mPendingTeleportPosition->y,
                           mPendingTeleportPosition->z, mPlayer->mYRot, mPlayer->mXRot);
        mPendingTeleportPosition = 0;
        if (mPlayer->isChangingDimension())
            mPlayer->hasChangedDimension();
    }
}
void ServerGamePacketListenerImpl::handleMovePlayer(boost::shared_ptr<ServerboundMovePlayerPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handlePlayerAction(boost::shared_ptr<ServerboundPlayerActionPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleUseItemOn(boost::shared_ptr<ServerboundUseItemOnPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleUseItem(boost::shared_ptr<ServerboundUseItemPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleTeleportToEntityPacket(boost::shared_ptr<ServerboundTeleportToEntityPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handlePaddleBoat(boost::shared_ptr<ServerboundPaddleBoatPacket> packet)
{
    boost::shared_ptr<Entity> vehicle = mPlayer->getVehicle();
    if (vehicle && vehicle->GetType() == 0x8000008ULL)
        boost::static_pointer_cast<Boat>(vehicle)->setPaddleState(packet->getLeft(), packet->getRight());
}

void ServerGamePacketListenerImpl::handleSetCarriedItem(boost::shared_ptr<ServerboundSetCarriedItemPacket> packet)
{
    int slot = packet->getSlot();
    if (slot >= 0 && slot < Inventory::getSelectionSize())
    {
        mPlayer->mField0x5f0->m_selected = slot;
        mPlayer->resetLastActionTime();
    }
}
void ServerGamePacketListenerImpl::handleChat(boost::shared_ptr<ClientboundChatPacket> packet) {}
void ServerGamePacketListenerImpl::handleAnimate(boost::shared_ptr<ServerboundSwingPacket> packet)
{
    if (mPlayer)
    {
        mPlayer->resetLastActionTime();
        if (packet)
            mPlayer->swing((uint32_t)packet->getHand());
    }
}
void ServerGamePacketListenerImpl::handlePlayerCommand(boost::shared_ptr<ServerboundPlayerCommandPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleInteract(boost::shared_ptr<ServerboundInteractPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleClientCommand(boost::shared_ptr<ServerboundClientCommandPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleContainerClose(boost::shared_ptr<ServerboundContainerClosePacket> packet)
{
    mPlayer->doCloseContainer();
}
void ServerGamePacketListenerImpl::handleContainerClick(boost::shared_ptr<ServerboundContainerClickPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleContainerButtonClick(boost::shared_ptr<ServerboundContainerButtonClickPacket> packet)
{
    mPlayer->resetLastActionTime();
    if (mPlayer->mContainerMenu->m_containerId == packet->getContainerId() &&
        mPlayer->mContainerMenu->isSynched(mPlayer) && !mPlayer->isSpectator())
    {
        mPlayer->mContainerMenu->clickMenuButton(mPlayer, packet->getButtonId());
        mPlayer->mContainerMenu->broadcastChanges();
    }
}
void ServerGamePacketListenerImpl::handleSetCreativeModeSlot(boost::shared_ptr<ServerboundSetCreativeModeSlotPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleContainerAck(boost::shared_ptr<ServerboundContainerAckPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleSignUpdate(boost::shared_ptr<ServerboundSignUpdatePacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleKeepAlive(boost::shared_ptr<ServerboundKeepAlivePacket> packet)
{
    if (packet->id == mKeepAliveId)
    {
        int elapsed = (int)(getMilliseconds() - mKeepAliveTime);
        mPlayer->latency = (mPlayer->latency * 3 + elapsed) / 4;
    }
}
void ServerGamePacketListenerImpl::handlePlayerInfo(boost::shared_ptr<PlayerInfoPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handlePlayerAbilities(boost::shared_ptr<ServerboundPlayerAbilitiesPacket> packet)
{
    mPlayer->mAbilities.mFlying = packet->isFlying() &&
        (mPlayer->mAbilities.mMayfly || mPlayer->isAllowedToFly());
}
void ServerGamePacketListenerImpl::handleCustomPayload(boost::shared_ptr<ServerboundCustomPayloadPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleCraftItem(boost::shared_ptr<CraftItemPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleTradeItem(boost::shared_ptr<TradeItemPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleDebugOptions(boost::shared_ptr<DebugOptionsPacket> packet)
{
    mPlayer->mDebugOptions = packet->options;
}
void ServerGamePacketListenerImpl::handleTexture(boost::shared_ptr<TexturePacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleTextureAndGeometry(boost::shared_ptr<TextureAndGeometryPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleTextureChange(boost::shared_ptr<TextureChangePacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleTextureAndGeometryChange(boost::shared_ptr<TextureAndGeometryChangePacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleServerSettingsChanged(boost::shared_ptr<ServerSettingsChangedPacket> packet) { (void)packet; }
void ServerGamePacketListenerImpl::handleKickPlayer(boost::shared_ptr<KickPlayerPacket> packet)
{
    INetworkPlayer* networkPlayer = getNetworkPlayer();
    if ((networkPlayer != 0 && networkPlayer->IsHost()) || mPlayer->isModerator())
        mServer->getPlayers()->kickPlayerByShortId(packet->reason);
}
void ServerGamePacketListenerImpl::handleGameCommand(boost::shared_ptr<GameCommandPacket> packet)
{
    boost::shared_ptr<CommandSender> sender = boost::static_pointer_cast<CommandSender>(mPlayer);
    mServer->getCommandDispatcher()->performCommand(sender, packet->command, packet->data);
}

void ServerGamePacketListenerImpl::handleVote(boost::shared_ptr<VotePacket> packet)
{
    if (packet->voteableId >= -1 && packet->voteableId < 5)
    {
        Voteable* voteable = mServer->mMasterGameMode->mVoteables->getVoteable(packet->voteableId);
        if (voteable != 0)
        {
            voteable->getVote(packet->playerId, packet->voteId);
            voteable->setVoteAndSync(packet->playerId, packet->voteId, packet->vote);
            if (packet->voteableId == 1)
                mServer->mMasterGameMode->OnKickReqested(packet->playerId, packet->voteId);
        }
    }
}
void ServerGamePacketListenerImpl::handlePlayerReady(boost::shared_ptr<PlayerReadyPacket> packet)
{
    mServer->mMasterGameMode->SetPlayerReady(packet->mPlayerId, packet->mReady);
}
void ServerGamePacketListenerImpl::handleDisconnect(boost::shared_ptr<DisconnectPacket> packet)
{
    mServer->getPlayers()->removePlayerFromReceiving(mPlayer, true, 0);
    mConnection->close(DisconnectPacket::eDisconnectReason_Quit);
}
void ServerGamePacketListenerImpl::handleGameMode(boost::shared_ptr<GameModePacket> packet)
{
    switch (packet->GetMessage())
    {
    case 0:
    {
        GameModePacket::FollowData data;
        packet->ReadPacket(data);
        mServer->mMasterGameMode->OnFollow(mPlayer, data.follow, data.teleport);
        break;
    }
    case 1:
        mServer->mMasterGameMode->OnStartSpectating(mPlayer);
        break;
    case 6:
        mServer->mMasterGameMode->PlayerFinishedLoadingLocally();
        break;
    }
}
void ServerGamePacketListenerImpl::handleClientboundSetPlayerTeamPacket(boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet) {}
void ServerGamePacketListenerImpl::handleSoundEvent(boost::shared_ptr<ClientboundSoundPacket> packet)
{
    mServer->getPlayers()->broadcast(mPlayer, packet->getX(), packet->getY(), packet->getZ(),
                                     30.0, mPlayer->mDimension, packet);
}

void ServerGamePacketListenerImpl::onDisconnect(
    DisconnectPacket::eDisconnectReason reason, void* userData)
{
    (void)reason;
    (void)userData;
    mDisconnected = true;
}

uint64_t ServerGamePacketListenerImpl::getMilliseconds()
{
    return System::processTimeInMilliSecs();
}

void ServerGamePacketListenerImpl::NameItemOnAnvil(
    const boost::shared_ptr<ServerboundCustomPayloadPacket>& packet, bool useName)
{
    (void)packet;
    (void)useName;
}

void ServerGamePacketListenerImpl::handleTextureReceived(const std::wstring& name)
{
    (void)name;
}

void ServerGamePacketListenerImpl::handleTextureAndGeometryReceived(const std::wstring& name)
{
    (void)name;
}

bool ServerGamePacketListenerImpl::isDisconnected()
{
    return mDisconnected;
}

bool ServerGamePacketListenerImpl::isGuest()
{
    INetworkPlayer* networkPlayer = getNetworkPlayer();
    return networkPlayer != 0 && networkPlayer->IsGuest();
}
