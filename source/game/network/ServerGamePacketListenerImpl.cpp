#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/core/EntityType.hpp"
#include <algorithm>
#include "game/core/System.hpp"
#include "game/core/Socket.hpp"
#include "game/item/Inventory.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/util/Vec3.hpp"
#include "game/network/ServerboundPlayerInputPacket.hpp"
#include "game/network/ServerboundMoveVehiclePacket.hpp"
#include "game/network/ServerboundAcceptTeleportationPacket.hpp"
#include "game/network/ServerboundSwingPacket.hpp"
#include "game/network/ServerboundUseItemPacket.hpp"
#include "game/network/ServerboundSetCarriedItemPacket.hpp"
#include "game/network/ServerboundContainerClosePacket.hpp"
#include "game/network/ClientboundPlayerPositionPacket.hpp"
#include "game/network/ClientboundRespawnPacket.hpp"
#include "game/network/ClientboundBlockUpdatePacket.hpp"
#include "game/network/ClientboundMoveVehiclePacket.hpp"
#include "game/network/ClientboundContainerAckPacket.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/CGameNetworkManager.hpp"
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
#include "game/server/ServerPlayerGameMode.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/entity/mob/MoverType.hpp"
#include "game/util/AABB.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/core/Boat.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DLCSkinFile.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/Voteable.hpp"
#include "game/core/Voteables.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Merchant.hpp"
#include "game/core/Slot.hpp"
#include "game/command/CommandDispatcher.hpp"
#include "game/command/CommandSender.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/Horse.hpp"
#include "game/item/ElytraItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/MerchantRecipe.hpp"
#include "game/item/MerchantRecipeList.hpp"
#include "game/item/WritableBookItem.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/StringTag.hpp"
#include "game/item/WrittenBookItem.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/CraftItemPacket.hpp"
#include "game/network/VotePacket.hpp"
#include "game/network/ServerboundContainerButtonClickPacket.hpp"
#include "game/network/ServerboundContainerClickPacket.hpp"
#include "game/network/ServerboundContainerAckPacket.hpp"
#include "game/network/ServerboundClientCommandPacket.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/network/ServerboundInteractPacket.hpp"
#include "game/network/ServerboundPlayerCommandPacket.hpp"
#include "game/network/ServerboundSetCreativeModeSlotPacket.hpp"
#include "game/network/ServerboundPlayerActionPacket.hpp"
#include "game/network/ServerboundMovePlayerPacket.hpp"
#include "game/network/ServerboundUseItemOnPacket.hpp"
#include "game/network/ServerboundSignUpdatePacket.hpp"
#include "game/network/ServerboundTeleportToEntityPacket.hpp"
#include "game/network/ServerConnection.hpp"
#include "game/network/TexturePacket.hpp"
#include "game/network/TextureAndGeometryPacket.hpp"
#include "game/network/TextureChangePacket.hpp"
#include "game/network/TextureAndGeometryChangePacket.hpp"
#include "game/network/TradeItemPacket.hpp"
#include "game/network/ServerSettingsChangedPacket.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/AnvilMenu.hpp"
#include "game/ui/BeaconMenu.hpp"
#include "game/ui/MerchantMenu.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/SignBlockEntity.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/server/ServerLevel.hpp"

ServerGamePacketListenerImpl::ServerGamePacketListenerImpl(
    MinecraftServer* server, Connection* connection,
    boost::shared_ptr<ServerPlayer> player)
    : mConnection(connection), mDisconnected(false), mPlayerUID(0),
      mOldPlayerUID(0), mServer(server), mTickCount(0),
      mKeepAliveId(0), mKeepAliveTime(0), mKeepAlivePending(0), mChatSpamTickCount(0),
      mDropSpamTickCount(0), mFirstGoodX(0.0), mFirstGoodY(0.0),
      mFirstGoodZ(0.0), mLastGoodX(0.0), mLastGoodY(0.0), mLastGoodZ(0.0),
      mVehicleFirstGoodX(0.0), mVehicleFirstGoodY(0.0), mVehicleFirstGoodZ(0.0),
      mVehicleLastGoodX(0.0), mVehicleLastGoodY(0.0), mVehicleLastGoodZ(0.0),
      mPendingTeleportPosition(NULL), mTeleportId(0), mTeleportTime(0),
      mClientIsFloating(false), mAboveGroundTickCount(0),
      mClientVehicleIsFloating(false), mAboveGroundVehicleTickCount(0),
      mReceivedMovePacketCount(0), mKnownMovePacketCount(0)
{
    connection->setListener(this);
    mPlayer = player;
    InitializeCriticalSection(&mDisconnectMutex);
    mConnectionClosed = false;
    mFriendsOnly = false;
    mUseAltLeaveMessage = false;
    mPlayerUID = PlayerUID(0);
    mOldPlayerUID = PlayerUID(0);
    setShowOnMaps(g_pApp->GetGameHostOption(eGameHostOption_Gamertags) != 0);
}

ServerGamePacketListenerImpl::~ServerGamePacketListenerImpl()
{
    delete mPendingTeleportPosition;
    delete mConnection;
    DeleteCriticalSection(&mDisconnectMutex);
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
    EnterCriticalSection(&mDisconnectMutex);
    if (!mDisconnected)
    {
        mPlayer->disconnect();
        mServer->getPlayers()->removePlayerFromReceiving(mPlayer, true, 0);
        send(boost::shared_ptr<Packet>(new DisconnectPacket(reason)));
        mConnection->sendAndQuit();
        ClientboundChatPacket::EChatPacketMessage message = mUseAltLeaveMessage
            ? ClientboundChatPacket::eChatPacketMessage_PlayerKicked
            : ClientboundChatPacket::eChatPacketMessage_PlayerLeft;
        mServer->getPlayers()->broadcastAll(
            boost::shared_ptr<Packet>(new ClientboundChatPacket(mPlayer->getName(), message, -1)));
        mServer->getPlayers()->remove(mPlayer);
        mDisconnected = true;
    }
    LeaveCriticalSection(&mDisconnectMutex);
}

void ServerGamePacketListenerImpl::setShowOnMaps(bool show)
{
    mPlayer->mShowOnMaps = show;
}

bool ServerGamePacketListenerImpl::isLocal()
{
    return mConnection->m_socket != 0 && mConnection->m_socket->mLocal;
}

uint32_t ServerGamePacketListenerImpl::countDelayedPackets()
{
    return mConnection->countDelayedPackets();
}

void ServerGamePacketListenerImpl::queueSend(boost::shared_ptr<Packet> packet)
{
    if (mConnection->m_socket != 0 &&
        (mServer->getPlayers()->canReceiveAllPackets(mPlayer) || Packet::canSendToAnyClient(packet)))
        mConnection->queueSend(packet);
}

void ServerGamePacketListenerImpl::resetPosition()
{
    mFirstGoodX = mPlayer->mX;
    mFirstGoodY = mPlayer->mY;
    mFirstGoodZ = mPlayer->mZ;
    mLastGoodX = mPlayer->mX;
    mLastGoodY = mPlayer->mY;
    mLastGoodZ = mPlayer->mZ;
}

void ServerGamePacketListenerImpl::teleport(
    double x, double y, double z, float yRot, float xRot,
    const boost::unordered_set<const ClientboundPlayerPositionPacket::RelativeArgument*>& relativeArguments,
    bool sendPacket, bool includeRotation)
{
    delete mPendingTeleportPosition;
    mPendingTeleportPosition = Vec3::newPermanent(x, y, z);
    ++mTeleportId;
    if (mTeleportId == 0x7fffffff)
        mTeleportId = 0;
    mTeleportTime = mTickCount;

    if (includeRotation)
        mPlayer->absMoveTo(x, y, z, yRot, xRot);
    else
        mPlayer->absMoveTo(x, y, z);

    if (sendPacket)
    {
        if (includeRotation)
        {
            send(boost::shared_ptr<Packet>(new ClientboundPlayerPositionPacket(
                x, y, z, yRot, xRot, relativeArguments, mTeleportId)));
        }
        else
        {
            send(boost::shared_ptr<Packet>(new ClientboundPlayerPositionPacket(
                x, y, z, relativeArguments, mTeleportId)));
        }
    }
    resetPosition();
}

