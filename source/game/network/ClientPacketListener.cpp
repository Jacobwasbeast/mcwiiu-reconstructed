#include "game/network/ClientPacketListener.hpp"
#include "game/network/Connection.hpp"
#include "game/network/ClientboundRespawnPacket.hpp"
#include "game/network/ClientboundSetTimePacket.hpp"
#include "game/network/ClientboundSetCarriedItemPacket.hpp"
#include "game/network/ClientboundRemoveEntitiesPacket.hpp"
#include "game/network/ClientboundRotateHeadPacket.hpp"
#include "game/network/ClientboundBlockEventPacket.hpp"
#include "game/network/ClientboundSetEquippedItemPacket.hpp"
#include "game/network/ClientboundAddExperienceOrbPacket.hpp"
#include "game/network/ClientboundPowerupPacket.hpp"
#include "game/network/ClientboundBlockCollectionDestructionPacket.hpp"
#include "game/network/ClientboundAddEntityPacket.hpp"
#include "game/network/PlayerInfoPacket.hpp"
#include "game/core/Boat.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/core/EyeOfEnderSignal.hpp"
#include "game/core/ThrownExpBottle.hpp"
#include "game/entity/mob/LeashFenceKnotEntity.hpp"
#include "game/core/ArmorStand.hpp"
#include "game/core/EvocationFangs.hpp"
#include "game/entity/projectile/SpectralArrow.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/core/Snowball.hpp"
#include "game/core/ThrownEgg.hpp"
#include "game/entity/projectile/LargeFireball.hpp"
#include "game/entity/projectile/SmallFireball.hpp"
#include "game/core/ThrownEnderpearl.hpp"
#include "game/core/WitherSkull.hpp"
#include "game/core/ShulkerBullet.hpp"
#include "game/core/LlamaSpit.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Item.hpp"
#include "game/network/ClientboundBlockDestructionPacket.hpp"
#include "game/network/ClientboundContainerClosePacket.hpp"
#include "game/network/ClientboundSetBorderPacket.hpp"
#include "game/network/ClientboundChangeDifficultyPacket.hpp"
#include "game/network/ServerSettingsChangedPacket.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/core/DLCPack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/network/ClientboundDamageIndicatorPacket.hpp"
#include "game/network/ClientboundSetExperiencePacket.hpp"
#include "game/network/ClientboundCooldownPacket.hpp"
#include "game/network/ClientboundSetEntityMotionPacket.hpp"
#include "game/network/ClientboundMoveEntityPacket.hpp"
#include "game/network/ClientboundBlockUpdatePacket.hpp"
#include "game/network/ClientboundLevelEventPacket.hpp"
#include "game/network/ClientboundPlayerAbilitiesPacket.hpp"
#include "game/network/ClientboundSetCameraPacket.hpp"
#include "game/network/ClientboundSetHealthPacket.hpp"
#include "game/network/ClientboundContainerSetDataPacket.hpp"
#include "game/network/BlockRegionUpdatePacket.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/item/ItemCooldowns.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/core/Gui.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringUtils.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Stats.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameType.hpp"
#include "game/util/StringUtils.hpp"
#include "game/network/ClientboundAwardStatPacket.hpp"
#include "game/core/BossHealthOverlay.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/Voteable.hpp"
#include "game/core/Voteables.hpp"
#include "game/network/VotePacket.hpp"
#include "game/network/UpdateProgressPacket.hpp"
#include "game/network/PlayerReadyPacket.hpp"
#include "game/network/ChunkVisibilityPacket.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/CauldronBlockEntity.hpp"
#include "game/world/block/ConduitBlockEntity.hpp"

namespace
{
    bool blockEntityTypeMatches(BlockEntity* entity, unsigned long long expectedType)
    {
        return entity != NULL && entity->GetType() == expectedType;
    }
}

ClientPacketListener::ClientPacketListener(Minecraft* minecraft, Socket* socket, int playerId)
    : m_closed(false), m_connection(0), mRemoteAddress(L""), m_socketOpen(false),
      m_minecraft(minecraft), m_level(0), m_started(false), m_playerId(playerId), m_random(),
      m_savedDataStorage(0)
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

void ClientPacketListener::handleBlockEntityDataInternal(
    boost::shared_ptr<ClientboundBlockEntityDataPacket> packet, Level* level)
{
    if (level == NULL || packet == NULL)
    {
        return;
    }

    BlockPos pos = packet->getPos();
    if (!level->hasChunkAt(pos))
    {
        return;
    }

    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity == NULL)
    {
        return;
    }

    const int type = packet->getType();
    CompoundTag* tag = packet->getTag();

    if (type == 0xc)
    {
        if (blockEntityTypeMatches(blockEntity.get(), 0x1000211ULL))
        {
            boost::shared_ptr<CauldronBlockEntity> cauldron =
                boost::static_pointer_cast<CauldronBlockEntity>(blockEntity);
            cauldron->loadFromPacket(level, pos, tag);
        }
        return;
    }

    if (type == 0xd)
    {
        if (blockEntityTypeMatches(blockEntity.get(), 0x1200212ULL))
        {
            boost::shared_ptr<ConduitBlockEntity> conduit =
                boost::static_pointer_cast<ConduitBlockEntity>(blockEntity);
            conduit->loadFromPacket(level, pos, tag);
        }
        return;
    }

    bool loadTag = false;
    switch (type)
    {
    case 0:
        break;
    case 1:
        loadTag = blockEntityTypeMatches(blockEntity.get(), 0x1200201ULL);
        break;
    case 2:
        loadTag = blockEntityTypeMatches(blockEntity.get(), 0x1000209ULL);
        break;
    case 3:
        loadTag = blockEntityTypeMatches(blockEntity.get(), 0x1e00204ULL);
        break;
    case 4:
        loadTag = blockEntityTypeMatches(blockEntity.get(), 0x1200207ULL);
        break;
    case 5:
        loadTag = blockEntityTypeMatches(blockEntity.get(), 0x100020dULL);
        break;
    case 6:
        loadTag = blockEntityTypeMatches(blockEntity.get(), 0x100020cULL);
        break;
    case 8:
        loadTag = blockEntityTypeMatches(blockEntity.get(), 0x1300201ULL);
        break;
    case 9:
        loadTag = blockEntityTypeMatches(blockEntity.get(), 0x1000202ULL);
        break;
    case 10:
        loadTag = blockEntityTypeMatches(blockEntity.get(), 0x120020fULL);
        break;
    case 11:
        loadTag = blockEntityTypeMatches(blockEntity.get(), 0x1000210ULL);
        break;
    default:
        break;
    }

    if (loadTag)
    {
        blockEntity->load(tag);
    }
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

