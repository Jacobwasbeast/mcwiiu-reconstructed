#include "game/network/ClientPacketListener.hpp"
#include "game/network/Connection.hpp"

ClientPacketListener::ClientPacketListener(Minecraft* minecraft, Socket* socket, int playerId)
    : m_closed(false), m_connection(0), m_minecraft(minecraft), m_level(0), m_started(false),
      m_socketOpen(false), m_playerId(playerId), m_random(), m_name(L""), m_savedDataStorage(0)
{
    InitializeCriticalSection(&m_mutex);
    if (socket != 0)
    {
        m_socketOpen = true;
        m_connection = new Connection(socket, std::wstring(L"Client"), this);
    }
}

ClientPacketListener::~ClientPacketListener()
{
    if (m_connection != 0)
        delete m_connection;
    m_connection = 0;
    DeleteCriticalSection(&m_mutex);
}

void ClientPacketListener::handleBlockEntityDataInternal(boost::shared_ptr<ClientboundBlockEntityDataPacket> packet, Level* level)
{
    (void)packet;
    (void)level;
}

uint8_t ClientPacketListener::isStarted() { return m_started ? 1 : 0; }
uint8_t ClientPacketListener::isClosed() { return m_closed ? 1 : 0; }
uint32_t ClientPacketListener::getSocket() { return m_connection == 0 ? 0 : 1; }
uint8_t ClientPacketListener::getAndSetClosed()
{
    EnterCriticalSection(&m_mutex);
    const uint8_t wasClosed = m_closed ? 1 : 0;
    m_closed = true;
    LeaveCriticalSection(&m_mutex);
    return wasClosed;
}

void ClientPacketListener::tick()
{
    if (!m_closed && m_connection != 0)
        m_connection->tick();
    if (m_connection != 0)
        m_connection->flush();
}

uint32_t ClientPacketListener::getNetworkPlayer()
{
    return 0;
}

bool ClientPacketListener::IsPrimaryPlayer()
{
    return m_playerId == 0;
}

void ClientPacketListener::displayPrivilegeChanges(boost::shared_ptr<MultiplayerLocalPlayer> player, uint32_t changes)
{
    (void)player;
    (void)changes;
}

void ClientPacketListener::handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet) { (void)packet; }
uint32_t ClientPacketListener::getEntity(int entityId) { (void)entityId; return 0; }
void ClientPacketListener::checkDeferredEntityLinkPackets(int entityId) { (void)entityId; }

#define CLIENT_PACKET_VOID(name, type) \
void ClientPacketListener::name(boost::shared_ptr<type> packet) { (void)packet; }

CLIENT_PACKET_VOID(handleAddEntity, ClientboundAddEntityPacket)
CLIENT_PACKET_VOID(handleAddExperienceOrb, ClientboundAddExperienceOrbPacket)
CLIENT_PACKET_VOID(handleAddGlobalEntity, ClientboundAddGlobalEntityPacket)
CLIENT_PACKET_VOID(handleAddPainting, ClientboundAddPaintingPacket)
CLIENT_PACKET_VOID(handleSetEntityMotion, ClientboundSetEntityMotionPacket)
CLIENT_PACKET_VOID(handleSetEntityData, ClientboundSetEntityDataPacket)

void ClientPacketListener::send(boost::shared_ptr<Packet> packet)
{
    if (m_connection != 0)
        m_connection->send(packet);
}

CLIENT_PACKET_VOID(handleAddPlayer, ClientboundAddPlayerPacket)
CLIENT_PACKET_VOID(handleTeleportEntity, ClientboundTeleportEntityPacket)
CLIENT_PACKET_VOID(handleSetCarriedItem, ClientboundSetCarriedItemPacket)
CLIENT_PACKET_VOID(handleMoveEntity, ClientboundMoveEntityPacket)
CLIENT_PACKET_VOID(handleRotateMob, ClientboundRotateHeadPacket)
CLIENT_PACKET_VOID(handleMoveEntitySmall, MoveEntityPacketSmall)
CLIENT_PACKET_VOID(handleRemoveEntity, ClientboundRemoveEntitiesPacket)
CLIENT_PACKET_VOID(handleMovePlayer, ClientboundPlayerPositionPacket)
CLIENT_PACKET_VOID(handleChunkVisibilityArea, ChunkVisibilityAreaPacket)
CLIENT_PACKET_VOID(handleChunkVisibility, ChunkVisibilityPacket)
CLIENT_PACKET_VOID(handleChunkBlocksUpdate, ClientboundChunkBlocksUpdatePacket)
CLIENT_PACKET_VOID(handleBlockRegionUpdate, BlockRegionUpdatePacket)
CLIENT_PACKET_VOID(handleBlockUpdate, ClientboundBlockUpdatePacket)
CLIENT_PACKET_VOID(handleDisconnect, DisconnectPacket)