void ServerGamePacketListenerImpl::teleport(
    double x, double y, double z, float yRot, float xRot,
    bool sendPacket, bool includeRotation)
{
    boost::unordered_set<const ClientboundPlayerPositionPacket::RelativeArgument*> relativeArguments;
    teleport(x, y, z, yRot, xRot, relativeArguments, sendPacket, includeRotation);
}

void ServerGamePacketListenerImpl::tick()
{
    if (mDisconnected)
        return;
    if (mConnectionClosed)
    {
        disconnect(DisconnectPacket::eDisconnectReason_ConnectionClosed);
        return;
    }

    resetPosition();
    mPlayer->doTick(false, false, false);
    mPlayer->absMoveTo(mFirstGoodX, mFirstGoodY, mFirstGoodZ,
                       mPlayer->mYRot, mPlayer->mXRot);
    ++mTickCount;
    mKnownMovePacketCount = mReceivedMovePacketCount;

    if (mClientIsFloating)
    {
        ++mAboveGroundTickCount;
        if (mAboveGroundTickCount > 80)
        {
            disconnect(DisconnectPacket::eDisconnectReason_KeepAliveTimeout);
            return;
        }
    }
    else
    {
        mAboveGroundTickCount = 0;
        mClientIsFloating = false;
    }

    boost::shared_ptr<Entity> vehicle = mPlayer->getRootVehicle();
    mLastVehicle = vehicle;
    if (!vehicle || vehicle == mPlayer || vehicle->getControllingPassenger() != mPlayer)
    {
        mAboveGroundVehicleTickCount = 0;
        mClientVehicleIsFloating = false;
    }
    else
    {
        mVehicleFirstGoodX = vehicle->mX;
        mVehicleFirstGoodY = vehicle->mY;
        mVehicleFirstGoodZ = vehicle->mZ;
        mVehicleLastGoodX = vehicle->mX;
        mVehicleLastGoodY = vehicle->mY;
        mVehicleLastGoodZ = vehicle->mZ;

        bool stillControllingVehicle = false;
        if (mClientVehicleIsFloating)
        {
            boost::shared_ptr<Entity> checkVehicle = mPlayer->getRootVehicle();
            if (checkVehicle->getControllingPassenger() == mPlayer)
                stillControllingVehicle = true;
        }

        if (stillControllingVehicle)
        {
            ++mAboveGroundVehicleTickCount;
            if (mAboveGroundVehicleTickCount > 80)
            {
                disconnect(DisconnectPacket::eDisconnectReason_KeepAliveTimeout);
                return;
            }
        }
        else
        {
            mAboveGroundVehicleTickCount = 0;
            mClientVehicleIsFloating = false;
        }
    }

    mConnection->tick();

    if (!mDisconnected)
    {
        if (mChatSpamTickCount > 0)
            --mChatSpamTickCount;
        if (mDropSpamTickCount > 0)
            --mDropSpamTickCount;
    }
}

bool ServerGamePacketListenerImpl::containsInvalidValues(boost::shared_ptr<ServerboundMoveVehiclePacket> packet)
{
    packet->getX();
    packet->getY();
    packet->getZ();
    packet->getXRot();
    packet->getYRot();
    return false;
}