INetworkPlayer* ClientPacketListener::getNetworkPlayer()
{
    if (m_connection != 0)
    {
        Socket* socket = (Socket*)getSocket();
        if (socket != 0)
            return socket->getPlayer();
    }
    return 0;
}

bool ClientPacketListener::IsPrimaryPlayer()
{
    return m_playerId == 0;
}

void ClientPacketListener::displayPrivilegeChanges(boost::shared_ptr<MultiplayerLocalPlayer> player, uint32_t changes)
{
}

void ClientPacketListener::handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet) { }
uint32_t ClientPacketListener::getEntity(int entityId) { return 0; }
void ClientPacketListener::checkDeferredEntityLinkPackets(int entityId) { }

void ClientPacketListener::handleAddEntity(boost::shared_ptr<ClientboundAddEntityPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    const double x = static_cast<double>(packet->getX()) * 0.03125;
    const double y = static_cast<double>(packet->getY()) * 0.03125;
    const double z = static_cast<double>(packet->getZ()) * 0.03125;
    boost::shared_ptr<Entity> entity;
    switch (packet->getType())
    {
    case 60: entity.reset(new TippableArrow(level, x, y, z)); break;
    case 61: entity.reset(new Snowball(level, x, y, z)); break;
    case 62: entity.reset(new ThrownEgg(level, x, y, z)); break;
    case 63: entity.reset(new LargeFireball(level, x, y, z, packet->getXa(), packet->getYa(), packet->getZa())); break;
    case 64: entity.reset(new SmallFireball(level, x, y, z, packet->getXa(), packet->getYa(), packet->getZa())); break;
    case 65: entity.reset(new ThrownEnderpearl(level, x, y, z)); break;
    case 66: entity.reset(new WitherSkull(level, x, y, z, packet->getXa(), packet->getYa(), packet->getZa())); break;
    case 67: entity.reset(new ShulkerBullet(level, x, y, z, packet->getXa(), packet->getYa(), packet->getZa())); break;
    case 68: entity.reset(new LlamaSpit(level, x, y, z, packet->getXa(), packet->getYa(), packet->getZa())); break;
    case 70: entity.reset(new FallingBlockEntity(level, x, y, z, Block::stateByIdWithData(packet->getData() & 0xffff))); packet->setData(0); break;
    case 71: entity.reset(new ItemFrame(level, BlockPos((int)x, (int)y, (int)z), Direction::from2DDataValue(packet->getData()))); packet->setData(0); break;
    case 73:
        entity.reset(new ThrownPotion(level, x, y, z,
            not_null_ptr<ItemInstance>(new ItemInstance(Item::byId(packet->getData())))));
        break;
    case 1: entity.reset(new Boat(level, x, y, z)); break;
    case 2: entity.reset(new ItemEntity(level, x, y, z)); break;
    case 3: entity.reset(new AreaEffectCloud(level, x, y, z)); break;
    case 72: entity.reset(new EyeOfEnderSignal(level, x, y, z)); break;
    case 75: entity.reset(new ThrownExpBottle(level, x, y, z)); break;
    case 77: entity.reset(new LeashFenceKnotEntity(level, BlockPos((int)x, (int)y, (int)z))); break;
    case 78: entity.reset(new ArmorStand(level, x, y, z)); break;
    case 79: entity.reset(new EvocationFangs(level, x, y, z,
        packet->getyRot() * (360.0f / 256.0f), 0, boost::shared_ptr<LivingEntity>())); break;
    case 91: entity.reset(new SpectralArrow(level, x, y, z)); break;
    case 94: entity.reset(new ThrownTrident(level, x, y, z)); break;
    default: return;
    }
    entity->setId(packet->getId());
    entity->setPos(x, y, z);
    entity->setRot(packet->getyRot() * (360.0f / 256.0f), packet->getxRot() * (360.0f / 256.0f));
    level->putEntity(packet->getId(), entity);
    checkDeferredEntityLinkPackets(entity->getId());
}

void ClientPacketListener::handleAddExperienceOrb(boost::shared_ptr<ClientboundAddExperienceOrbPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    boost::shared_ptr<ExperienceOrb> orb(new ExperienceOrb(level,
                                                            packet->getX(),
                                                            packet->getY(),
                                                            packet->getZ(),
                                                            packet->getValue()));
    level->putEntity(packet->getId(), orb);
}

void ClientPacketListener::handleAddGlobalEntity(boost::shared_ptr<ClientboundAddGlobalEntityPacket> packet)
{
}

void ClientPacketListener::handleAddPainting(boost::shared_ptr<ClientboundAddPaintingPacket> packet)
{
}

void ClientPacketListener::handleSetEntityMotion(boost::shared_ptr<ClientboundSetEntityMotionPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    boost::shared_ptr<Entity> entity = level->getEntity(packet->getId());
    if (entity != 0)
    {
        entity->mXd = static_cast<double>(packet->getXa()) / 8000.0;
        entity->mYd = static_cast<double>(packet->getYa()) / 8000.0;
        entity->mZd = static_cast<double>(packet->getZa()) / 8000.0;
    }
}

