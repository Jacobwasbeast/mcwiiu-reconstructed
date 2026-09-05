#ifndef GAME_NETWORK_FAKECLIENTPACKETLISTENER_HPP
#define GAME_NETWORK_FAKECLIENTPACKETLISTENER_HPP

#include <boost/shared_ptr.hpp>
#include "game/network/ClientPacketListener.hpp"

class Minecraft;
class Socket;
class Connection;
class ClientboundPlayerPositionPacket;

class FakeClientPacketListener : public ClientPacketListener
{
public:
    FakeClientPacketListener(Minecraft* minecraft, Socket* socket, int playerId);
    virtual ~FakeClientPacketListener();
    void tick();
    void setDirection(double);
    virtual void handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet);
    virtual void handleAddEntity(boost::shared_ptr<ClientboundAddEntityPacket> packet);
    virtual void handleAddExperienceOrb(boost::shared_ptr<ClientboundAddExperienceOrbPacket> packet);
    virtual void handleAddGlobalEntity(boost::shared_ptr<ClientboundAddGlobalEntityPacket> packet);
    virtual void handleAddPainting(boost::shared_ptr<ClientboundAddPaintingPacket> packet);
    virtual void handleSetEntityMotion(boost::shared_ptr<ClientboundSetEntityMotionPacket> packet);
    virtual void handleSetEntityData(boost::shared_ptr<ClientboundSetEntityDataPacket> packet);
    virtual void handleAddPlayer(boost::shared_ptr<ClientboundAddPlayerPacket> packet);
    virtual void handleTeleportEntity(boost::shared_ptr<ClientboundTeleportEntityPacket> packet);
    virtual void handleSetCarriedItem(boost::shared_ptr<ClientboundSetCarriedItemPacket> packet);
    virtual void handleMoveEntity(boost::shared_ptr<ClientboundMoveEntityPacket> packet);
    virtual void handleRotateMob(boost::shared_ptr<ClientboundRotateHeadPacket> packet);
    virtual void handleMoveEntitySmall(boost::shared_ptr<MoveEntityPacketSmall> packet);
    virtual void handleRemoveEntity(boost::shared_ptr<ClientboundRemoveEntitiesPacket> packet);
    virtual void handleMovePlayer(boost::shared_ptr<ClientboundPlayerPositionPacket> packet);
    virtual void handleChunkVisibilityArea(boost::shared_ptr<ChunkVisibilityAreaPacket> packet);
    virtual void handleChunkVisibility(boost::shared_ptr<ChunkVisibilityPacket> packet);
    virtual void handleChunkBlocksUpdate(boost::shared_ptr<ClientboundChunkBlocksUpdatePacket> packet);
    virtual void handleBlockRegionUpdate(boost::shared_ptr<BlockRegionUpdatePacket> packet);
    virtual void handleBlockUpdate(boost::shared_ptr<ClientboundBlockUpdatePacket> packet);
    virtual void handleDisconnect(boost::shared_ptr<DisconnectPacket> packet);
    virtual void onDisconnect(DisconnectPacket::eDisconnectReason reason, void* userData);
    virtual void handleTakeItemEntity(boost::shared_ptr<ClientboundTakeItemEntityPacket> packet);
    virtual void handleChat(boost::shared_ptr<ClientboundChatPacket> packet);
    virtual void handleAnimate(boost::shared_ptr<ClientboundAnimatePacket> packet);
    virtual void handleEntityActionAtPosition(boost::shared_ptr<ClientboundPlayerSleepPacket> packet);
    virtual void handlePreLogin(boost::shared_ptr<ClientboundPreLoginPacket> packet);
    virtual void handleAddMob(boost::shared_ptr<ClientboundAddMobPacket> packet);
    virtual void handleSetTime(boost::shared_ptr<ClientboundSetTimePacket> packet);
    virtual void handleSetSpawn(boost::shared_ptr<ClientboundSetSpawnPositionPacket> packet);
    virtual void handleEntityLinkPacket(boost::shared_ptr<ClientboundSetEntityLinkPacket> packet);
    virtual void handleEntityEvent(boost::shared_ptr<ClientboundEntityEventPacket> packet);
    virtual void handleDamageIndicator(boost::shared_ptr<ClientboundDamageIndicatorPacket> packet);
    virtual void handleSetHealth(boost::shared_ptr<ClientboundSetHealthPacket> packet);
    virtual void handleSetExperience(boost::shared_ptr<ClientboundSetExperiencePacket> packet);
    virtual void handleRespawn(boost::shared_ptr<ClientboundRespawnPacket> packet);
    virtual void handleExplosion(boost::shared_ptr<ClientboundExplodePacket> packet);
    virtual void handleContainerOpen(boost::shared_ptr<ClientboundContainerOpenPacket> packet, int type, int slots, int id);
    virtual void handleContainerSetSlot(boost::shared_ptr<ClientboundContainerSetSlotPacket> packet);
    virtual void handleContainerAck(boost::shared_ptr<ClientboundContainerAckPacket> packet);
    virtual void handleContainerContent(boost::shared_ptr<ClientboundContainerSetContentPacket> packet);
    virtual void handleOpenSignEditor(boost::shared_ptr<ClientboundOpenSignEditorPacket> packet);
    virtual void handleBlockEntityData(boost::shared_ptr<ClientboundBlockEntityDataPacket> packet);
    virtual void handleContainerSetData(boost::shared_ptr<ClientboundContainerSetDataPacket> packet);
    virtual void handleSetEquippedItem(boost::shared_ptr<ClientboundSetEquippedItemPacket> packet);
    virtual void handleContainerClose(boost::shared_ptr<ClientboundContainerClosePacket> packet);
    virtual void handleBlockEvent(boost::shared_ptr<ClientboundBlockEventPacket> packet);
    virtual void handleBlockDestruction(boost::shared_ptr<ClientboundBlockDestructionPacket> packet);
    virtual bool canHandleAsyncPackets();
    virtual void handleGameEvent(boost::shared_ptr<ClientboundGameEventPacket> packet);
    virtual void handleMapItemData(boost::shared_ptr<ClientboundMapItemDataPacket> packet);
    virtual void handleLevelEvent(boost::shared_ptr<ClientboundLevelEventPacket> packet);
    virtual void handleAwardStat(boost::shared_ptr<ClientboundAwardStatPacket> packet);
    virtual void handleUpdateMobEffect(boost::shared_ptr<ClientboundUpdateMobEffectPacket> packet);
    virtual void handlePlayerCombat(boost::shared_ptr<ClientboundPlayerCombatPacket> packet);
    virtual void handleChangeDifficulty(boost::shared_ptr<ClientboundChangeDifficultyPacket> packet);
    virtual void handleSetCamera(boost::shared_ptr<ClientboundSetCameraPacket> packet);
    virtual void handleSetBorder(boost::shared_ptr<ClientboundSetBorderPacket> packet);
    virtual void handleSetTitles(boost::shared_ptr<ClientboundSetTitlesPacket> packet);
    virtual void handleTabListCustomisation(boost::shared_ptr<ClientboundTabListPacket> packet);
    virtual void handleRemoveMobEffect(boost::shared_ptr<ClientboundRemoveMobEffectPacket> packet);
    virtual bool isServerPacketListener();
    virtual void handlePlayerInfo(boost::shared_ptr<PlayerInfoPacket> packet);
    virtual void handleKeepAlive(boost::shared_ptr<ClientboundKeepAlivePacket> packet);
    virtual void handlePlayerAbilities(boost::shared_ptr<ClientboundPlayerAbilitiesPacket> packet);
    virtual void handleSoundEvent(boost::shared_ptr<ClientboundSoundPacket> packet);
    virtual void handleCustomPayload(boost::shared_ptr<ClientboundCustomPayloadPacket> packet);
    virtual Connection* getConnection();
    virtual void handleServerSettingsChanged(boost::shared_ptr<ServerSettingsChangedPacket> packet);
    virtual void handleTexture(boost::shared_ptr<TexturePacket> packet);
    virtual void handleTextureAndGeometry(boost::shared_ptr<TextureAndGeometryPacket> packet);
    virtual void handleUpdateProgress(boost::shared_ptr<UpdateProgressPacket> packet);
    virtual void handleTextureChange(boost::shared_ptr<TextureChangePacket> packet);
    virtual void handleTextureAndGeometryChange(boost::shared_ptr<TextureAndGeometryChangePacket> packet);
    virtual void handleUpdateGameRuleProgressPacket(boost::shared_ptr<UpdateGameRuleProgressPacket> packet);
    virtual void handleXZ(boost::shared_ptr<XZPacket> packet);
    virtual void handleScoreboardPacket(boost::shared_ptr<ScoreboardPacket> packet);
    virtual void handleClientboundSetPlayerTeamPacket(boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet);
    virtual void handleParticleEvent(boost::shared_ptr<ClientboundLevelParticlesPacket> packet);
    virtual void handleUpdateAttributes(boost::shared_ptr<ClientboundUpdateAttributesPacket> packet);
    virtual void handleGameMode(boost::shared_ptr<GameModePacket> packet);
    virtual void handleMapSelectInfo(boost::shared_ptr<MapSelectInfoPacket> packet);
    virtual void handlePlayerReady(boost::shared_ptr<PlayerReadyPacket> packet);
    virtual void handlePowerup(boost::shared_ptr<ClientboundPowerupPacket> packet);
private:
    Connection* m_connection;
    int m_unknown0;
    int m_unknown1;
    int m_unknown2;
    bool m_started;
    double m_x;
    double m_y;
    double m_z;
};

#endif