void ServerGamePacketListenerImpl::handlePlayerInput(boost::shared_ptr<ServerboundPlayerInputPacket> packet)
{
    mPlayer->setPlayerInput(packet->getXxa(), packet->getZza(), packet->isJumping(), packet->isSneaking());
}
void ServerGamePacketListenerImpl::handleMoveVehicle(boost::shared_ptr<ServerboundMoveVehiclePacket> packet)
{
    if (containsInvalidValues(packet))
    {
        disconnect(DisconnectPacket::eDisconnectReason_InvalidMovement);
        return;
    }

    boost::shared_ptr<Entity> vehicle = mPlayer->getRootVehicle();
    if (vehicle == mPlayer || vehicle->getControllingPassenger() != mPlayer || vehicle != mLastVehicle.lock())
        return;

    ServerLevel* level = mPlayer->getLevel();
    double oldX = vehicle->mX;
    double oldY = vehicle->mY;
    double oldZ = vehicle->mZ;
    double x = packet->getX();
    double y = packet->getY();
    double z = packet->getZ();
    float yRot = packet->getYRot();
    float xRot = packet->getXRot();

    double dx = x - mVehicleFirstGoodX;
    double dy = y - mVehicleFirstGoodY;
    double dz = z - mVehicleFirstGoodZ;
    double expected = vehicle->mXd * vehicle->mXd + vehicle->mYd * vehicle->mYd + vehicle->mZd * vehicle->mZd;
    if (dx * dx + dy * dy + dz * dz - expected > 100.0)
    {
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundMoveVehiclePacket(vehicle)));
        return;
    }

    bool wasFree = level->getCollisionAABBs(vehicle, vehicle->mBoundingBox->shrink(0.0625), false, false, false)->empty();
    dx = x - mVehicleLastGoodX;
    dy = y - mVehicleLastGoodY - 1.0E-6;
    dz = z - mVehicleLastGoodZ;
    vehicle->move(eMoverType_Player, dx, dy, dz, false);
    double movedY = dy;

    dx = x - vehicle->mX;
    dy = y - vehicle->mY;
    if (dy > -0.5 || dy < 0.5)
        dy = 0.0;
    dz = z - vehicle->mZ;
    bool movedWrongly = dx * dx + dy * dy + dz * dz > 0.0625;
    vehicle->absMoveTo(x, y, z, yRot, xRot);
    bool isFree = level->getCollisionAABBs(vehicle, vehicle->mBoundingBox->shrink(0.0625), false, false, false)->empty();
    if (wasFree && (movedWrongly || !isFree))
    {
        vehicle->absMoveTo(oldX, oldY, oldZ, yRot, xRot);
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundMoveVehiclePacket(vehicle)));
        return;
    }

    PlayerList* players = gMinecraftServer != NULL ? gMinecraftServer->getPlayers() : NULL;
    players->move(mPlayer);
    mPlayer->checkMovementStatistiscs(mPlayer->mX - oldX, mPlayer->mY - oldY, mPlayer->mZ - oldZ);
    mClientVehicleIsFloating = movedY >= -0.03125 && !mServer->isFlightAllowed()
                            && !level->containsAnyBlocks(vehicle->mBoundingBox->grow(0.0625)->expand(0.0, -0.55, 0.0));
    mVehicleLastGoodX = vehicle->mX;
    mVehicleLastGoodY = vehicle->mY;
    mVehicleLastGoodZ = vehicle->mZ;
}
void ServerGamePacketListenerImpl::handleAcceptTeleportPacket(boost::shared_ptr<ServerboundAcceptTeleportationPacket> packet)
{
    if (packet->getId() == mTeleportId)
    {
        mPlayer->absMoveTo(mPendingTeleportPosition->x, mPendingTeleportPosition->y,
                           mPendingTeleportPosition->z, mPlayer->mYRot, mPlayer->mXRot);
        if (mPlayer->isChangingDimension())
        {
            mLastGoodX = mPendingTeleportPosition->x;
            mLastGoodY = mPendingTeleportPosition->y;
            mLastGoodZ = mPendingTeleportPosition->z;
            mPlayer->hasChangedDimension();
        }
        delete mPendingTeleportPosition;
        mPendingTeleportPosition = 0;
    }
}
void ServerGamePacketListenerImpl::handleMovePlayer(boost::shared_ptr<ServerboundMovePlayerPacket> packet)
{
    ServerLevel* level = mServer->getLevel(mPlayer->mDimension);
    if (mPlayer->mRespawnState)
        return;

    if (mTickCount == 0)
        resetPosition();

    if (mPendingTeleportPosition != NULL)
    {
        if (mTickCount - mTeleportTime > TELEPORT_RESEND_TICKS)
        {
            mTeleportTime = mTickCount;
            teleport(mPendingTeleportPosition->x, mPendingTeleportPosition->y, mPendingTeleportPosition->z,
                     mPlayer->mYRot, mPlayer->mXRot, true, true);
        }
        return;
    }

    mTeleportTime = mTickCount;
    if (mPlayer->isPassenger())
    {
        mPlayer->absMoveTo(mPlayer->mX, mPlayer->mY, mPlayer->mZ, packet->getYRot(mPlayer->mYRot),
                           packet->getXRot(mPlayer->mXRot));
        PlayerList* players = gMinecraftServer != NULL ? gMinecraftServer->getPlayers() : NULL;
        players->move(mPlayer);
        return;
    }

    double startX = mPlayer->mX;
    double startY = mPlayer->mY;
    double startZ = mPlayer->mZ;
    double fallStartY = mPlayer->mY;
    double x = packet->getX(mPlayer->mX);
    double y = packet->getY(mPlayer->mY);
    double z = packet->getZ(mPlayer->mZ);
    float yRot = packet->getYRot(mPlayer->mYRot);
    float xRot = packet->getXRot(mPlayer->mXRot);

    double dx = x - mFirstGoodX;
    double dy = y - mFirstGoodY;
    double dz = z - mFirstGoodZ;
    if (mPlayer->isSleeping() && dx * dx + dy * dy + dz * dz > 1.0)
        teleport(mPlayer->mX, mPlayer->mZ, mPlayer->mZ, packet->getYRot(mPlayer->mYRot),
                 packet->getXRot(mPlayer->mXRot), true, true);

    ++mReceivedMovePacketCount;
    bool missedMovePackets = mReceivedMovePacketCount - mKnownMovePacketCount > 1;

    dx = x - mLastGoodX;
    dy = y - mLastGoodY;
    dz = z - mLastGoodZ;
    if (mPlayer->mOnGround && !packet->isOnGround() && dy > 0.0)
        mPlayer->jumpFromGround();
    mPlayer->move(eMoverType_Player, dx, dy, dz, false);
    mPlayer->mOnGround = packet->isOnGround();

    mPlayer->absMoveTo(x, y, z, yRot, xRot);
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        mPlayer->mYBodyRot = yRot;
    mPlayer->checkMovementStatistiscs(mPlayer->mX - startX, mPlayer->mY - startY, mPlayer->mZ - startZ);

    mClientIsFloating = dy >= -0.03125;
    mClientIsFloating &= !mServer->isFlightAllowed() && !mPlayer->mAbilities.mMayfly;
    mClientIsFloating &= !mPlayer->hasEffect(MobEffects::LEVITATION) && !mPlayer->isFallFlying()
                      && !level->containsAnyBlocks(mPlayer->mBoundingBox->grow(0.0625)->expand(0.0, -0.55, 0.0));
    mPlayer->mOnGround = packet->isOnGround();

    PlayerList* players = gMinecraftServer != NULL ? gMinecraftServer->getPlayers() : NULL;
    players->move(mPlayer);
    mPlayer->doCheckFallDamage(mPlayer->mY - fallStartY, packet->isOnGround());
    mLastGoodX = mPlayer->mX;
    mLastGoodY = mPlayer->mY;
    mLastGoodZ = mPlayer->mZ;

    if (missedMovePackets && Minecraft::GetInstance()->GetMiniGame()->HasFeatureCheckpoints())
    {
        MasterGameMode* masterGameMode = mServer->mMasterGameMode;
        double minX = std::min(startX, mPlayer->mX);
        double minY = std::min(startY, mPlayer->mY);
        double minZ = std::min(startZ, mPlayer->mZ);
        double maxX = std::max(startX, mPlayer->mX);
        double maxY = std::max(startY, mPlayer->mY);
        double maxZ = std::max(startZ, mPlayer->mZ);
        if (masterGameMode != NULL)
            masterGameMode->CatchUpCheckpointCheck(mPlayer, AABB::newTemp(minX, minY, minZ, maxX, maxY, maxZ));
    }
}
void ServerGamePacketListenerImpl::handlePlayerAction(boost::shared_ptr<ServerboundPlayerActionPacket> packet)
{
    ServerLevel* level = mServer->getLevel(mPlayer->mDimension);
    BlockPos pos = packet->getPos();
    mPlayer->resetLastActionTime();

    switch (packet->getAction())
    {
    case ServerboundPlayerActionPacket::SWAP_ITEM_WITH_OFFHAND:
        if (!mPlayer->isSpectator())
        {
            not_null_ptr<ItemInstance> offHand = mPlayer->getItemInHand(InteractionHand::eInteractionHand_OffHand);
            mPlayer->setItemInHand(InteractionHand::eInteractionHand_OffHand,
                                   mPlayer->getItemInHand(InteractionHand::eInteractionHand_MainHand));
            mPlayer->setItemInHand(InteractionHand::eInteractionHand_MainHand, offHand);
        }
        return;
    case ServerboundPlayerActionPacket::DROP_ITEM:
        if (!mPlayer->isSpectator())
            mPlayer->drop(false);
        return;
    case ServerboundPlayerActionPacket::DROP_ALL_ITEMS:
        if (!mPlayer->isSpectator())
            mPlayer->drop(true);
        return;
    case ServerboundPlayerActionPacket::RELEASE_USE_ITEM:
        mPlayer->releaseUsingItem(packet->getUseTime());
        return;
    case ServerboundPlayerActionPacket::START_DESTROY_BLOCK:
    case ServerboundPlayerActionPacket::ABORT_DESTROY_BLOCK:
    case ServerboundPlayerActionPacket::STOP_DESTROY_BLOCK:
        break;
    default:
        return;
    }

    double dx = mPlayer->mX - (pos.x + 0.5);
    double dy = mPlayer->mY - (pos.y + 0.5) + 1.5;
    double dz = mPlayer->mZ - (pos.z + 0.5);
    if (dx * dx + dy * dy + dz * dz > 36.0 || pos.y >= mServer->getMaxBuildHeight())
        return;

    if (packet->getAction() == ServerboundPlayerActionPacket::START_DESTROY_BLOCK)
    {
        if (level->getWorldBorder()->isWithinBounds(pos))
            mPlayer->mGameMode->startDestroyBlock(pos, packet->getDirection());
        else
            mPlayer->mConnection->send(boost::shared_ptr<Packet>(new ClientboundBlockUpdatePacket(level, pos)));
        return;
    }

    if (packet->getAction() == ServerboundPlayerActionPacket::STOP_DESTROY_BLOCK)
    {
        mPlayer->mGameMode->stopDestroyBlock(pos);
        mServer->getPlayers()->prioritiseBlockChanges(pos, level->mDimension->getType()->getId());
    }
    else if (packet->getAction() == ServerboundPlayerActionPacket::ABORT_DESTROY_BLOCK)
    {
        mPlayer->mGameMode->abortDestroyBlock();
    }
    if (level->getBlockState(pos)->getBlock()->mMaterial != Material::AIR)
        mPlayer->mConnection->send(boost::shared_ptr<Packet>(new ClientboundBlockUpdatePacket(level, pos)));
}
void ServerGamePacketListenerImpl::handleUseItemOn(boost::shared_ptr<ServerboundUseItemOnPacket> packet)
{
    ServerLevel* level = mServer->getLevel(mPlayer->mDimension);
    InteractionHand::EInteractionHand hand = packet->getHand();
    not_null_ptr<ItemInstance> item = mPlayer->getItemInHand(hand);
    BlockPos pos = packet->getPos();
    const Direction* face = packet->getFace();
    mPlayer->resetLastActionTime();

    bool belowBuildLimit = pos.y < mServer->getMaxBuildHeight() - 1
                        || (face != Direction::UP && pos.y < mServer->getMaxBuildHeight());
    if (belowBuildLimit && mPendingTeleportPosition == NULL
        && mPlayer->distanceToSqr(pos.x + 0.5, pos.y + 0.5, pos.z + 0.5) < 64.0
        && level->getWorldBorder()->isWithinBounds(pos))
    {
        bool result = false;
        mPlayer->mGameMode->useItemOn(mPlayer, level, item, hand, pos, face, packet->getClickX(),
                                      packet->getClickY(), packet->getClickZ(), false, &result);
    }

    mPlayer->mConnection->send(boost::shared_ptr<Packet>(new ClientboundBlockUpdatePacket(level, pos)));
    if (level->getBlockState(pos)->getBlock() != Blocks::piston_head)
        mPlayer->mConnection->send(
            boost::shared_ptr<Packet>(new ClientboundBlockUpdatePacket(level, pos.relative(face))));
}
void ServerGamePacketListenerImpl::handleUseItem(boost::shared_ptr<ServerboundUseItemPacket> packet)
{
    ServerLevel* level = mServer->getLevel(mPlayer->mDimension);
    InteractionHand::EInteractionHand hand = packet->getHand();
    not_null_ptr<ItemInstance> item = mPlayer->getItemInHand(hand);
    mPlayer->resetLastActionTime();
    if (!item->isEmpty())
        mPlayer->mGameMode->useItem(mPlayer, level, hand, false);
}
void ServerGamePacketListenerImpl::handleTeleportToEntityPacket(
    boost::shared_ptr<ServerboundTeleportToEntityPacket> packet)
{
    if (!mPlayer->isSpectator())
        return;

    for (unsigned int i = 0; i < mServer->mLevels.length; ++i)
    {
        ServerLevel* targetLevel = mServer->mLevels.data[i];
        if (targetLevel == 0)
            continue;

        boost::shared_ptr<Entity> target = packet->getEntity(targetLevel);
        if (!target)
            continue;

        mPlayer->SetCamera(mPlayer);
        mPlayer->stopRiding();

        if (target->mDimension == mPlayer->mDimension)
        {
            mPlayer->teleportTo(target->mX, target->mY, target->mZ);
            return;
        }

        ServerLevel* oldLevel = mPlayer->getLevel();
        mPlayer->mDimension = target->mDimension;
        LevelData* oldLevelData = oldLevel->getLevelData();
        int seaLevel = targetLevel->useNewSeaLevel() ? targetLevel->getSeaLevel() : -1;

        send(boost::shared_ptr<Packet>(new ClientboundRespawnPacket(
            (char)mPlayer->mDimension,
            oldLevelData->getSeed(),
            mPlayer->GetGameType(),
            oldLevel->getDifficulty(),
            oldLevelData->getGeneratorType(),
            targetLevel->useNewSeaLevel(),
            mPlayer->getId(),
            oldLevelData->getXZSize(),
            oldLevelData->getHellScale(),
            oldLevelData->getCloudHeight(),
            oldLevelData->getModernEnd(),
            (unsigned char)oldLevelData->getBiomeScale(),
            oldLevelData->getBiomeCentreXChunk(),
            oldLevelData->getBiomeCentreZChunk(),
            true,
            seaLevel)));

        oldLevel->removeEntityImmediately(mPlayer);
        mPlayer->mRemoved = false;
        mPlayer->teleportTo(target->mX, target->mY, target->mZ,
                            target->mYRot, target->mXRot);

        if (mPlayer->isAlive())
        {
            oldLevel->tick(mPlayer, false);
            targetLevel->addEntity(mPlayer);
            targetLevel->tick(mPlayer, false);
        }

        mPlayer->setLevel(targetLevel);
        mServer->getPlayers()->changeDimension(mPlayer, oldLevel);
        mPlayer->teleportTo(target->mX, target->mY, target->mZ);
        mPlayer->mGameMode->setLevel(targetLevel);
        mServer->getPlayers()->sendLevelInfo(mPlayer, targetLevel);
        mServer->getPlayers()->sendAllPlayerInfo(mPlayer);
        return;
    }
}
void ServerGamePacketListenerImpl::handlePaddleBoat(boost::shared_ptr<ServerboundPaddleBoatPacket> packet)
{
    boost::shared_ptr<Entity> vehicle = mPlayer->getVehicle();
    if (vehicle && vehicle->GetType() == EntityType::BOAT)
        boost::static_pointer_cast<Boat>(vehicle)->setPaddleState(packet->getLeft(), packet->getRight());
}