void ClientPacketListener::handleSetEntityData(boost::shared_ptr<ClientboundSetEntityDataPacket> packet)
{
}

void ClientPacketListener::send(boost::shared_ptr<Packet> packet)
{
    if (m_connection != 0)
        m_connection->send(packet);
}

void ClientPacketListener::handleAddPlayer(boost::shared_ptr<ClientboundAddPlayerPacket> packet)
{
}

void ClientPacketListener::handleTeleportEntity(boost::shared_ptr<ClientboundTeleportEntityPacket> packet)
{
}

void ClientPacketListener::handleSetCarriedItem(boost::shared_ptr<ClientboundSetCarriedItemPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player == 0 || player->mInventory == 0)
        return;
    int slot = packet->getSlot();
    if (slot >= 0 && slot < Inventory::getSelectionSize())
        player->mInventory->pickSlot(slot);
}

void ClientPacketListener::handleMoveEntity(boost::shared_ptr<ClientboundMoveEntityPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    boost::shared_ptr<Entity> entity = level->getEntity(packet->getEntityId());
    if (entity == 0)
        return;
    entity->mX += static_cast<double>(packet->getXa()) / 32.0;
    entity->mY += static_cast<double>(packet->getYa()) / 32.0;
    entity->mZ += static_cast<double>(packet->getZa()) / 32.0;
    if (packet->hasRotation())
        entity->setRot(packet->getyRot() * (360.0f / 256.0f), packet->getxRot() * (360.0f / 256.0f));
    entity->mOnGround = packet->isOnGround();
}

void ClientPacketListener::handleRotateMob(boost::shared_ptr<ClientboundRotateHeadPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    boost::shared_ptr<Entity> entity = level->getEntity(packet->getEntityId());
    if (entity != 0)
        entity->setYHeadRot(packet->getYHeadRot() * (360.0f / 256.0f));
}

void ClientPacketListener::handleMoveEntitySmall(boost::shared_ptr<MoveEntityPacketSmall> packet)
{
}

void ClientPacketListener::handleRemoveEntity(boost::shared_ptr<ClientboundRemoveEntitiesPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    for (unsigned int i = 0; i < packet->entityIds.length; ++i)
        level->removeEntity(packet->entityIds.data[i]);
}

void ClientPacketListener::handleMovePlayer(boost::shared_ptr<ClientboundPlayerPositionPacket> packet)
{
}

void ClientPacketListener::handleChunkVisibilityArea(boost::shared_ptr<ChunkVisibilityAreaPacket> packet)
{
}

void ClientPacketListener::handleChunkVisibility(boost::shared_ptr<ChunkVisibilityPacket> packet)
{
    if (m_level != 0 && packet != 0)
    {
        MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
        if (level != 0)
            level->setChunkVisible(packet->x, packet->z, packet->visible, packet->loaded);
    }
}

void ClientPacketListener::handleChunkBlocksUpdate(boost::shared_ptr<ClientboundChunkBlocksUpdatePacket> packet)
{
}

void ClientPacketListener::handleBlockRegionUpdate(boost::shared_ptr<BlockRegionUpdatePacket> packet)
{
    if (packet == 0 || m_level == 0 || packet->GetChanges() == 0)
        return;
    const std::vector<LevelChunk::BlockChange>& changes = *packet->GetChanges();
    for (std::vector<LevelChunk::BlockChange>::const_iterator it = changes.begin(); it != changes.end(); ++it)
        m_level->setBlock(it->GetPos(), it->GetState(), 3, false);
}

void ClientPacketListener::handleBlockUpdate(boost::shared_ptr<ClientboundBlockUpdatePacket> packet)
{
    if (packet != 0 && m_level != 0)
        m_level->setBlock(packet->getPos(), packet->getBlock(), 3, false);
}

void ClientPacketListener::handleDisconnect(boost::shared_ptr<DisconnectPacket> packet)
{
    if (packet == 0 || getAndSetClosed())
        return;
    if (m_connection != 0)
        m_connection->close(packet->reason);
    m_minecraft->mConnectionFailed[m_playerId] = true;
    m_minecraft->mDisconnectReasons[m_playerId] = packet->reason;
    g_pApp->SetDisconnectReason(packet->reason);
    g_pApp->SetAction(m_playerId, static_cast<eXuiAction>(3), (void*)1);
}

int ClientPacketListener::ExitGameAndSaveReturned(void* context, int result, C4JStorage::EMessageResult messageResult)
{
    if (messageResult == static_cast<C4JStorage::EMessageResult>(5))
        gApp.SetAction(result, static_cast<eXuiAction>(1), NULL);
    return 0;
}

int ClientPacketListener::HostDisconnectReturned(void* context, int result, C4JStorage::EMessageResult messageResult)
{
    TexturePack* selected = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    if (selected != TexturePackRepository::defaultTexturePack)
    {
        DLCPack* pack = selected->getDLCPack();
        if (pack != NULL && !pack->hasPurchasedPack())
        {
            gApp.SetAction(result, static_cast<eXuiAction>(3), NULL);
            return 0;
        }
    }

    bool saveExists = false;
    gStorage->DoesSaveExist(&saveExists);
    if (!saveExists)
    {
        gApp.SetAction(result, static_cast<eXuiAction>(3), NULL);
    }
    else
    {
        int parameters[2] = { static_cast<int>(0x4fcbc7e7), static_cast<int>(0xb9a1bf4d) };
        gConsoleUIController.RequestAlertMessage(
            0x3143fcab, static_cast<int>(0xd66b8839), parameters, 2, gApp.GetPrimaryUserIndex(),
            ExitGameAndSaveReturned, NULL, NULL, true);
    }
    return 0;
}

void ClientPacketListener::onDisconnect(DisconnectPacket::eDisconnectReason reason, void* userData)
{
    close();
}

