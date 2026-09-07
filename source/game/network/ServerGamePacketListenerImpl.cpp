#include "game/network/ServerGamePacketListenerImpl.hpp"
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
#include "game/core/Boat.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DLCSkinFile.hpp"
#include "game/core/Minecraft.hpp"
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
#include "game/world/block/SignBlockEntity.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/server/ServerLevel.hpp"

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
    if (mDisconnected)
        return;

    mPlayer->disconnect();
    mServer->getPlayers()->removePlayerFromReceiving(mPlayer, true, 0);
    send(boost::shared_ptr<Packet>(new DisconnectPacket(reason)));
    mConnection->sendAndQuit();
    mServer->getPlayers()->broadcastAll(boost::shared_ptr<Packet>(
        new ClientboundChatPacket(mPlayer->getName(),
                                  static_cast<ClientboundChatPacket::EChatPacketMessage>(8), -1)));
    mServer->getPlayers()->remove(mPlayer);
    mDisconnected = true;
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
    bool sendPacket, bool includeRotation)
{
    delete mPendingTeleportPosition;
    mPendingTeleportPosition = Vec3::newPermanent(x, y, z);
    ++mTeleportId;
    if (mTeleportId == 0x7fffffff)
        mTeleportId = 0;

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
    if (mServer->IsServerPaused())
    {
        disconnect(DisconnectPacket::eDisconnectReason_ConnectionClosed);
        return;
    }

    resetPosition();
    mPlayer->doTick(false, false, false);
    mPlayer->absMoveTo(mLastX, mLastY, mLastZ, mPlayer->mYRot, mPlayer->mXRot);
    mConnection->tick();
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
void ServerGamePacketListenerImpl::handleMoveVehicle(boost::shared_ptr<ServerboundMoveVehiclePacket> packet)
{
    if (containsInvalidValues(packet))
    {
        disconnect(DisconnectPacket::eDisconnectReason_InvalidMovement);
        return;
    }

    boost::shared_ptr<Entity> vehicle = mPlayer->getRootVehicle();
    if (!vehicle || vehicle == mPlayer || vehicle->getControllingPassenger() != mPlayer)
        return;

    double oldX = vehicle->mX;
    double oldY = vehicle->mY;
    double oldZ = vehicle->mZ;
    vehicle->absMoveTo(packet->getX(), packet->getY(), packet->getZ(),
                       packet->getYRot(), packet->getXRot());
    mPlayer->getLevel()->tick(vehicle, false);
    mServer->getPlayers()->move(mPlayer);

    double dx = packet->getX() - oldX;
    double dy = packet->getY() - oldY;
    double dz = packet->getZ() - oldZ;
    mPlayer->doCheckFallDamage(dy, mPlayer->mOnGround);
    mLastX = packet->getX();
    mLastY = packet->getY();
    mLastZ = packet->getZ();

    if (dx * dx + dy * dy + dz * dz > 100.0 && !mServer->isFlightAllowed())
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundMoveVehiclePacket(vehicle)));
}
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
void ServerGamePacketListenerImpl::handleMovePlayer(boost::shared_ptr<ServerboundMovePlayerPacket> packet)
{
    if (mPlayer->isChangingDimension())
        return;

    double x = packet->getX(mPlayer->mX);
    double y = packet->getY(mPlayer->mY);
    double z = packet->getZ(mPlayer->mZ);
    float yRot = packet->getYRot(mPlayer->mYRot);
    float xRot = packet->getXRot(mPlayer->mXRot);
    double dy = y - mPlayer->mY;

    if (mPendingTeleportPosition != 0)
    {
        if (mPlayer->distanceToSqr(mPendingTeleportPosition->x,
                                   mPendingTeleportPosition->y,
                                   mPendingTeleportPosition->z) > 1.0)
            teleport(mPendingTeleportPosition->x, mPendingTeleportPosition->y,
                     mPendingTeleportPosition->z, yRot, xRot, false, false);
        return;
    }

    mPlayer->absMoveTo(x, y, z, yRot, xRot);
    mPlayer->mOnGround = packet->isOnGround();
    mPlayer->doCheckFallDamage(dy, packet->isOnGround());
    mServer->getPlayers()->move(mPlayer);
    resetPosition();
}
void ServerGamePacketListenerImpl::handlePlayerAction(boost::shared_ptr<ServerboundPlayerActionPacket> packet)
{
    ServerLevel* level = mServer->getLevel(mPlayer->mDimension);
    BlockPos pos = packet->getPos();
    mPlayer->resetLastActionTime();

    switch (packet->getAction())
    {
    case ServerboundPlayerActionPacket::START_DESTROY_BLOCK:
        if (mPlayer->distanceToSqr(pos.x + 0.5, pos.y + 0.5,
                                   pos.z + 0.5) <= 36.0 &&
            pos.y < mServer->getMaxBuildHeight() &&
            level->getWorldBorder()->isWithinBounds(pos))
        {
            if (mPlayer->mGameMode->startDestroyBlock(pos, packet->getDirection()))
                mServer->getPlayers()->prioritiseBlockChanges(pos, mPlayer->mDimension);
        }
        else
        {
            send(boost::shared_ptr<Packet>(new ClientboundBlockUpdatePacket(level, pos)));
        }
        break;
    case ServerboundPlayerActionPacket::ABORT_DESTROY_BLOCK:
        mPlayer->mGameMode->abortDestroyBlock();
        break;
    case ServerboundPlayerActionPacket::STOP_DESTROY_BLOCK:
        mPlayer->mGameMode->stopDestroyBlock(pos);
        break;
    case ServerboundPlayerActionPacket::DROP_ALL_ITEMS:
        if (!mPlayer->isSpectator())
            mPlayer->drop(true);
        break;
    case ServerboundPlayerActionPacket::DROP_ITEM:
        if (!mPlayer->isSpectator())
            mPlayer->drop(false);
        break;
    case ServerboundPlayerActionPacket::RELEASE_USE_ITEM:
        mPlayer->releaseUsingItem(packet->getUseTime());
        break;
    case ServerboundPlayerActionPacket::SWAP_ITEM_WITH_OFFHAND:
        if (!mPlayer->isSpectator())
        {
            not_null_ptr<ItemInstance> mainHand = mPlayer->getItemSlot(EquipmentSlot::MAINHAND);
            not_null_ptr<ItemInstance> offHand = mPlayer->getItemSlot(EquipmentSlot::OFFHAND);
            mPlayer->setItemSlot(EquipmentSlot::MAINHAND, offHand);
            mPlayer->setItemSlot(EquipmentSlot::OFFHAND, mainHand);
            mPlayer->stopUsingItem();
        }
        break;
    }
}
void ServerGamePacketListenerImpl::handleUseItemOn(boost::shared_ptr<ServerboundUseItemOnPacket> packet)
{
    ServerLevel* level = mServer->getLevel(mPlayer->mDimension);
    InteractionHand::EInteractionHand hand = packet->getHand();
    not_null_ptr<ItemInstance> item = mPlayer->getItemInHand(hand);
    BlockPos pos = packet->getPos();
    const Direction* face = packet->getFace();
    mPlayer->resetLastActionTime();

    double reachX = pos.x + packet->getClickX();
    double reachY = pos.y + packet->getClickY();
    double reachZ = pos.z + packet->getClickZ();
    if (mPlayer->distanceToSqr(reachX, reachY, reachZ) < 64.0 &&
        pos.y < mServer->getMaxBuildHeight() &&
        level->getWorldBorder()->isWithinBounds(pos))
    {
        bool result = false;
        mPlayer->mGameMode->useItemOn(mPlayer, level, item, hand, pos, face,
                                      packet->getClickX(), packet->getClickY(),
                                      packet->getClickZ(), false, &result);
    }

    send(boost::shared_ptr<Packet>(new ClientboundBlockUpdatePacket(level, pos)));
    send(boost::shared_ptr<Packet>(new ClientboundBlockUpdatePacket(level, pos.relative(face))));
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
            oldLevelData->getCloudHeight(),
            oldLevelData->getXZSize(),
            oldLevelData->getHellScale(),
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
    if (vehicle && vehicle->GetType() == 0x8000008ULL)
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
    if (mPlayer)
    {
        mPlayer->resetLastActionTime();
        if (packet)
            mPlayer->swing(packet->getHand());
    }
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
        if (vehicle && vehicle->GetType() == 0x2d690001ULL)
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
        if (vehicle && (vehicle->GetType() & 0xd690000ULL) == 0xd690000ULL)
            boost::static_pointer_cast<AbstractHorse>(vehicle)->handleStopJump();
        break;
    }
    case ServerboundPlayerCommandPacket::OPEN_INVENTORY:
    {
        boost::shared_ptr<Entity> vehicle = mPlayer->getVehicle();
        if (vehicle && (vehicle->GetType() & 0xd690000ULL) == 0xd690000ULL)
            boost::static_pointer_cast<AbstractHorse>(vehicle)->openInventory(mPlayer);
        break;
    }
    case ServerboundPlayerCommandPacket::ACTION_8:
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
    case ServerboundPlayerCommandPacket::ACTION_9:
        mPlayer->startSwimming();
        break;
    case ServerboundPlayerCommandPacket::ACTION_10:
        mPlayer->stopSwimming();
        break;
    case ServerboundPlayerCommandPacket::ACTION_11:
        mPlayer->startSpinAttack();
        break;
    case ServerboundPlayerCommandPacket::ACTION_12:
        mPlayer->stopSpinAttack();
        break;
    }
}
void ServerGamePacketListenerImpl::handleInteract(boost::shared_ptr<ServerboundInteractPacket> packet)
{
    boost::shared_ptr<Entity> target = packet->getTarget(mServer->getLevel(mPlayer->mDimension));
    mPlayer->resetLastActionTime();
    if (target && target->isPickable())
    {
        unsigned long long type = target->GetType();
        if ((type & 0xc800000ULL) != 0xc800000ULL || mPlayer->distanceToSqr(target) < 36.0)
        {
            if (packet->getAction() == ServerboundInteractPacket::INTERACT)
                mPlayer->interactOn(target, packet->getHand());
            else if (packet->getAction() == ServerboundInteractPacket::INTERACT_AT)
                target->interactAt(mPlayer, packet->getLocation(), packet->getHand());
            else if (packet->getAction() == ServerboundInteractPacket::ATTACK &&
                     type != 0x800000aULL && type != 0x8000003ULL &&
                     type != 0x8a020000ULL && type != 0x8a020002ULL && target != mPlayer)
                mPlayer->attack(target);
        }
    }
}
void ServerGamePacketListenerImpl::handleClientCommand(boost::shared_ptr<ServerboundClientCommandPacket> packet)
{
    mPlayer->resetLastActionTime();
    if (packet->getAction() == ServerboundClientCommandPacket::PERFORM_RESPAWN)
    {
        if (mPlayer->mFallFlying)
        {
            int dimension = mPlayer->mRespawnState ? 0 : mPlayer->mDimension;
            mPlayer = mServer->getPlayers()->respawn(mPlayer, dimension, true);
            mPlayer->mFallFlying = false;
        }
        else if (mPlayer->getHealth() <= 0.0f)
        {
            mPlayer = mServer->getPlayers()->respawn(mPlayer, 0, false);
        }
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
    if (menu->m_containerId != packet->getContainerId())
    {
        mPlayer->refreshContainer(mPlayer->mInventoryMenu);
        return;
    }
    if (!menu->isSynched(mPlayer))
    {
        mPlayer->refreshContainer(mPlayer->mInventoryMenu);
        return;
    }
    if (mPlayer->isSpectator())
    {
        std::vector<not_null_ptr<ItemInstance> > items;
        for (std::vector<Slot*>::iterator slot = menu->m_slots.begin();
             slot != menu->m_slots.end(); ++slot)
            items.push_back((*slot)->getItem());
        mPlayer->refreshContainer(menu, &items);
        return;
    }

    not_null_ptr<ItemInstance> result = menu->clicked(
        packet->getSlotNum(), packet->getButtonNum(), packet->getClickType(), mPlayer);
    bool accepted = packet->getNumSlots() < 2
        ? ItemInstance::matches(packet->getItem(), result, true)
        : true;
    send(boost::shared_ptr<Packet>(new ClientboundContainerAckPacket(
        packet->getContainerId(), packet->getUid(), accepted)));

    if (!accepted)
    {
        mContainerType[menu->m_containerId] = packet->getUid();
        menu->setSynched(mPlayer, false);
        mPlayer->refreshContainer(menu);
        mPlayer->broadcastCarriedItem();
    }
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

    int slot = packet->getSlotNum();
    not_null_ptr<ItemInstance> item = packet->getItem();
    if (slot >= 1 && slot < 46)
    {
        if (!item->isEmpty())
            item->setCount(std::min(item->getCount(), 64));
        mPlayer->mInventoryMenu->setItem(slot, item);
        mPlayer->mInventoryMenu->setSynched(mPlayer, true);
    }
    else if (slot < 0 && !item->isEmpty())
    {
        boost::shared_ptr<ItemEntity> dropped = mPlayer->drop(item, true);
        if (dropped)
            dropped->setShortLifeTime();
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
        boost::shared_ptr<SignBlockEntity> sign =
            boost::dynamic_pointer_cast<SignBlockEntity>(level->getBlockEntity(pos));
        if (sign)
        {
            std::wstring* lines = packet->getLines();
            for (int line = 0; line < 4; ++line)
                sign->SetMessage(line, lines[line]);
            sign->SetVerified(false);
            sign->setCreatedByRestrictedPlayer(packet->isCreatedByRestrictedPlayer());
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
    bool allowHostPrivileges = g_pApp->GetGameHostOption(eGameHostOption_12) != 0;
    bool allowPlayerPrivileges = g_pApp->GetGameHostOption(eGameHostOption_15) != 0;

    if (target == mPlayer)
    {
        if (allowPlayerPrivileges)
        {
            if (Player::getPlayerGamePrivilege(
                    privileges, Player::ePlayerGamePrivilege_13) != 0)
            {
                target->setPlayerGamePrivilege(
                    Player::ePlayerGamePrivilege_7,
                    Player::getPlayerGamePrivilege(
                        packet->mPrivileges, Player::ePlayerGamePrivilege_7));
                target->setPlayerGamePrivilege(
                    Player::ePlayerGamePrivilege_8,
                    Player::getPlayerGamePrivilege(
                        packet->mPrivileges, Player::ePlayerGamePrivilege_8));
            }

            if (Player::getPlayerGamePrivilege(
                    privileges, Player::ePlayerGamePrivilege_9) == 0)
            {
                if (Player::getPlayerGamePrivilege(
                        privileges, Player::ePlayerGamePrivilege_14) != 0)
                    target->setPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_5,
                        Player::getPlayerGamePrivilege(
                            packet->mPrivileges, Player::ePlayerGamePrivilege_5));
                if (Player::getPlayerGamePrivilege(
                        privileges, Player::ePlayerGamePrivilege_15) != 0)
                    target->setPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_6,
                        Player::getPlayerGamePrivilege(
                            packet->mPrivileges, Player::ePlayerGamePrivilege_6));
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
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_0,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_0));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_1,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_1));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_3,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_3));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_10,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_10));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_11,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_11));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_12,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_12));
            }
        }

        if (sender != 0 && sender->IsHost())
        {
            if (allowPlayerPrivileges)
            {
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_13,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_13));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_14,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_14));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_15,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_15));
                target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_16,
                    Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_16));
            }
            target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_4,
                Player::getPlayerGamePrivilege(packet->mPrivileges, Player::ePlayerGamePrivilege_4));
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
void ServerGamePacketListenerImpl::handleCustomPayload(boost::shared_ptr<ServerboundCustomPayloadPacket> packet)
{
    std::wstring identifier = packet->getIdentifier();
    arrayWithLength<unsigned char> data = packet->getData();

    if (identifier == L"MC|ItemName")
    {
        NameItemOnAnvil(packet, true);
        return;
    }
    if (identifier == L"MC|ItemNameCancel")
    {
        NameItemOnAnvil(packet, false);
        return;
    }

    ByteArrayInputStream bytes(data);
    DataInputStream input(&bytes);
    if (identifier == L"MC|TrSel")
    {
        MerchantMenu* menu = dynamic_cast<MerchantMenu*>(mPlayer->mContainerMenu);
        if (menu != 0)
            menu->setSelectionHint(input.readInt());
    }
    else if (identifier == L"MC|Beacon")
    {
        BeaconMenu* menu = dynamic_cast<BeaconMenu*>(mPlayer->mContainerMenu);
        int primary = input.readInt();
        int secondary = input.readInt();
        if (menu != 0)
        {
            boost::shared_ptr<Container> beacon = menu->getBeaconContainer();
            beacon->setContainerData(1, primary);
            beacon->setContainerData(2, secondary);
            beacon->removeItem(0, 1);
        }
    }
    else if (identifier == L"MC|PickItem")
    {
        not_null_ptr<ItemInstance> item = input.readItem();
        mPlayer->mInventory->setPickedItem(item);
    }
    else if (identifier == L"MC|BEdit" || identifier == L"MC|BSign")
    {
        not_null_ptr<ItemInstance> book = input.readItem();
        not_null_ptr<ItemInstance> held = mPlayer->getItemInHand(InteractionHand::eInteractionHand_MainHand);
        bool signing = identifier == L"MC|BSign";
        bool valid = signing
            ? WrittenBookItem::makeSureTagIsValid(book->getTag())
            : WritableBookItem::makeSureTagIsValid(book->getTag());
        if (valid && held->getItem() == Items::writableBook &&
            (!signing || book->getItem() == Items::writtenBook))
            mPlayer->setItemInHand(InteractionHand::eInteractionHand_MainHand, book);
    }
    input.close();
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
                eGameHostOption_14, eGameHostOption_13, eGameHostOption_24,
                eGameHostOption_25, eGameHostOption_26, eGameHostOption_27,
                eGameHostOption_28, eGameHostOption_30, eGameHostOption_29,
                eGameHostOption_21
            };
            for (unsigned int i = 0; i < sizeof(options) / sizeof(options[0]); ++i)
                g_pApp->SetGameHostOption(
                    options[i], g_pApp->GetGameHostOption(packet->mValue, options[i]));

            unsigned int value = g_pApp->GetGameHostOption(eGameHostOption_32);
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
    DisconnectPacket::eDisconnectReason, void*)
{
    if (mDisconnected)
        return;

    mServer->getPlayers()->broadcastAll(boost::shared_ptr<Packet>(
        new ClientboundChatPacket(mPlayer->getName(),
                                  static_cast<ClientboundChatPacket::EChatPacketMessage>(8), -1)));
    mPlayer->disconnect();
    mServer->getPlayers()->remove(mPlayer);
    mDisconnected = true;
}

uint64_t ServerGamePacketListenerImpl::getMilliseconds()
{
    return System::processTimeInMilliSecs();
}

void ServerGamePacketListenerImpl::NameItemOnAnvil(
    const boost::shared_ptr<ServerboundCustomPayloadPacket>& packet, bool useName)
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
                menu->setItemName(name, useName);
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
    INetworkPlayer* networkPlayer = getNetworkPlayer();
    return networkPlayer != 0 && networkPlayer->IsGuest();
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