void ServerGamePacketListenerImpl::handleSetCarriedItem(boost::shared_ptr<ServerboundSetCarriedItemPacket> packet)
{
    int slot = packet->getSlot();
    if (slot >= 0 && slot < Inventory::getSelectionSize())
    {
        mPlayer->mInventory->m_selected = slot;
        mPlayer->resetLastActionTime();
    }
}
void ServerGamePacketListenerImpl::handleChat(boost::shared_ptr<ClientboundChatPacket> packet) {}
void ServerGamePacketListenerImpl::handleAnimate(boost::shared_ptr<ServerboundSwingPacket> packet)
{
    mPlayer->resetLastActionTime();
    mPlayer->swing(packet->getHand());
}
void ServerGamePacketListenerImpl::handlePlayerCommand(boost::shared_ptr<ServerboundPlayerCommandPacket> packet)
{
    mPlayer->resetLastActionTime();
    switch (packet->getAction())
    {
    case ServerboundPlayerCommandPacket::PRESS_SHIFT_KEY:
        mPlayer->setSneaking(true);
        break;
    case ServerboundPlayerCommandPacket::RELEASE_SHIFT_KEY:
        mPlayer->setSneaking(false);
        break;
    case ServerboundPlayerCommandPacket::STOP_SLEEPING:
        if (mPlayer->isSleeping())
        {
            mPlayer->stopSleepInBed(false, true, true);
            delete mPendingTeleportPosition;
            mPendingTeleportPosition = Vec3::newPermanent(mPlayer->mX, mPlayer->mY, mPlayer->mZ);
        }
        break;
    case ServerboundPlayerCommandPacket::START_SPRINTING:
        mPlayer->setSprinting(true);
        break;
    case ServerboundPlayerCommandPacket::STOP_SPRINTING:
        mPlayer->setSprinting(false);
        break;
    case ServerboundPlayerCommandPacket::RIDING_JUMP:
    {
        boost::shared_ptr<Entity> vehicle = mPlayer->getVehicle();
        if (vehicle && vehicle->GetType() == EntityType::HORSE)
        {
            boost::shared_ptr<Horse> horse = boost::static_pointer_cast<Horse>(vehicle);
            if (horse->canJump() && packet->getData() > 0)
                horse->handleStartJump(packet->getData());
        }
        break;
    }
    case ServerboundPlayerCommandPacket::STOP_RIDING_JUMP:
    {
        boost::shared_ptr<Entity> vehicle = mPlayer->getVehicle();
        if (vehicle && (vehicle->GetType() & EntityType::ABSTRACT_HORSE) == EntityType::ABSTRACT_HORSE)
            boost::static_pointer_cast<AbstractHorse>(vehicle)->handleStopJump();
        break;
    }
    case ServerboundPlayerCommandPacket::OPEN_INVENTORY:
    {
        boost::shared_ptr<Entity> vehicle = mPlayer->getVehicle();
        if (vehicle && (vehicle->GetType() & EntityType::ABSTRACT_HORSE) == EntityType::ABSTRACT_HORSE)
            boost::static_pointer_cast<AbstractHorse>(vehicle)->openInventory(mPlayer);
        break;
    }
    case ServerboundPlayerCommandPacket::START_FALL_FLYING:
    {
        not_null_ptr<ItemInstance> chest = mPlayer->getItemSlot(EquipmentSlot::CHEST);
        if (!mPlayer->mOnGround && mPlayer->mYd < 0.0 && !mPlayer->isFallFlying() &&
            !mPlayer->isInWater() && chest->getItem() == Items::elytra &&
            ElytraItem::isFlyEnabled(chest))
            mPlayer->startFallFlying();
        if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureAutoElytraFly())
            mPlayer->stopFallFlying();
        break;
    }
    case ServerboundPlayerCommandPacket::START_SWIMMING:
        mPlayer->startSwimming();
        break;
    case ServerboundPlayerCommandPacket::STOP_SWIMMING:
        mPlayer->stopSwimming();
        break;
    case ServerboundPlayerCommandPacket::START_SPIN_ATTACK:
        mPlayer->startSpinAttack();
        break;
    case ServerboundPlayerCommandPacket::STOP_SPIN_ATTACK:
        mPlayer->stopSpinAttack();
        break;
    }
}
void ServerGamePacketListenerImpl::handleInteract(boost::shared_ptr<ServerboundInteractPacket> packet)
{
    boost::shared_ptr<Entity> target = packet->getTarget(mServer->getLevel(mPlayer->mDimension));
    mPlayer->resetLastActionTime();
    if (target && target->isAlive())
    {
        unsigned long long type = target->GetType();
        if ((type & EntityType::PLAYER) != EntityType::PLAYER || mPlayer->distanceToSqr(target) < 36.0)
        {
            if (packet->getAction() == ServerboundInteractPacket::INTERACT)
                mPlayer->interactOn(target, packet->getHand());
            else if (packet->getAction() == ServerboundInteractPacket::INTERACT_AT)
                target->interactAt(mPlayer, packet->getLocation(), packet->getHand());
            else if (packet->getAction() == ServerboundInteractPacket::ATTACK &&
                     type != EntityType::ITEM_ENTITY && type != EntityType::EXPERIENCE_ORB &&
                     type != EntityType::ARROW && type != EntityType::TIPPABLE_ARROW && target != mPlayer)
                mPlayer->attack(target);
        }
    }
}
void ServerGamePacketListenerImpl::handleClientCommand(boost::shared_ptr<ServerboundClientCommandPacket> packet)
{
    mPlayer->resetLastActionTime();
    if (packet->getAction() != ServerboundClientCommandPacket::PERFORM_RESPAWN)
        return;

    if (mPlayer->mRespawnState)
    {
        PlayerList* players = gMinecraftServer != NULL ? gMinecraftServer->getPlayers() : NULL;
        mPlayer = players->respawn(mPlayer, mPlayer->mWonGame ? 0 : mPlayer->mDimension, true);
        mPlayer->mRespawnState = false;
    }
    else if (mPlayer->mChangedDimension || mPlayer->getHealth() <= 0.0f)
    {
        PlayerList* players = gMinecraftServer != NULL ? gMinecraftServer->getPlayers() : NULL;
        mPlayer = players->respawn(mPlayer, 0, false);
    }
}
void ServerGamePacketListenerImpl::handleContainerClose(boost::shared_ptr<ServerboundContainerClosePacket> packet)
{
    mPlayer->doCloseContainer();
}
void ServerGamePacketListenerImpl::handleContainerClick(boost::shared_ptr<ServerboundContainerClickPacket> packet)
{
    mPlayer->resetLastActionTime();
    AbstractContainerMenu* menu = mPlayer->mContainerMenu;
    if (menu->m_containerId != packet->getContainerId() || !menu->isSynched(mPlayer) || !menu->stillValid(mPlayer))
    {
        if (mPlayer->mContainerMenu->m_containerId != packet->getContainerId()
            || !mPlayer->mContainerMenu->stillValid(mPlayer))
            mPlayer->refreshContainer(mPlayer->mInventoryMenu);
        return;
    }

    if (mPlayer->isSpectator())
    {
        AbstractContainerMenu::ItemList items;
        for (unsigned int i = 0; i < mPlayer->mContainerMenu->m_slots.size(); ++i)
            items.push_back(mPlayer->mContainerMenu->m_slots.at(i)->getItem());
        mPlayer->refreshContainer(mPlayer->mContainerMenu, &items);
        return;
    }

    bool accepted = false;
    if (packet->getNumSlots() > 1)
    {
        AbstractContainerMenu::ItemList taken = mPlayer->mContainerMenu->takeAll(
            mPlayer, packet->getButtonNum(), packet->getSlotNum(), packet->getNumSlots());
        if (taken.size() == packet->getItems().size())
        {
            accepted = true;
            for (unsigned int i = 0; i < taken.size(); ++i)
            {
                if (!ItemInstance::matches(taken.at(i), packet->getItemAt(i), true))
                {
                    accepted = false;
                    break;
                }
            }
        }
    }
    else
    {
        not_null_ptr<ItemInstance> result = mPlayer->mContainerMenu->clicked(
            packet->getSlotNum(), packet->getButtonNum(), packet->getClickType(), mPlayer);
        if (ItemInstance::matches(packet->getItem(), result, true))
        {
            accepted = true;
            if (packet->getClickType() == ClickType::eClickType_QuickMove && mPlayer->mConnection != NULL
                && mPlayer->mConnection->getNetworkPlayer() != NULL
                && mPlayer->mConnection->getNetworkPlayer()->IsLocal())
            {
                boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
                    Minecraft::GetInstance()->mLocalPlayers[mPlayer->mPlayerIndex];
                if (localPlayer != NULL
                    && !AbstractContainerMenu::matches(localPlayer->mContainerMenu, mPlayer->mContainerMenu))
                    accepted = false;
            }
        }
    }

    if (accepted)
    {
        mPlayer->mConnection->send(boost::shared_ptr<Packet>(
            new ClientboundContainerAckPacket(packet->getContainerId(), packet->getUid(), true)));
        mPlayer->mIsChangingQuantityOnly = true;
        mPlayer->mContainerMenu->broadcastChanges();
        mPlayer->broadcastCarriedItem();
        mPlayer->mIsChangingQuantityOnly = false;
        return;
    }

    mContainerType[mPlayer->mContainerMenu->m_containerId] = packet->getUid();
    mPlayer->mConnection->send(boost::shared_ptr<Packet>(
        new ClientboundContainerAckPacket(packet->getContainerId(), packet->getUid(), false)));
    mPlayer->mContainerMenu->setSynched(mPlayer, false);
    AbstractContainerMenu::ItemList items;
    for (unsigned int i = 0; i < mPlayer->mContainerMenu->m_slots.size(); ++i)
    {
        not_null_ptr<ItemInstance> item = mPlayer->mContainerMenu->m_slots.at(i)->getItem();
        items.push_back(item->isEmpty() ? ItemInstance::EMPTY : item);
    }
    mPlayer->refreshContainer(mPlayer->mContainerMenu, &items);
}
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
void ServerGamePacketListenerImpl::handleSetCreativeModeSlot(boost::shared_ptr<ServerboundSetCreativeModeSlotPacket> packet)
{
    if (!mPlayer->mGameMode->isCreative())
        return;

    bool drop = packet->getSlotNum() < 0;
    not_null_ptr<ItemInstance> item = packet->getItem();
    if (!item->isEmpty() && item->hasTag() && item->getTag()->contains(L"BlockEntityTag", Tag::TAG_COMPOUND))
    {
        CompoundTag* blockEntityTag = item->getTag()->getCompound(L"BlockEntityTag");
        if (blockEntityTag->contains(L"x") && blockEntityTag->contains(L"y") && blockEntityTag->contains(L"z"))
        {
            BlockPos pos(blockEntityTag->getInt(L"x"), blockEntityTag->getInt(L"y"), blockEntityTag->getInt(L"z"));
            boost::shared_ptr<BlockEntity> blockEntity = mPlayer->mLevel->getBlockEntity(pos);
            if (blockEntity != NULL)
            {
                CompoundTag* saved = blockEntity->save(new CompoundTag());
                saved->remove(L"x");
                saved->remove(L"y");
                saved->remove(L"z");
                item->addTagElement(L"BlockEntityTag", saved);
            }
        }
    }

    bool validSlot = packet->getSlotNum() >= INVENTORY_MENU_FIRST_SLOT && packet->getSlotNum() <= INVENTORY_MENU_LAST_SLOT;
    bool validItem = item->isEmpty()
                  || (item->getAuxValue() >= 0 && item->getCount() <= 64 && item->getCount() > 0);
    if (validSlot && validItem)
    {
        if (item->isEmpty())
            mPlayer->mInventoryMenu->setItem(packet->getSlotNum(), ItemInstance::EMPTY);
        else
            mPlayer->mInventoryMenu->setItem(packet->getSlotNum(), item);
        mPlayer->mInventoryMenu->setSynched(mPlayer, true);
    }
    else if (drop && validItem && mDropSpamTickCount < DROP_SPAM_LIMIT)
    {
        mDropSpamTickCount += DROP_SPAM_STEP;
        boost::shared_ptr<ItemEntity> dropped = mPlayer->drop(item, true);
        if (dropped != NULL)
            dropped->setShortLifeTime();
    }

    if (item->getItem() == Items::filledMap)
    {
        std::vector<not_null_ptr<ItemInstance> > items;
        for (unsigned int i = 0; i < mPlayer->mInventoryMenu->m_slots.size(); ++i)
            items.push_back(mPlayer->mInventoryMenu->m_slots.at(i)->getItem());
        mPlayer->refreshContainer(mPlayer->mInventoryMenu, &items);
    }
}
void ServerGamePacketListenerImpl::handleContainerAck(boost::shared_ptr<ServerboundContainerAckPacket> packet)
{
    boost::unordered_map<int, short, IntKeyHash, IntKeyEq>::iterator transaction =
        mContainerType.find(mPlayer->mContainerMenu->m_containerId);
    if (transaction != mContainerType.end() && transaction->second == packet->getUid() &&
        mPlayer->mContainerMenu->m_containerId == packet->getContainerId() &&
        !mPlayer->mContainerMenu->isSynched(mPlayer) && !mPlayer->isSpectator())
        mPlayer->mContainerMenu->setSynched(mPlayer, true);
}
void ServerGamePacketListenerImpl::handleSignUpdate(boost::shared_ptr<ServerboundSignUpdatePacket> packet)
{
    mPlayer->resetLastActionTime();
    ServerLevel* level = mServer->getLevel(mPlayer->mDimension);
    BlockPos pos = packet->getPos();
    if (level->hasChunkAt(pos))
    {
        const BlockState* state = level->getBlockState(pos);
        boost::shared_ptr<SignBlockEntity> sign =
            boost::dynamic_pointer_cast<SignBlockEntity>(level->getBlockEntity(pos));
        if (sign)
        {
            std::wstring* lines = packet->getLines();
            for (int line = 0; line < 4; ++line)
                sign->SetMessage(line, lines[line]);
            sign->SetVerified(false);
            sign->setCreatedByRestrictedPlayer(packet->isCreatedByRestrictedPlayer());
            level->sendBlockUpdated(pos, state, state, 3, true);
        }
    }
}
void ServerGamePacketListenerImpl::handleKeepAlive(boost::shared_ptr<ServerboundKeepAlivePacket> packet)
{
    if (packet->id == mKeepAliveId)
    {
        int elapsed = (int)(getMilliseconds() - mKeepAliveTime);
        mPlayer->latency = (mPlayer->latency * 3 + elapsed) / 4;
    }
}
void ServerGamePacketListenerImpl::handlePlayerInfo(boost::shared_ptr<PlayerInfoPacket> packet)
{
    INetworkPlayer* sender = getNetworkPlayer();
    if ((sender == 0 || !sender->IsHost()) && !mPlayer->isModerator())
        return;

    boost::shared_ptr<ServerPlayer> target;
    std::vector<boost::shared_ptr<ServerPlayer> >* players = mServer->getPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator player = players->begin();
         player != players->end(); ++player)
    {
        INetworkPlayer* networkPlayer = (*player)->mConnection->getNetworkPlayer();
        if (networkPlayer != 0 && networkPlayer->GetSmallId() == packet->mPlayerId)
        {
            target = *player;
            break;
        }
    }

    if (!target)
        return;

    unsigned int privileges = target->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All);
    bool allowHostPrivileges = g_pApp->GetGameHostOption(eGameHostOption_TrustPlayers) != 0;
    bool allowPlayerPrivileges = g_pApp->GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;

    if (target == mPlayer)
    {
        if (allowPlayerPrivileges)
        {
            if (Player::getPlayerGamePrivilege(
                    privileges, Player::ePlayerGamePrivilege_CanToggleInvisible) != 0)
            {
                target->setPlayerGamePrivilege(
                    Player::ePlayerGamePrivilege_Invisible,
                    Player::getPlayerGamePrivilege(
                        packet->mPrivileges, Player::ePlayerGamePrivilege_Invisible));
                target->setPlayerGamePrivilege(
                    Player::ePlayerGamePrivilege_Invulnerable,
                    Player::getPlayerGamePrivilege(
                        packet->mPrivileges, Player::ePlayerGamePrivilege_Invulnerable));
            }

            if (Player::getPlayerGamePrivilege(
                    privileges, Player::ePlayerGamePrivilege_CreativeMode) == 0)
            {
                if (Player::getPlayerGamePrivilege(
                        privileges, Player::ePlayerGamePrivilege_CanToggleFly) != 0)
                    target->setPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_CanFly,
                        Player::getPlayerGamePrivilege(
                            packet->mPrivileges, Player::ePlayerGamePrivilege_CanFly));
                if (Player::getPlayerGamePrivilege(
                        privileges, Player::ePlayerGamePrivilege_CanToggleExhaustion) != 0)
                    target->setPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_ExhaustionDisabled,
                        Player::getPlayerGamePrivilege(
                            packet->mPrivileges, Player::ePlayerGamePrivilege_ExhaustionDisabled));
            }
        }
    }
    else
    {
        if (!allowHostPrivileges)
        {
            INetworkPlayer* targetNetworkPlayer = target->mConnection->getNetworkPlayer();
            if (targetNetworkPlayer != 0 && !targetNetworkPlayer->IsHost())
            {
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotMine,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_CannotMine));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotBuild,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_CannotBuild));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotAttackPlayers,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_CannotAttackPlayers));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotAttackAnimals,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_CannotAttackAnimals));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanUseContainers,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_CanUseContainers));
            }
        }

        if (sender != 0 && sender->IsHost())
        {
            if (allowPlayerPrivileges)
            {
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanToggleInvisible,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_CanToggleInvisible));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanToggleFly,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_CanToggleFly));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanToggleExhaustion,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_CanToggleExhaustion));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanTeleport,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_CanTeleport));
            }
            target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Op,
                Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_Op));
        }
    }

    mServer->getPlayers()->broadcastAll(
        boost::shared_ptr<Packet>(new PlayerInfoPacket(target)));
}
void ServerGamePacketListenerImpl::handlePlayerAbilities(boost::shared_ptr<ServerboundPlayerAbilitiesPacket> packet)
{
    mPlayer->mAbilities.mFlying = packet->isFlying() &&
        (mPlayer->mAbilities.mMayfly || mPlayer->isAllowedToFly());
}
static void releaseStreamBuffer(ByteArrayInputStream& bytes)
{
    bytes.mBuffer.data = NULL;
    bytes.mBuffer.length = 0;
    bytes.mCount = 0;
    bytes.mMark = 0;
    bytes.mPosition = 0;
}