void ClientPacketListener::sendAndDisconnect(boost::shared_ptr<Packet> packet)
{
    send(packet);
    close();
}

void ClientPacketListener::handleTakeItemEntity(boost::shared_ptr<ClientboundTakeItemEntityPacket> packet)
{
}

std::wstring ClientPacketListener::GetDisplayNameByGamertag(std::wstring gamertag) { return gamertag; }
void ClientPacketListener::handleChat(boost::shared_ptr<ClientboundChatPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_minecraft->mGui == 0 ||
        m_playerId < 0 || m_playerId >= 4)
        return;

    const int message = static_cast<int>(packet->getMessage());
    if (message == 12)
    {
        g_pApp->SetGlobalXuiAction(static_cast<eXuiAction>(0x21));
        return;
    }

    std::wstring text;
    const auto replaceFirst = [](std::wstring& value, const std::wstring& key, const std::wstring& replacement)
    {
        const std::wstring::size_type pos = value.find(key);
        if (pos != std::wstring::npos)
            value.replace(pos, key.size(), replacement);
    };
    if ((message == 0 || message == 18 || message == 38 || message == 41 || message == 75) &&
        !packet->strings.empty())
    {
        text = packet->strings[0];
    }
    else
    {
        int stringId = 0;
        switch (message)
        {
        case 1: stringId = 0xD08616D9; break;
        case 2: stringId = 0xFA2A57FA; break;
        case 3: stringId = 0xE08B6A47; break;
        case 4: stringId = 0x16C81B7F; break;
        case 5: stringId = 0xF084EC0C; break;
        case 6: stringId = 0x56C64197; break;
        case 7: stringId = 0xFAB38114; break;
        case 8: stringId = 0x095FCE8B; break;
        case 9: stringId = 0x941D21C3; break;
        case 10: stringId = 0x3677FA23; break;
        case 11: stringId = 0x49B6297D; break;
        case 13: stringId = 0xEFC9FEF0; break;
        case 14: stringId = 0xF4980932; break;
        case 15: stringId = 0xD711D936; break;
        case 16: stringId = 0x5EFF1005; break;
        case 17: stringId = 0x6AE921B6; break;
        case 19: stringId = 0x576593EF; break;
        case 20: stringId = 0xF244DF30; break;
        case 21: stringId = 0x2BF5A7DC; break;
        case 22: stringId = 0xD63D2190; break;
        case 23: stringId = 0x15C8F4AE; break;
        case 24: stringId = 0x6F2C7352; break;
        case 25: if (!packet->ints.empty()) stringId = packet->ints[0]; break;
        case 26: stringId = 0xD98E2236; break;
        case 27: stringId = 0xB04CAD6A; break;
        case 28: stringId = 0x189C795E; break;
        case 29: stringId = 0x4BEC30BF; break;
        case 30: stringId = 0x3C5DA2B4; break;
        case 31: stringId = 0x7A2A89AE; break;
        case 32: stringId = 0x0242D45C; break;
        case 33: stringId = 0xB3FB4F7F; break;
        case 34: stringId = 0x3FC22003; break;
        case 35: stringId = 0x2DCF5941; break;
        case 36: stringId = 0x0D235D4B; break;
        case 37: stringId = 0xA1216033; break;
        case 39: stringId = 0x5AC8AFBB; break;
        case 40: stringId = 0xB04CAD6A; break;
        case 42: stringId = 0x80D4C2E3; break;
        case 43: stringId = 0x62F8280D; break;
        case 44: stringId = 0x461BA4E8; break;
        case 45: stringId = 0xC130636A; break;
        case 46: stringId = 0x44E2DD06; break;
        case 47: stringId = 0x8BC1A50E; break;
        case 48: stringId = 0x1ECE8CFC; break;
        case 49: stringId = 0xD871BB46; break;
        case 50: stringId = 0x7BA47E90; break;
        case 51: stringId = 0x3EC0C74A; break;
        case 52: stringId = 0xC30A962C; break;
        case 53: stringId = 0xF4980932; break;
        case 54: stringId = 0x48FFBD9A; break;
        case 55: stringId = 0xE2B2D991; break;
        case 56: stringId = 0xCF30BEFB; break;
        case 57: stringId = 0x7AD1A880; break;
        case 58: stringId = 0xAF29A206; break;
        case 59: stringId = 0x516DB1A2; break;
        case 60: stringId = 0x31C3794E; break;
        case 61: stringId = 0xF0EBA2E4; break;
        case 62: stringId = 0x31B789A6; break;
        case 63: stringId = 0xE3BE8E0D; break;
        case 64: stringId = 0x1A3B16DB; break;
        case 65: stringId = 0x9EEFF8CF; break;
        case 66: stringId = 0x64400F01; break;
        case 67: stringId = 0x06046121; break;
        case 68: stringId = 0xD3560061; break;
        case 69: stringId = 0x6E3A65E3; break;
        case 70: stringId = 0x9F8BFFAE; break;
        case 71: stringId = 0x82460728; break;
        case 72: stringId = 0x1E0D17D0; break;
        case 73: stringId = 0x3E35B6E8; break;
        case 74: stringId = 0x70EDC806; break;
        case 76: stringId = 0xA82B5A66; break;
        case 77: stringId = 0x16972FBF; break;
        case 78: stringId = 0x1EE681BA; break;
        case 79: stringId = 0x8ECD701E; break;
        case 80: stringId = 0xD97FDBA2; break;
        case 82: stringId = 0xB399BD15; break;
        case 83: stringId = 0x99326383; break;
        case 84: stringId = 0x00FE5875; break;
        case 85: stringId = 0xEC344E3D; break;
        case 86: stringId = 0x3221ECE8; break;
        case 87: stringId = 0xB2145884; break;
        case 88: stringId = 0xB604AAFF; break;
        case 89: stringId = 0xFAB1DEB6; break;
        case 90: stringId = 0xFDF65C18; break;
        case 91: stringId = 0xC952E8B9; break;
        case 92: stringId = 0xEF12CC40; break;
        case 93: stringId = 0xB714E0E2; break;
        case 94: stringId = 0x3DCD926B; break;
        case 95: stringId = 0x60A3835D; break;
        case 96: stringId = 0xCE88D7B9; break;
        case 97: stringId = 0x5C19FA7A; break;
        case 98: stringId = 0x186485A6; break;
        case 100: stringId = 0xE34B099B; break;
        case 101: stringId = 0xA816EC2E; break;
        case 102: stringId = 0x590236FB; break;
        case 103: stringId = 0x334E4F83; break;
        case 104: stringId = 0xC5814157; break;
        case 105: stringId = 0xA7D52E1F; break;
        case 106: stringId = 0xB5F2B622; break;
        case 107: stringId = 0x3CED883F; break;
        case 108: stringId = 0x0A09D437; break;
        case 109: stringId = 0xDA993CC6; break;
        case 110: stringId = 0x5EEB14CE; break;
        case 111: stringId = 0xDD05C6B4; break;
        case 112: stringId = 0xD2BD3683; break;
        case 113: stringId = 0xCFEB0335; break;
        case 114: stringId = 0x06E12057; break;
        case 115: stringId = 0x9760A9E7; break;
        case 116: stringId = 0x52F1EF84; break;
        case 117: stringId = 0x7CCCD9D4; break;
        case 118: stringId = 0x69E5CA0B; break;
        case 119: stringId = 0xC4219DE3; break;
        case 120: stringId = 0xB0B23BCD; break;
        case 122: stringId = 0x2E19C8B8; break;
        case 123: stringId = 0x464A6890; break;
        case 124: stringId = 0xF53F833A; break;
        case 125: stringId = 0x0E73497D; break;
        case 126: stringId = 0xD5566021; break;
        case 128: stringId = 0xAF2E63A6; break;
        case 129: stringId = 0xE028E842; break;
        case 130: stringId = 0x2FB55D20; break;
        case 131: stringId = 0xB8AF706D; break;
        case 132: stringId = 0xA19C12DB; break;
        case 133: stringId = 0x431EEECE; break;
        case 134: stringId = 0x1B75FFCC; break;
        case 135: stringId = 0xAAE0D4FF; break;
        case 136: stringId = 0x6ACB714A; break;
        case 137: stringId = 0xA292A825; break;
        case 138: stringId = 0x3041D7AC; break;
        case 139: stringId = 0x90C46BBB; break;
        case 140: stringId = 0xB7FC5190; break;
        case 141: stringId = 0x40BC050C; break;
        case 142: stringId = 0x09988B49; break;
        case 143: stringId = 0xC08F5C90; break;
        case 144: stringId = 0x05F0B230; break;
        case 145: stringId = 0xADC5F08E; break;
        case 146: stringId = 0x87C37625; break;
        case 147: stringId = 0xE9D46798; break;
        case 148: stringId = 0x5BDBD6D3; break;
        case 149: stringId = 0x39B60856; break;
        case 151: stringId = 0x056D9F12; break;
        case 152: stringId = 0xBC3B07CD; break;
        case 153: stringId = 0xD296C94C; break;
        case 154: stringId = 0xDB4972F1; break;
        case 155: stringId = 0x6A14E1E7; break;
        default: break;
        }
        if (stringId != 0)
            text = L10N::GetString(stringId);
    }

    if (message == 99 && !packet->ints.empty())
    {
        const int dynamicStringId = packet->ints[0] == 1000 ? 0xD982F837 :
                                    packet->ints[0] == 14000 ? 0x0087FB07 : 0x24A5EDDC;
        text = L10N::GetString(dynamicStringId);
        replaceAll(text, text, L"%d", toString(packet->ints[0]));
    }
    else if (message == 6 || message == 8 || message == 9 || message == 10 || message == 68 || message == 69)
    {
        if (packet->strings.size() > 1 && (message == 6 || message == 8 || message == 9 || message == 10))
            replaceAll(text, text, L"%s", packet->strings[1]);
        else if (!packet->strings.empty())
            replaceAll(text, text, L"%s", packet->strings[0]);
    }
    else if (message == 11 && packet->strings.size() > 2)
    {
        replaceAll(text, text, L"{*PLAYER*}", packet->strings[1]);
        replaceAll(text, text, L"{*PLAYER2*}", packet->strings[2]);
    }

    switch (message)
    {
    case 122: case 125: case 126: case 128: case 129: case 154:
        if (!packet->ints.empty()) replaceAll(text, text, L"%d", toString(packet->ints[0]));
        break;
    case 130: case 131: case 132: case 133:
        if (packet->ints.size() > 1) replaceAll(text, text, L"{*QUANTITY*}", toString(packet->ints[1]));
        break;
    case 134: case 135: case 136:
        if (!packet->ints.empty()) replaceAll(text, text, L"{*QUANTITY*}", toString(packet->ints[0]));
        break;
    case 141:
        if (!packet->ints.empty()) replaceAll(text, text, L"{*SIZE*}", toString(packet->ints[0]));
        break;
    case 100:
        if (!packet->ints.empty() && Difficulty::byId(packet->ints[0]) != 0)
            replaceAll(text, text, L"{*DIFFICULTY*}", L10N::GetString(Difficulty::byId(packet->ints[0])->getKey()));
        break;
    case 101: case 109: case 110:
        if (!packet->ints.empty() && GameType::byId(packet->ints[0]) != 0)
            replaceAll(text, text, L"{*MODE*}", L10N::GetString(GameType::byId(packet->ints[0])->getLabel()));
        break;
    case 102: case 104:
        if (packet->ints.size() > 2)
        {
            replaceFirst(text, L"%d", toString(packet->ints[0]));
            replaceFirst(text, L"%d", toString(packet->ints[1]));
            replaceFirst(text, L"%d", toString(packet->ints[2]));
        }
        break;
    case 143: case 144: case 145: case 147:
        if (!packet->strings.empty()) replaceAll(text, text, L"{*PLAYER*}", packet->strings[0]);
        break;
    case 146: case 148: case 149:
        if (!packet->strings.empty()) replaceAll(text, text, L"{*TEAM*}", packet->strings[0]);
        break;
    case 151:
        if (!packet->ints.empty()) replaceAll(text, text, L"{*NUMBER*}", toString(packet->ints[0]));
        break;
    }

    const int singleMessageDuration = GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(38));
    if (singleMessageDuration >= 0)
        m_minecraft->mGui->setSingleMessage(text, m_playerId, singleMessageDuration);
    else
        m_minecraft->mGui->addMessage(text, m_playerId, message == 13 || message == 14 || message == 15,
                                      !g_pApp->IsConsoleChatCensored(), false, false);
}