uint32_t ClientPacketListener::ExitGameAndSaveReturned(void* context, int result, C4JStorage::EMessageResult messageResult)
{
    (void)context;
    (void)result;
    (void)messageResult;
    return 0;
}

uint32_t ClientPacketListener::HostDisconnectReturned(void* context, int result, C4JStorage::EMessageResult messageResult)
{
    (void)context;
    (void)result;
    (void)messageResult;
    return 0;
}

void ClientPacketListener::onDisconnect(DisconnectPacket::eDisconnectReason reason, void* userData)
{
    (void)reason;
    (void)userData;
    close();
}

void ClientPacketListener::sendAndDisconnect(boost::shared_ptr<Packet> packet)
{
    send(packet);
    close();
}

CLIENT_PACKET_VOID(handleTakeItemEntity, ClientboundTakeItemEntityPacket)
std::wstring ClientPacketListener::GetDisplayNameByGamertag(std::wstring gamertag) { return gamertag; }
CLIENT_PACKET_VOID(handleChat, ClientboundChatPacket)
uint32_t ClientPacketListener::isLocalPlayer(int entityId) { return entityId == m_playerId ? 1 : 0; }
CLIENT_PACKET_VOID(handleAnimate, ClientboundAnimatePacket)
CLIENT_PACKET_VOID(handleEntityActionAtPosition, ClientboundPlayerSleepPacket)
CLIENT_PACKET_VOID(handlePreLogin, ClientboundPreLoginPacket)

void ClientPacketListener::close()
{
    if (m_connection != 0)
        m_connection->close(0);
    m_closed = true;
}

CLIENT_PACKET_VOID(handleAddMob, ClientboundAddMobPacket)
CLIENT_PACKET_VOID(handleSetTime, ClientboundSetTimePacket)
CLIENT_PACKET_VOID(handleSetSpawn, ClientboundSetSpawnPositionPacket)
CLIENT_PACKET_VOID(handleSetEntityPassengersPacket, ClientboundSetPassengersPacket)
CLIENT_PACKET_VOID(handleEntityLinkPacket, ClientboundSetEntityLinkPacket)
CLIENT_PACKET_VOID(handleEntityEvent, ClientboundEntityEventPacket)
CLIENT_PACKET_VOID(handleDamageIndicator, ClientboundDamageIndicatorPacket)
CLIENT_PACKET_VOID(handleMiniGamePlayerSettingsUpdatePacket, ClientboundMGPlayerSettingsUpdatePacket)
CLIENT_PACKET_VOID(handleSetHealth, ClientboundSetHealthPacket)
CLIENT_PACKET_VOID(handleSetExperience, ClientboundSetExperiencePacket)
CLIENT_PACKET_VOID(handleRespawn, ClientboundRespawnPacket)
CLIENT_PACKET_VOID(handleExplosion, ClientboundExplodePacket)

void ClientPacketListener::handleContainerOpen(boost::shared_ptr<ClientboundContainerOpenPacket> packet, int type, int slots, int id)
{
    (void)packet;
    (void)type;
    (void)slots;
    (void)id;
}