void ServerGamePacketListenerImpl::handleCustomPayload(boost::shared_ptr<ServerboundCustomPayloadPacket> packet)
{
    std::wstring identifier = packet->getIdentifier();
    if (identifier == L"MC|BEdit")
    {
        ByteArrayInputStream bytes(packet->getData());
        DataInputStream input(&bytes);
        not_null_ptr<ItemInstance> book = input.readItem();
        input.close();
        releaseStreamBuffer(bytes);
        if (book->isEmpty() || !WritableBookItem::makeSureTagIsValid(book->getTag()))
            return;
        not_null_ptr<ItemInstance> held = mPlayer->getMainHandItem();
        if (held->isEmpty())
            return;
        if (book->getItem() == Items::writableBook && book->getItem() == held->getItem())
        {
            CompoundTag* bookTag = book->getTag();
            held->addTagElement(L"pages", bookTag->getList(L"pages", Tag::TAG_STRING)->copy());
            held->getTag()->putString(L"ownerUUID", bookTag->getString(L"ownerUUID"));
            held->getTag()->putByte(L"createdByRestrictedPlayer", bookTag->getByte(L"createdByRestrictedPlayer"));
            held->getTag()->putByte(L"createdByHost", bookTag->getByte(L"createdByHost"));
        }
        return;
    }

    if (identifier == L"MC|BSign")
    {
        ByteArrayInputStream bytes(packet->getData());
        DataInputStream input(&bytes);
        not_null_ptr<ItemInstance> book = input.readItem();
        input.close();
        releaseStreamBuffer(bytes);
        if (book->isEmpty() || !WrittenBookItem::makeSureTagIsValid(book->getTag()))
            return;
        not_null_ptr<ItemInstance> held = mPlayer->getMainHandItem();
        if (held->isEmpty())
            return;
        if (book->getItem() == Items::writableBook && held->getItem() == Items::writableBook)
        {
            not_null_ptr<ItemInstance> signedBook(new ItemInstance(Items::writtenBook));
            signedBook->addTagElement(L"author", new StringTag(mPlayer->getName()));
            signedBook->addTagElement(L"title", new StringTag(book->getTag()->getString(L"title")));
            signedBook->addTagElement(L"pages", book->getTag()->getList(L"pages", Tag::TAG_STRING)->copy());
            mPlayer->setItemInHand(InteractionHand::eInteractionHand_MainHand, signedBook);
            signedBook->getTag()->putString(L"ownerUUID", book->getTag()->getString(L"ownerUUID"));
            signedBook->getTag()->putByte(L"createdByRestrictedPlayer",
                                          book->getTag()->getByte(L"createdByRestrictedPlayer"));
            signedBook->getTag()->putByte(L"createdByHost", book->getTag()->getByte(L"createdByHost"));
        }
        return;
    }

    if (identifier == L"MC|TrSel")
    {
        ByteArrayInputStream bytes(packet->getData());
        DataInputStream input(&bytes);
        int selection = input.readInt();
        input.close();
        AbstractContainerMenu* menu = mPlayer->mContainerMenu;
        if (menu != NULL && dynamic_cast<MerchantMenu*>(menu) != NULL)
            static_cast<MerchantMenu*>(menu)->setSelectionHint(selection);
        releaseStreamBuffer(bytes);
        return;
    }

    if (identifier == L"MC|Beacon")
    {
        AbstractContainerMenu* menu = mPlayer->mContainerMenu;
        if (menu == NULL)
            return;
        BeaconMenu* beaconMenu = dynamic_cast<BeaconMenu*>(menu);
        if (beaconMenu == NULL)
            return;
        ByteArrayInputStream bytes(packet->getData());
        DataInputStream input(&bytes);
        int primary = input.readInt();
        int secondary = input.readInt();
        input.close();
        Slot* payment = beaconMenu->getSlot(0);
        if (payment->hasItem())
        {
            payment->remove(1);
            boost::shared_ptr<Container> beacon = beaconMenu->getBeaconContainer();
            beacon->setContainerData(1, primary);
            beacon->setContainerData(2, secondary);
            beacon->setChanged();
        }
        releaseStreamBuffer(bytes);
        return;
    }

    if (identifier == L"MC|ItemName")
    {
        NameItemOnAnvil(packet, false);
        return;
    }
    if (identifier == L"MC|RIName")
    {
        NameItemOnAnvil(packet, true);
        return;
    }

    if (identifier == L"MC|PickItem")
    {
        ByteArrayInputStream bytes(packet->getData());
        DataInputStream input(&bytes);
        not_null_ptr<ItemInstance> item = input.readItem();
        mPlayer->mInventory->setPickedItem(item);
        input.close();
        releaseStreamBuffer(bytes);
    }
}
void ServerGamePacketListenerImpl::handleCraftItem(boost::shared_ptr<CraftItemPacket> packet)
{
    if (packet->mStackedItems.empty() && packet->mItemIds.empty())
        return;

    boost::shared_ptr<ItemInstance> craftedItem;
    if (packet->mStackedItems.size() == 1 && packet->mStackedItems[0].second == -1)
    {
        craftedItem = mPlayer->mGameMode->handleCraftItem(
            packet->mStackedItems[0].first, packet->mCraftType, mPlayer);
    }
    else if (packet->mItemIds.empty())
    {
        craftedItem = mPlayer->mGameMode->handleStackedCraftItem(
            packet->mStackedItems, packet->mCraftType, mPlayer);
    }
    else
    {
        craftedItem = mPlayer->mGameMode->handleCustomCraftItem(
            packet->mItemIds, mPlayer);
    }

    if (craftedItem->getItem() == Items::map)
    {
        std::vector<not_null_ptr<ItemInstance> > items;
        for (std::vector<Slot*>::iterator slot = mPlayer->mContainerMenu->m_slots.begin();
             slot != mPlayer->mContainerMenu->m_slots.end(); ++slot)
            items.push_back((*slot)->getItem());
        mPlayer->refreshContainer(mPlayer->mContainerMenu, &items);
    }
    else
    {
        mPlayer->mIsChangingQuantityOnly = true;
        mPlayer->mContainerMenu->broadcastChanges();
        mPlayer->broadcastCarriedItem();
        mPlayer->mIsChangingQuantityOnly = false;
    }
}
void ServerGamePacketListenerImpl::handleTradeItem(boost::shared_ptr<TradeItemPacket> packet)
{
    if (mPlayer->mContainerMenu->m_containerId != packet->containerId)
        return;

    MerchantMenu* menu = static_cast<MerchantMenu*>(mPlayer->mContainerMenu);
    boost::shared_ptr<Merchant> merchant = menu->getMerchant();
    MerchantRecipeList* offers = merchant->getOffers(mPlayer);
    if (offers == 0 || packet->itemIndex < 0 ||
        (unsigned int)packet->itemIndex >= (unsigned int)offers->size())
        return;

    MerchantRecipe* recipe = offers->at(packet->itemIndex);
    if (recipe->isDeprecated())
        return;

    not_null_ptr<ItemInstance> buyA = recipe->getBuyAItem();
    not_null_ptr<ItemInstance> buyB = recipe->getBuyBItem();
    int buyACount = mPlayer->mInventory->countMatches(buyA);
    int buyBCount = mPlayer->mInventory->countMatches(buyB);
    if (buyA->isEmpty() || buyA->getCount() > buyACount ||
        (!buyB->isEmpty() && buyB->getCount() > buyBCount))
        return;

    menu->getMerchant()->notifyTrade(recipe);
    mPlayer->mInventory->removeResources(buyA);
    mPlayer->mInventory->removeResources(buyB);

    not_null_ptr<ItemInstance> sell = recipe->getSellItem()->copy();
    mPlayer->awardStat(gStats->get_itemsBought(Item::VALIDgetId(sell->getItem())),
                       GenericStats::param_itemsBought(sell));
    if (!mPlayer->mInventory->add(sell))
        mPlayer->drop(sell, false);
}
void ServerGamePacketListenerImpl::handleDebugOptions(boost::shared_ptr<DebugOptionsPacket> packet)
{
    mPlayer->mDebugOptions = packet->options;
}
void ServerGamePacketListenerImpl::handleTexture(boost::shared_ptr<TexturePacket> packet)
{
    if (packet->length != 0)
    {
        g_pApp->AddMemoryTextureFile(packet->textureName, packet->data, packet->length);
        mServer->getConnection()->handleTextureReceived(packet->textureName);
        return;
    }

    unsigned char* data = 0;
    unsigned int length = 0;
    g_pApp->GetMemFileDetails(packet->textureName, &data, &length);
    if (length == 0)
        mNames.push_back(packet->textureName);
    else
        send(boost::shared_ptr<Packet>(new TexturePacket(packet->textureName, data, length)));
}
void ServerGamePacketListenerImpl::handleTextureAndGeometry(boost::shared_ptr<TextureAndGeometryPacket> packet)
{
    if (packet->length != 0)
    {
        g_pApp->AddMemoryTextureFile(packet->textureName, packet->data, packet->length);
        if (packet->additionalBoxes != 0)
            g_pApp->SetAdditionalSkinBoxes(packet->geometryId, packet->additionalBoxes,
                                           packet->additionalBoxCount, packet->skinAdjustments);
        g_pApp->SetSkinAdjustments(packet->geometryId, packet->skinAdjustments);
        mPlayer->setCustomSkin(packet->geometryId);
        mServer->getConnection()->handleTextureAndGeometryReceived(packet->textureName);
        return;
    }

    unsigned char* data = 0;
    unsigned int length = 0;
    g_pApp->GetMemFileDetails(packet->textureName, &data, &length);
    DLCSkinFile* skinFile = static_cast<DLCSkinFile*>(
        g_pApp->mDLCManager.getSkinFile(packet->textureName));
    if (length == 0)
    {
        mNames.push_back(packet->textureName);
    }
    else if (skinFile == 0)
    {
        std::vector<SKIN_BOX*>* boxes = g_pApp->GetAdditionalSkinBoxes(packet->geometryId);
        _SkinAdjustments adjustments = g_pApp->GetSkinAdjustments(packet->geometryId);
        send(boost::shared_ptr<Packet>(new TextureAndGeometryPacket(
            packet->textureName, data, length, boxes, adjustments)));
    }
    else
    {
        send(boost::shared_ptr<Packet>(new TextureAndGeometryPacket(
            packet->textureName, data, length, skinFile)));
    }
}
void ServerGamePacketListenerImpl::handleTextureChange(boost::shared_ptr<TextureChangePacket> packet)
{
    if (packet->mChangeType == TextureChangePacket::eTextureChangeType_Skin)
        mPlayer->setCustomSkin(CMinecraftApp::getSkinIdFromPath(packet->mTextureName));
    else if (packet->mChangeType == TextureChangePacket::eTextureChangeType_Cape)
        mPlayer->setCustomCape(Player::getCapeIdFromPath(packet->mTextureName));

    bool requestTexture = !packet->mTextureName.empty() &&
        packet->mTextureName.compare(0, 3, L"def") != 0 &&
        !g_pApp->IsFileInMemoryTextures(packet->mTextureName);
    if (requestTexture)
    {
        if (mServer->getConnection()->addPendingTextureRequest(packet->mTextureName))
            send(boost::shared_ptr<Packet>(new TexturePacket(packet->mTextureName, 0, 0)));
    }
    else if (!packet->mTextureName.empty() &&
             g_pApp->IsFileInMemoryTextures(packet->mTextureName))
    {
        g_pApp->AddMemoryTextureFile(packet->mTextureName, 0, 0);
    }

    boost::shared_ptr<Entity> player = mPlayer;
    mServer->getPlayers()->broadcastAll(
        boost::shared_ptr<Packet>(new TextureChangePacket(
            player, packet->mChangeType, packet->mTextureName)),
        mPlayer->mDimension);
}
void ServerGamePacketListenerImpl::handleTextureAndGeometryChange(boost::shared_ptr<TextureAndGeometryChangePacket> packet)
{
    mPlayer->setCustomSkin(CMinecraftApp::getSkinIdFromPath(packet->geometryName));

    bool requestGeometry = !packet->geometryName.empty() &&
        packet->geometryName.compare(0, 3, L"def") != 0 &&
        !g_pApp->IsFileInMemoryTextures(packet->geometryName);
    if (requestGeometry)
    {
        if (mServer->getConnection()->addPendingTextureRequest(packet->geometryName))
            send(boost::shared_ptr<Packet>(new TextureAndGeometryPacket(
                packet->geometryName, 0, 0)));
    }
    else if (!packet->geometryName.empty() &&
             g_pApp->IsFileInMemoryTextures(packet->geometryName))
    {
        g_pApp->AddMemoryTextureFile(packet->geometryName, 0, 0);
        mPlayer->setCustomSkin(packet->geometryId);
    }

    boost::shared_ptr<Entity> player = mPlayer;
    mServer->getPlayers()->broadcastAll(
        boost::shared_ptr<Packet>(new TextureAndGeometryChangePacket(
            player, packet->geometryName)),
        mPlayer->mDimension);
}
void ServerGamePacketListenerImpl::handleServerSettingsChanged(boost::shared_ptr<ServerSettingsChangedPacket> packet)
{
    if (packet->mSetting == 1)
    {
        INetworkPlayer* networkPlayer = getNetworkPlayer();
        if ((networkPlayer != 0 && networkPlayer->IsHost()) || mPlayer->isModerator())
        {
            const eGameHostOption options[] = {
                eGameHostOption_FireSpreads, eGameHostOption_TNT, eGameHostOption_MobGriefing,
                eGameHostOption_KeepInventory, eGameHostOption_DoMobSpawning, eGameHostOption_DoMobLoot,
                eGameHostOption_DoTileDrops, eGameHostOption_DoDaylightCycle, eGameHostOption_NaturalRegeneration,
                eGameHostOption_DoWeatherCycle
            };
            for (unsigned int i = 0; i < sizeof(options) / sizeof(options[0]); ++i)
                g_pApp->SetGameHostOption(
                    options[i], g_pApp->GetGameHostOption(packet->mValue, options[i]));

            unsigned int value = g_pApp->GetGameHostOption(eGameHostOption_All);
            mServer->getPlayers()->broadcastAll(boost::shared_ptr<Packet>(
                new ServerSettingsChangedPacket(1, value)));
            CGameNetworkManager::sInstance.UpdateAndSetGameSessionData(0);
        }
    }
}
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
    case GameModePacket::eMessage_Follow:
    {
        GameModePacket::FollowData data;
        packet->ReadPacket(data);
        mServer->mMasterGameMode->OnFollow(mPlayer, data.follow, data.teleport);
        break;
    }
    case GameModePacket::eMessage_StartSpectating:
        mServer->mMasterGameMode->OnStartSpectating(mPlayer);
        break;
    case GameModePacket::eMessage_FinishedLoadingLocally:
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
    DisconnectPacket::eDisconnectReason, void*)
{
    EnterCriticalSection(&mDisconnectMutex);
    if (!mDisconnected)
    {
        ClientboundChatPacket::EChatPacketMessage message = mUseAltLeaveMessage
            ? ClientboundChatPacket::eChatPacketMessage_PlayerKicked
            : ClientboundChatPacket::eChatPacketMessage_PlayerLeft;
        PlayerList* players = gMinecraftServer != NULL ? gMinecraftServer->getPlayers() : NULL;
        players->broadcastAll(
            boost::shared_ptr<Packet>(new ClientboundChatPacket(mPlayer->getName(), message, -1)));
        mPlayer->disconnect();
        players = gMinecraftServer != NULL ? gMinecraftServer->getPlayers() : NULL;
        players->remove(mPlayer);
        mDisconnected = true;
    }
    LeaveCriticalSection(&mDisconnectMutex);
}