uint32_t ClientPacketListener::isLocalPlayer(int entityId)
{
    if (m_minecraft == NULL)
        return 0;

    for (int i = 0; i < 4; ++i)
    {
        boost::shared_ptr<MultiplayerLocalPlayer>& localPlayer = m_minecraft->mLocalPlayers[i];
        if (localPlayer != NULL && localPlayer->getId() == entityId)
            return 1;
    }
    return 0;
}
void ClientPacketListener::handleAnimate(boost::shared_ptr<ClientboundAnimatePacket> packet)
{
}

void ClientPacketListener::handleEntityActionAtPosition(boost::shared_ptr<ClientboundPlayerSleepPacket> packet)
{
}

void ClientPacketListener::handlePreLogin(boost::shared_ptr<ClientboundPreLoginPacket> packet)
{
}

void ClientPacketListener::close()
{
    if (m_connection != 0)
        m_connection->close(DisconnectPacket::eDisconnectReason_None);
    m_closed = true;
}

void ClientPacketListener::handleAddMob(boost::shared_ptr<ClientboundAddMobPacket> packet)
{
}

void ClientPacketListener::handleSetTime(boost::shared_ptr<ClientboundSetTimePacket> packet)
{
    if (m_level == 0 || !packet)
        return;
    m_level->setGameTime(packet->getGameTime());
    if (m_level->getLevelData() != 0)
        m_level->getLevelData()->setDayTime(packet->getDayTime());
}

