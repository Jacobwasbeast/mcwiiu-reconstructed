#include "game/network/PacketListener.hpp"

template <typename T>
static void dispatchUnhandled(PacketListener* listener, const boost::shared_ptr<T>& packet)
{
    boost::shared_ptr<void> holder(packet);
    boost::shared_ptr<Packet> base(holder, static_cast<Packet*>(holder.get()));
    listener->onUnhandledPacket(base);
}

PacketListener::~PacketListener()
{
}

void PacketListener::onUnhandledPacket(boost::shared_ptr<Packet> packet)
{
}

#define onUnhandledPacket(packet) dispatchUnhandled(this, packet)

void PacketListener::onDisconnect(DisconnectPacket::eDisconnectReason reason, void* userData)
{
}

bool PacketListener::canHandleAsyncPackets()
{
    return false;
}

void PacketListener::handleAcceptedLogin(boost::shared_ptr<ClientboundLoginPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleGetInfo(boost::shared_ptr<GetInfoPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAddEntity(boost::shared_ptr<ClientboundAddEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAddExperienceOrb(boost::shared_ptr<ClientboundAddExperienceOrbPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAddGlobalEntity(boost::shared_ptr<ClientboundAddGlobalEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAddMob(boost::shared_ptr<ClientboundAddMobPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAddPainting(boost::shared_ptr<ClientboundAddPaintingPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAddPlayer(boost::shared_ptr<ClientboundAddPlayerPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAnimate(boost::shared_ptr<ClientboundAnimatePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAnimate(boost::shared_ptr<ServerboundSwingPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAwardStat(boost::shared_ptr<ClientboundAwardStatPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBlockDestruction(boost::shared_ptr<ClientboundBlockDestructionPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBlockCollectionDestruction(boost::shared_ptr<ClientboundBlockCollectionDestructionPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleOpenSignEditor(boost::shared_ptr<ClientboundOpenSignEditorPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBlockEntityData(boost::shared_ptr<ClientboundBlockEntityDataPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBlockEvent(boost::shared_ptr<ClientboundBlockEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBlockUpdate(boost::shared_ptr<ClientboundBlockUpdatePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBlockRegionUpdate(boost::shared_ptr<BlockRegionUpdatePacket> packet)
{
}

void PacketListener::handleChat(boost::shared_ptr<ClientboundChatPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChat(boost::shared_ptr<ServerboundChatPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChatAutoComplete(boost::shared_ptr<ClientboundChatAutoCompletePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChatAutoComplete(boost::shared_ptr<ServerboundChatAutoCompletePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChunkBlocksUpdate(boost::shared_ptr<ClientboundChunkBlocksUpdatePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChunkVisibility(boost::shared_ptr<ChunkVisibilityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChunkVisibilityArea(boost::shared_ptr<ChunkVisibilityAreaPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMapItemData(boost::shared_ptr<ClientboundMapItemDataPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerAck(boost::shared_ptr<ClientboundContainerAckPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerAck(boost::shared_ptr<ServerboundContainerAckPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerClose(boost::shared_ptr<ClientboundContainerClosePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerClose(boost::shared_ptr<ServerboundContainerClosePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerContent(boost::shared_ptr<ClientboundContainerSetContentPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerOpen(boost::shared_ptr<ClientboundContainerOpenPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerSetData(boost::shared_ptr<ClientboundContainerSetDataPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerSetSlot(boost::shared_ptr<ClientboundContainerSetSlotPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleCustomPayload(boost::shared_ptr<ClientboundCustomPayloadPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleCustomPayload(boost::shared_ptr<ServerboundCustomPayloadPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleDisconnect(boost::shared_ptr<DisconnectPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleEntityActionAtPosition(boost::shared_ptr<ClientboundPlayerSleepPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleEntityEvent(boost::shared_ptr<ClientboundEntityEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleEntityLinkPacket(boost::shared_ptr<ClientboundSetEntityLinkPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetEntityPassengersPacket(boost::shared_ptr<ClientboundSetPassengersPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleExplosion(boost::shared_ptr<ClientboundExplodePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleGameEvent(boost::shared_ptr<ClientboundGameEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleKeepAlive(boost::shared_ptr<ClientboundKeepAlivePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleKeepAlive(boost::shared_ptr<ServerboundKeepAlivePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleLevelEvent(boost::shared_ptr<ClientboundLevelEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMoveEntity(boost::shared_ptr<ClientboundMoveEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMoveEntitySmall(boost::shared_ptr<MoveEntityPacketSmall> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMovePlayer(boost::shared_ptr<ClientboundPlayerPositionPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMovePlayer(boost::shared_ptr<ServerboundMovePlayerPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleParticleEvent(boost::shared_ptr<ClientboundLevelParticlesPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerAbilities(boost::shared_ptr<ClientboundPlayerAbilitiesPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerAbilities(boost::shared_ptr<ServerboundPlayerAbilitiesPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerInfo(boost::shared_ptr<PlayerInfoPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePreLogin(boost::shared_ptr<ClientboundPreLoginPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePreLogin(boost::shared_ptr<ServerboundPreLoginPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleRemoveEntity(boost::shared_ptr<ClientboundRemoveEntitiesPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleRemoveMobEffect(boost::shared_ptr<ClientboundRemoveMobEffectPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleRespawn(boost::shared_ptr<ClientboundRespawnPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleRotateMob(boost::shared_ptr<ClientboundRotateHeadPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetCarriedItem(boost::shared_ptr<ClientboundSetCarriedItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetCarriedItem(boost::shared_ptr<ServerboundSetCarriedItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetEntityData(boost::shared_ptr<ClientboundSetEntityDataPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetEntityMotion(boost::shared_ptr<ClientboundSetEntityMotionPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetEquippedItem(boost::shared_ptr<ClientboundSetEquippedItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetExperience(boost::shared_ptr<ClientboundSetExperiencePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetHealth(boost::shared_ptr<ClientboundSetHealthPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetPlayerTeamPacket(boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetSpawn(boost::shared_ptr<ClientboundSetSpawnPositionPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetTime(boost::shared_ptr<ClientboundSetTimePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSoundEvent(boost::shared_ptr<ClientboundSoundPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTakeItemEntity(boost::shared_ptr<ClientboundTakeItemEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTeleportEntity(boost::shared_ptr<ClientboundTeleportEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleUpdateAttributes(boost::shared_ptr<ClientboundUpdateAttributesPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleUpdateMobEffect(boost::shared_ptr<ClientboundUpdateMobEffectPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerCombat(boost::shared_ptr<ClientboundPlayerCombatPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleChangeDifficulty(boost::shared_ptr<ClientboundChangeDifficultyPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetCamera(boost::shared_ptr<ClientboundSetCameraPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetBorder(boost::shared_ptr<ClientboundSetBorderPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetTitles(boost::shared_ptr<ClientboundSetTitlesPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTabListCustomisation(boost::shared_ptr<ClientboundTabListPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleResourcePack(boost::shared_ptr<ClientboundResourcePackPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleBossUpdate(boost::shared_ptr<ClientboundBossEventPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleItemCooldown(boost::shared_ptr<ClientboundCooldownPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMoveVehicle(boost::shared_ptr<ClientboundMoveVehiclePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMoveVehicle(boost::shared_ptr<ServerboundMoveVehiclePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleServerSettingsChanged(boost::shared_ptr<ServerSettingsChangedPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTexture(boost::shared_ptr<TexturePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTextureAndGeometry(boost::shared_ptr<TextureAndGeometryPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleUpdateProgress(boost::shared_ptr<UpdateProgressPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTextureChange(boost::shared_ptr<TextureChangePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTextureAndGeometryChange(boost::shared_ptr<TextureAndGeometryChangePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleUpdateGameRuleProgressPacket(boost::shared_ptr<UpdateGameRuleProgressPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleXZ(boost::shared_ptr<XZPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleScoreboardPacket(boost::shared_ptr<ScoreboardPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleGameMode(boost::shared_ptr<GameModePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMapSelectInfo(boost::shared_ptr<MapSelectInfoPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerReady(boost::shared_ptr<PlayerReadyPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePowerup(boost::shared_ptr<ClientboundPowerupPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleDamageIndicator(boost::shared_ptr<ClientboundDamageIndicatorPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleMiniGamePlayerSettingsUpdatePacket(boost::shared_ptr<ClientboundMGPlayerSettingsUpdatePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleClientCommand(boost::shared_ptr<ServerboundClientCommandPacket> packet)
{
}

void PacketListener::handleClientInformation(boost::shared_ptr<ServerboundClientInformationPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerButtonClick(boost::shared_ptr<ServerboundContainerButtonClickPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleContainerClick(boost::shared_ptr<ServerboundContainerClickPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleInteract(boost::shared_ptr<ServerboundInteractPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerAction(boost::shared_ptr<ServerboundPlayerActionPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerCommand(boost::shared_ptr<ServerboundPlayerCommandPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePlayerInput(boost::shared_ptr<ServerboundPlayerInputPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSetCreativeModeSlot(boost::shared_ptr<ServerboundSetCreativeModeSlotPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleSignUpdate(boost::shared_ptr<ServerboundSignUpdatePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleUseItemOn(boost::shared_ptr<ServerboundUseItemOnPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleUseItem(boost::shared_ptr<ServerboundUseItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTeleportToEntityPacket(boost::shared_ptr<ServerboundTeleportToEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleResourcePackResponse(boost::shared_ptr<ServerboundResourcePackPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handlePaddleBoat(boost::shared_ptr<ServerboundPaddleBoatPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleAcceptTeleportPacket(boost::shared_ptr<ServerboundAcceptTeleportationPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleCraftItem(boost::shared_ptr<CraftItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleTradeItem(boost::shared_ptr<TradeItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleDebugOptions(boost::shared_ptr<DebugOptionsPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleKickPlayer(boost::shared_ptr<KickPlayerPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleGameCommand(boost::shared_ptr<GameCommandPacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleVote(boost::shared_ptr<VotePacket> packet)
{
    onUnhandledPacket(packet);
}

void PacketListener::handleClientboundSetPlayerTeamPacket(boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet)
{
    onUnhandledPacket(packet);
}

bool PacketListener::isServerPacketListener()
{
    return false;
}

#undef onUnhandledPacket