uint64_t ServerGamePacketListenerImpl::getMilliseconds()
{
    return System::processTimeInMilliSecs();
}

void ServerGamePacketListenerImpl::NameItemOnAnvil(
    const boost::shared_ptr<ServerboundCustomPayloadPacket>& packet, bool censored)
{
    AnvilMenu* menu = dynamic_cast<AnvilMenu*>(mPlayer->mContainerMenu);
    if (menu != 0)
    {
        arrayWithLength<unsigned char> data = packet->getData();
        if (data.length != 0 && data.data != 0)
        {
            ByteArrayInputStream bytes(data);
            DataInputStream input(&bytes);
            std::wstring name = input.readUTF();
            input.close();
            if (name.length() < 31)
                menu->setItemName(name, censored);
            bytes.mBuffer.data = NULL;
            bytes.mBuffer.length = 0;
            bytes.mCount = 0;
            bytes.mMark = 0;
            bytes.mPosition = 0;
        }
        else
        {
            menu->setItemName(L"", false);
        }
    }
}

void ServerGamePacketListenerImpl::handleTextureReceived(const std::wstring& name)
{
    std::vector<std::wstring>::iterator request =
        std::find(mNames.begin(), mNames.end(), name);
    if (request == mNames.end())
        return;

    unsigned char* data = 0;
    unsigned int length = 0;
    g_pApp->GetMemFileDetails(name, &data, &length);
    if (length != 0)
    {
        send(boost::shared_ptr<Packet>(new TexturePacket(name, data, length)));
        mNames.erase(request);
    }
}