void ClientPacketListener::handleSetSpawn(boost::shared_ptr<ClientboundSetSpawnPositionPacket> packet)
{
}

void ClientPacketListener::handleSetEntityPassengersPacket(boost::shared_ptr<ClientboundSetPassengersPacket> packet)
{
}

void ClientPacketListener::handleEntityLinkPacket(boost::shared_ptr<ClientboundSetEntityLinkPacket> packet)
{
}

void ClientPacketListener::handleEntityEvent(boost::shared_ptr<ClientboundEntityEventPacket> packet)
{
}

void ClientPacketListener::handleDamageIndicator(boost::shared_ptr<ClientboundDamageIndicatorPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player != 0)
        player->updateDamageIndicator(static_cast<float>(packet->getX()),
                                      static_cast<float>(packet->getZ()),
                                      packet->isAllDirections() != 0);
}

void ClientPacketListener::handleMiniGamePlayerSettingsUpdatePacket(boost::shared_ptr<ClientboundMGPlayerSettingsUpdatePacket> packet)
{
}

void ClientPacketListener::handleSetHealth(boost::shared_ptr<ClientboundSetHealthPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player != 0)
    {
        player->hurtTo(packet->getHealth(), packet->damageSource);
        player->mFoodData.setFoodLevel(packet->getFood());
        player->mFoodData.setSaturation(packet->getSaturation());
    }
}

void ClientPacketListener::handleSetExperience(boost::shared_ptr<ClientboundSetExperiencePacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player != 0)
        player->setExperienceValues(packet->getExperienceProgress(),
                                    packet->getExperienceLevel(),
                                    packet->getTotalExperience());
}

void ClientPacketListener::handleRespawn(boost::shared_ptr<ClientboundRespawnPacket> packet)
{
    if (!packet || m_minecraft == 0)
        return;

    const int oldPlayer = m_minecraft->getLocalPlayerIdx();
    m_minecraft->setLocalPlayerIdx(m_playerId);
    m_minecraft->respawnPlayer(packet->getDimension(), m_playerId, packet->getDimension(),
                               packet->shouldKeepOldEntityData());
    if (m_minecraft->mGameModes[m_playerId] != 0)
        m_minecraft->mGameModes[m_playerId]->setLocalMode(packet->getPlayerGameType());

    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player != 0) {
        player->updateRichPresence();
        if (packet->hasPositionAndRotation())
        {
            player->fjResetBounceHeight();
            player->absMoveTo(packet->x, packet->y, packet->z, packet->yRot, packet->xRot);
        }
    }
    gConsoleUIController.CloseUIScenes(m_playerId, false);
    gConsoleUIController.NavigateToScene(m_playerId, static_cast<EUIScene>(3), 0,
                                          static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    gApp.SetAction(m_playerId, static_cast<eXuiAction>(0x0f), 0);
    m_minecraft->setLocalPlayerIdx(oldPlayer);
}

void ClientPacketListener::handleExplosion(boost::shared_ptr<ClientboundExplodePacket> packet)
{
}

void ClientPacketListener::handleContainerOpen(boost::shared_ptr<ClientboundContainerOpenPacket> packet)
{
}

void ClientPacketListener::handleContainerSetSlot(boost::shared_ptr<ClientboundContainerSetSlotPacket> packet)
{
}

void ClientPacketListener::handleContainerAck(boost::shared_ptr<ClientboundContainerAckPacket> packet)
{
}

void ClientPacketListener::handleContainerContent(boost::shared_ptr<ClientboundContainerSetContentPacket> packet)
{
}