CLIENT_PACKET_VOID(handleContainerSetSlot, ClientboundContainerSetSlotPacket)
CLIENT_PACKET_VOID(handleContainerAck, ClientboundContainerAckPacket)
CLIENT_PACKET_VOID(handleContainerContent, ClientboundContainerSetContentPacket)
CLIENT_PACKET_VOID(handleOpenSignEditor, ClientboundOpenSignEditorPacket)
CLIENT_PACKET_VOID(handleBlockEntityData, ClientboundBlockEntityDataPacket)
CLIENT_PACKET_VOID(handleContainerSetData, ClientboundContainerSetDataPacket)
CLIENT_PACKET_VOID(handleSetEquippedItem, ClientboundSetEquippedItemPacket)
CLIENT_PACKET_VOID(handleContainerClose, ClientboundContainerClosePacket)
CLIENT_PACKET_VOID(handleBlockEvent, ClientboundBlockEventPacket)
CLIENT_PACKET_VOID(handleBlockDestruction, ClientboundBlockDestructionPacket)
CLIENT_PACKET_VOID(handleBlockCollectionDestruction, ClientboundBlockCollectionDestructionPacket)
bool ClientPacketListener::canHandleAsyncPackets() { return false; }
CLIENT_PACKET_VOID(handleGameEvent, ClientboundGameEventPacket)
CLIENT_PACKET_VOID(handleMapItemData, ClientboundMapItemDataPacket)
CLIENT_PACKET_VOID(handleLevelEvent, ClientboundLevelEventPacket)
CLIENT_PACKET_VOID(handleAwardStat, ClientboundAwardStatPacket)
CLIENT_PACKET_VOID(handleUpdateMobEffect, ClientboundUpdateMobEffectPacket)
CLIENT_PACKET_VOID(handlePlayerCombat, ClientboundPlayerCombatPacket)
CLIENT_PACKET_VOID(handleChangeDifficulty, ClientboundChangeDifficultyPacket)
CLIENT_PACKET_VOID(handleSetCamera, ClientboundSetCameraPacket)
CLIENT_PACKET_VOID(handleSetBorder, ClientboundSetBorderPacket)
CLIENT_PACKET_VOID(handleSetTitles, ClientboundSetTitlesPacket)
CLIENT_PACKET_VOID(handleTabListCustomisation, ClientboundTabListPacket)
CLIENT_PACKET_VOID(handleRemoveMobEffect, ClientboundRemoveMobEffectPacket)
bool ClientPacketListener::isServerPacketListener() { return false; }
CLIENT_PACKET_VOID(handlePlayerInfo, PlayerInfoPacket)
CLIENT_PACKET_VOID(handleKeepAlive, ClientboundKeepAlivePacket)
CLIENT_PACKET_VOID(handlePlayerAbilities, ClientboundPlayerAbilitiesPacket)
CLIENT_PACKET_VOID(handleSoundEvent, ClientboundSoundPacket)
CLIENT_PACKET_VOID(handleMoveVehicle, ClientboundMoveVehiclePacket)
CLIENT_PACKET_VOID(handleBossUpdate, ClientboundBossEventPacket)
CLIENT_PACKET_VOID(handleItemCooldown, ClientboundCooldownPacket)
CLIENT_PACKET_VOID(handleCustomPayload, ClientboundCustomPayloadPacket)
Connection* ClientPacketListener::getConnection() { return m_connection; }
CLIENT_PACKET_VOID(handleServerSettingsChanged, ServerSettingsChangedPacket)
CLIENT_PACKET_VOID(handleTexture, TexturePacket)
CLIENT_PACKET_VOID(handleTextureAndGeometry, TextureAndGeometryPacket)
CLIENT_PACKET_VOID(handleUpdateProgress, UpdateProgressPacket)
CLIENT_PACKET_VOID(handleTextureChange, TextureChangePacket)
CLIENT_PACKET_VOID(handleTextureAndGeometryChange, TextureAndGeometryChangePacket)
CLIENT_PACKET_VOID(handleUpdateGameRuleProgressPacket, UpdateGameRuleProgressPacket)
CLIENT_PACKET_VOID(handleXZ, XZPacket)
CLIENT_PACKET_VOID(handleScoreboardPacket, ScoreboardPacket)
CLIENT_PACKET_VOID(handleClientboundSetPlayerTeamPacket, ClientboundSetPlayerTeamPacket)
CLIENT_PACKET_VOID(handleParticleEvent, ClientboundLevelParticlesPacket)
CLIENT_PACKET_VOID(handleUpdateAttributes, ClientboundUpdateAttributesPacket)
void ClientPacketListener::updateTutorialOnGameMode(eTutorial_GameMode gameMode) { (void)gameMode; }
CLIENT_PACKET_VOID(handleGameMode, GameModePacket)
CLIENT_PACKET_VOID(handleMapSelectInfo, MapSelectInfoPacket)
CLIENT_PACKET_VOID(handlePlayerReady, PlayerReadyPacket)
CLIENT_PACKET_VOID(handlePowerup, ClientboundPowerupPacket)
CLIENT_PACKET_VOID(handleVote, VotePacket)
void ClientPacketListener::staticCtor() {}

#undef CLIENT_PACKET_VOID

ClientPacketListener::DeferredEntityLinkPacket::DeferredEntityLinkPacket(boost::shared_ptr<ClientboundSetEntityLinkPacket> packet)
    : entityId(0), linkId(0), linkType(0)
{
    (void)packet;
}