void ServerGamePacketListenerImpl::handleTextureAndGeometryReceived(const std::wstring& name)
{
    std::vector<std::wstring>::iterator request =
        std::find(mNames.begin(), mNames.end(), name);
    if (request == mNames.end())
        return;

    unsigned char* data = 0;
    unsigned int length = 0;
    g_pApp->GetMemFileDetails(name, &data, &length);
    if (length == 0)
        return;

    unsigned int skinId = CMinecraftApp::getSkinIdFromPath(name);
    DLCSkinFile* skinFile = static_cast<DLCSkinFile*>(g_pApp->mDLCManager.getSkinFile(name));
    if (skinFile == 0)
    {
        std::vector<SKIN_BOX*>* boxes = g_pApp->GetAdditionalSkinBoxes(skinId);
        _SkinAdjustments adjustments = g_pApp->GetSkinAdjustments(skinId);
        send(boost::shared_ptr<Packet>(new TextureAndGeometryPacket(
            name, data, length, boxes, adjustments)));
    }
    else
    {
        send(boost::shared_ptr<Packet>(new TextureAndGeometryPacket(name, data, length, skinFile)));
    }
    mNames.erase(request);
}

bool ServerGamePacketListenerImpl::isDisconnected()
{
    return mDisconnected;
}

bool ServerGamePacketListenerImpl::isGuest()
{
    if (mConnection->m_socket == 0)
        return false;

    INetworkPlayer* networkPlayer = mConnection->m_socket->getPlayer();
    return networkPlayer != 0 && networkPlayer->IsGuest();
}

Connection* ServerGamePacketListenerImpl::getConnection()
{
    return mConnection;
}

INetworkPlayer* getNetworkPlayerFromIndex(int index)
{
    PlayerList* playerList = gMinecraftServer != 0 ? gMinecraftServer->getPlayers() : 0;
    std::vector<boost::shared_ptr<ServerPlayer> >* players = playerList->getPlayers();
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin();
    while (it != players->end())
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        if (player->mPlayerIndex == index)
            return player->mConnection->getNetworkPlayer();
        ++it;
    }
    return 0;
}