void ClientPacketListener::handleOpenSignEditor(boost::shared_ptr<ClientboundOpenSignEditorPacket> packet)
{
}

void ClientPacketListener::handleBlockEntityData(boost::shared_ptr<ClientboundBlockEntityDataPacket> packet)
{
}

void ClientPacketListener::handleContainerSetData(boost::shared_ptr<ClientboundContainerSetDataPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player != 0 && player->mContainerMenu != 0)
        player->mContainerMenu->setData(packet->getId(), packet->getValue());
}

void ClientPacketListener::handleSetEquippedItem(boost::shared_ptr<ClientboundSetEquippedItemPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    boost::shared_ptr<Entity> entity = level->getEntity(packet->getEntity());
    if (entity != 0)
        entity->setItemSlot(packet->getSlot(), packet->getItem());
}

void ClientPacketListener::handleContainerClose(boost::shared_ptr<ClientboundContainerClosePacket> packet)
{
    if (packet && m_playerId >= 0 && m_playerId < 4 && m_minecraft != 0 &&
        m_minecraft->mLocalPlayers[m_playerId] != 0)
        m_minecraft->mLocalPlayers[m_playerId]->clientSideCloseContainer();
}

void ClientPacketListener::handleBlockEvent(boost::shared_ptr<ClientboundBlockEventPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    const BlockPos pos = packet->getPos();
    const BlockState* state = m_level->getBlockState(pos);
    if (state != 0 && state->getBlock() == packet->getBlock())
        m_level->blockEvent(pos, packet->getBlock(), packet->getB0(), packet->getB1());
}

void ClientPacketListener::handleBlockDestruction(boost::shared_ptr<ClientboundBlockDestructionPacket> packet)
{
    if (m_level != 0 && packet)
        m_level->destroyBlockProgress(packet->getId(), packet->getPos(), packet->getProgress());
}

void ClientPacketListener::handleBlockCollectionDestruction(boost::shared_ptr<ClientboundBlockCollectionDestructionPacket> packet)
{
    if (packet == 0 || m_level == 0 || packet->progress >= 0)
        return;
    Random random(0xABB);
    for (unsigned int i = 0; i < packet->positions.size(); ++i)
    {
        random.nextInt(4);
        m_level->destroyBlockProgress(packet->sourceId - static_cast<int>(i),
                                      packet->positions[i], packet->progress);
    }
}

bool ClientPacketListener::canHandleAsyncPackets()
{
    return m_level != 0 && m_connection != 0 && m_started && !m_closed;
}
void ClientPacketListener::handleGameEvent(boost::shared_ptr<ClientboundGameEventPacket> packet)
{
}

void ClientPacketListener::handleMapItemData(boost::shared_ptr<ClientboundMapItemDataPacket> packet)
{
}

void ClientPacketListener::handleLevelEvent(boost::shared_ptr<ClientboundLevelEventPacket> packet)
{
    if (packet != 0 && m_level != 0)
        m_level->levelEvent(packet->getType(), packet->getPos(), packet->getData());
}

void ClientPacketListener::handleAwardStat(boost::shared_ptr<ClientboundAwardStatPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    Stat* stat = Stats::get(packet->statId);
    if (player != 0 && stat != 0)
        player->awardStatFromServer(stat, packet->getParamData());
}

void ClientPacketListener::handleUpdateMobEffect(boost::shared_ptr<ClientboundUpdateMobEffectPacket> packet)
{
}

void ClientPacketListener::handlePlayerCombat(boost::shared_ptr<ClientboundPlayerCombatPacket> packet)
{
}

void ClientPacketListener::handleChangeDifficulty(boost::shared_ptr<ClientboundChangeDifficultyPacket> packet)
{
    if (packet == 0)
        return;
    m_level->getLevelData()->setDifficulty(packet->getDifficulty());
    m_level->getLevelData()->setDifficultyLocked(packet->isLocked());
}

void ClientPacketListener::handleSetCamera(boost::shared_ptr<ClientboundSetCameraPacket> packet)
{
    if (packet == 0 || m_level == 0 || m_minecraft == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level != 0)
        m_minecraft->setCameraEntity(level->getEntity(packet->getCameraId()));
}

void ClientPacketListener::handleSetBorder(boost::shared_ptr<ClientboundSetBorderPacket> packet)
{
    if (m_level != 0 && packet != 0 && m_level->getWorldBorder() != 0)
        packet->applyChanges(m_level->getWorldBorder());
}

void ClientPacketListener::handleSetTitles(boost::shared_ptr<ClientboundSetTitlesPacket> packet)
{
}

void ClientPacketListener::handleTabListCustomisation(boost::shared_ptr<ClientboundTabListPacket> packet)
{
}

void ClientPacketListener::handleRemoveMobEffect(boost::shared_ptr<ClientboundRemoveMobEffectPacket> packet)
{
}

bool ClientPacketListener::isServerPacketListener() { return false; }
void ClientPacketListener::handlePlayerInfo(boost::shared_ptr<PlayerInfoPacket> packet)
{
    if (packet == 0)
        return;
    const uint32_t privileges = g_pApp->GetPlayerPrivileges(static_cast<uint8_t>(packet->mPlayerId));
    g_pApp->UpdatePlayerInfo(static_cast<uint8_t>(packet->mPlayerId), packet->mPlayerIndex,
                             privileges, packet->mGameType);
}

void ClientPacketListener::handleKeepAlive(boost::shared_ptr<ClientboundKeepAlivePacket> packet)
{
}

void ClientPacketListener::handlePlayerAbilities(boost::shared_ptr<ClientboundPlayerAbilitiesPacket> packet)
{
    if (packet == 0 || m_minecraft == 0)
        return;
    const int playerId = packet->getPlayerId();
    if (playerId < 0 || playerId >= 4 || m_minecraft->mLocalPlayers[playerId] == 0)
        return;
    MultiplayerLocalPlayer* player = m_minecraft->mLocalPlayers[playerId].get();
    player->mAbilities.mInvulnerable = packet->isInvulnerable();
    player->mAbilities.mFlying = packet->isFlying();
    player->mAbilities.mMayfly = packet->canFly();
    player->mAbilities.mInstabuild = packet->canInstabuild();
    player->mAbilities.mMayBuild = packet->canBuild();
    player->mAbilities.setFlyingSpeed(packet->getFlyingSpeed());
    player->mAbilities.setWalkingSpeed(packet->getWalkingSpeed());
}

void ClientPacketListener::handleSoundEvent(boost::shared_ptr<ClientboundSoundPacket> packet)
{
}

void ClientPacketListener::handleMoveVehicle(boost::shared_ptr<ClientboundMoveVehiclePacket> packet)
{
}

void ClientPacketListener::handleBossUpdate(boost::shared_ptr<ClientboundBossEventPacket> packet)
{
    if (m_minecraft != 0 && m_minecraft->mGui != 0 && packet != 0)
        m_minecraft->mGui->getBossOverlay(m_playerId)->update(packet);
}

void ClientPacketListener::handleItemCooldown(boost::shared_ptr<ClientboundCooldownPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player != 0 && player->getCooldowns() != 0)
    {
        if (packet->getDuration() == 0)
            player->getCooldowns()->removeCooldown(packet->getItem());
        else
            player->getCooldowns()->addCooldown(packet->getItem(), packet->getDuration());
    }
}

void ClientPacketListener::handleCustomPayload(boost::shared_ptr<ClientboundCustomPayloadPacket> packet)
{
}

Connection* ClientPacketListener::getConnection() { return m_connection; }
void ClientPacketListener::handleServerSettingsChanged(boost::shared_ptr<ServerSettingsChangedPacket> packet)
{
    if (packet->mSetting == 0)
        gApp.SetGameHostOption(static_cast<eGameHostOption>(4), packet->mValue);
    else if (packet->mSetting == 1)
        gApp.SetGameHostOption(static_cast<eGameHostOption>(0x20), packet->mValue);
}

void ClientPacketListener::handleTexture(boost::shared_ptr<TexturePacket> packet)
{
}

void ClientPacketListener::handleTextureAndGeometry(boost::shared_ptr<TextureAndGeometryPacket> packet)
{
}

void ClientPacketListener::handleUpdateProgress(boost::shared_ptr<UpdateProgressPacket> packet)
{
    if (!CGameNetworkManager::sInstance.IsHost() && m_minecraft != 0 && packet != 0 &&
        m_minecraft->mProgressRenderer != 0)
        m_minecraft->mProgressRenderer->progressStagePercentage(packet->mProgress);
}

void ClientPacketListener::handleTextureChange(boost::shared_ptr<TextureChangePacket> packet)
{
}

void ClientPacketListener::handleTextureAndGeometryChange(boost::shared_ptr<TextureAndGeometryChangePacket> packet)
{
}

void ClientPacketListener::handleUpdateGameRuleProgressPacket(boost::shared_ptr<UpdateGameRuleProgressPacket> packet)
{
}

void ClientPacketListener::handleXZ(boost::shared_ptr<XZPacket> packet)
{
}

void ClientPacketListener::handleScoreboardPacket(boost::shared_ptr<ScoreboardPacket> packet)
{
}

void ClientPacketListener::handleClientboundSetPlayerTeamPacket(boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet)
{
}

void ClientPacketListener::handleParticleEvent(boost::shared_ptr<ClientboundLevelParticlesPacket> packet)
{
}

void ClientPacketListener::handleUpdateAttributes(boost::shared_ptr<ClientboundUpdateAttributesPacket> packet)
{
}

void ClientPacketListener::updateTutorialOnGameMode(eTutorial_GameMode gameMode)
{
    if (m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    MultiPlayerGameMode* mode = m_minecraft->mGameModes[m_playerId];
    if (mode != 0 && mode->getTutorial() != 0)
        mode->getTutorial()->onGameModeChange(gameMode);
}
void ClientPacketListener::handleGameMode(boost::shared_ptr<GameModePacket> packet)
{
}

void ClientPacketListener::handleMapSelectInfo(boost::shared_ptr<MapSelectInfoPacket> packet)
{
}

void ClientPacketListener::handlePlayerReady(boost::shared_ptr<PlayerReadyPacket> packet)
{
    if (packet != 0 && Minecraft::GetInstance()->GetClientMasterGameMode() != 0)
        Minecraft::GetInstance()->GetClientMasterGameMode()->SetPlayerReady(packet->mPlayerId, packet->mReady);
}

void ClientPacketListener::handlePowerup(boost::shared_ptr<ClientboundPowerupPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || packet->getPowerupTime() <= 0 ||
        m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player != 0)
        PowerupItems::ActivateTimedPowerup(player,
            static_cast<PowerupItems::eGlide_Timed_Powerup_ID>(packet->getPowerupID()),
            packet->getPowerupTime());
}

void ClientPacketListener::handleVote(boost::shared_ptr<VotePacket> packet)
{
    if (packet == 0 || m_minecraft == 0)
        return;
    ClientMasterGameMode* gameMode = m_minecraft->GetClientMasterGameMode();
    if (gameMode == 0)
        return;
    int voteableId = packet->voteableId;
    if (voteableId <= -2 || voteableId >= 5)
        return;
    Voteable* voteable = gameMode->mVoteables->getVoteable(static_cast<unsigned int>(voteableId));
    if (voteable != 0)
        voteable->setVote(packet->playerId, packet->voteId, packet->vote);
}

void ClientPacketListener::staticCtor() {}

ClientPacketListener::DeferredEntityLinkPacket::DeferredEntityLinkPacket(boost::shared_ptr<ClientboundSetEntityLinkPacket> packet)
    : entityId(0), linkId(0), linkType(0)
{
}
