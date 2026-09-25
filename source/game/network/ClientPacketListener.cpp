#include "game/network/ClientPacketListener.hpp"
#include "game/network/Connection.hpp"
#include "game/core/Socket.hpp"
#include "game/data/SavedDataMemoryStorage.hpp"
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
#include "game/world/block/Blocks.hpp"
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
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/User.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/core/DLCPack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/core/TrialMode.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/item/Items.hpp"
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
#include "game/core/LightningBolt.hpp"
#include "game/core/Mth.hpp"
#include "game/world/LevelSettings.hpp"
#include "game/world/LevelType.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameType.hpp"
#include "game/core/ClientSideMerchant.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/GhostController.hpp"
#include "game/core/GameModeTransitionParams.hpp"
#include "game/core/MOJJANG_DATA.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/MapRenderer.hpp"
#include "game/core/Socket.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRules.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/generation/LevelGenerators.hpp"
#include "game/world/generation/RemotePlayer.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/core/Guardian.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/PrimedTnt.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/client/render/ItemPickupParticle.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/client/audio/FollowPlayerSoundInstance.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/world/chunk/ClientChunkCache.hpp"
#include "game/client/audio/GuardianAttackSoundInstance.hpp"
#include "game/client/audio/ConsoleSoundEngine.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/ui/LocalMenuContainer.hpp"
#include "game/entity/mob/HorseChest.hpp"
#include "game/network/ChunkVisibilityAreaPacket.hpp"
#include "game/network/ClientboundAddGlobalEntityPacket.hpp"
#include "game/network/ClientboundAddMobPacket.hpp"
#include "game/network/ClientboundAddPaintingPacket.hpp"
#include "game/network/ClientboundAddPlayerPacket.hpp"
#include "game/network/ClientboundAnimatePacket.hpp"
#include "game/network/ClientboundBossEventPacket.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/ClientboundChunkBlocksUpdatePacket.hpp"
#include "game/network/ClientboundContainerAckPacket.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/network/ClientboundContainerSetContentPacket.hpp"
#include "game/network/ClientboundContainerSetSlotPacket.hpp"
#include "game/network/ClientboundCustomPayloadPacket.hpp"
#include "game/network/ClientboundEntityEventPacket.hpp"
#include "game/network/ClientboundExplodePacket.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/ClientboundKeepAlivePacket.hpp"
#include "game/network/ClientboundLevelParticlesPacket.hpp"
#include "game/network/ClientboundLoginPacket.hpp"
#include "game/network/ClientboundMapItemDataPacket.hpp"
#include "game/network/ClientboundMGPlayerSettingsUpdatePacket.hpp"
#include "game/network/ClientboundMoveVehiclePacket.hpp"
#include "game/network/ClientboundOpenSignEditorPacket.hpp"
#include "game/network/ClientboundPlayerCombatPacket.hpp"
#include "game/network/ClientboundPlayerPositionPacket.hpp"
#include "game/network/ClientboundPlayerSleepPacket.hpp"
#include "game/network/ClientboundPreLoginPacket.hpp"
#include "game/network/ClientboundRemoveMobEffectPacket.hpp"
#include "game/network/ClientboundSetEntityDataPacket.hpp"
#include "game/network/ClientboundSetEntityLinkPacket.hpp"
#include "game/network/ClientboundSetPassengersPacket.hpp"
#include "game/network/ClientboundSetPlayerTeamPacket.hpp"
#include "game/network/ClientboundSetSpawnPositionPacket.hpp"
#include "game/network/ClientboundSetTitlesPacket.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/network/ClientboundTabListPacket.hpp"
#include "game/network/ClientboundTakeItemEntityPacket.hpp"
#include "game/network/ClientboundTeleportEntityPacket.hpp"
#include "game/network/ClientboundUpdateAttributesPacket.hpp"
#include "game/network/ClientboundUpdateMobEffectPacket.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/network/ScoreboardPacket.hpp"
#include "game/network/UpdateGameRuleProgressPacket.hpp"
#include "game/network/XZPacket.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/network/ServerboundMoveVehiclePacket.hpp"
#include "game/network/ServerboundContainerClosePacket.hpp"
#include "game/network/TexturePacket.hpp"
#include "game/network/TextureChangePacket.hpp"
#include "game/network/TextureAndGeometryPacket.hpp"
#include "game/network/TextureAndGeometryChangePacket.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/MapSelectInfoPacket.hpp"
#include "game/network/MoveEntityPacketSmall.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/LevelData.hpp"
#include "game/core/Slime.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/SimpleContainer.hpp"
#include "game/item/Container.hpp"
#include "game/network/ServerboundAcceptTeleportationPacket.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/data/McRegionLevelStorage.hpp"
#include "game/data/ConsoleSaveFileReadOnly.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/AreaConstraint.hpp"
#include "game/core/RegionFileCache.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/ui/UIScene_InGameSettingsListMenu.hpp"
#include "game/core/Painting.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/core/Explosion.hpp"
#include "game/network/ServerboundContainerAckPacket.hpp"
#include "game/world/block/SignBlockEntity.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/item/MerchantRecipeList.hpp"
#include "game/ui/UIScene_TradingMenu.hpp"
#include "game/ui/MerchantMenu.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/RangedAttribute.hpp"
#include "game/core/System.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/core/Team.hpp"
#include "game/core/File.hpp"
#include "platform/wiiu/CProfile.hpp"
#include <cmath>

extern unsigned int GetTickCount();
extern void instructionSynchronize();

namespace
{
    const unsigned long long kCauldronBlockEntityTypeWiiU = 0x1000211ULL;
const unsigned long long kConduitBlockEntityType = 0x1200212ULL;
    const unsigned long long kBlockEntityTypeForPacket01 = 0x1200201ULL;
const unsigned long long kBlockEntityTypeForPacket2 = 0x1000209ULL;
const unsigned long long kBlockEntityTypeForPacket3 = 0x1e00204ULL;
const unsigned long long kBlockEntityTypeForPacket4 = 0x1200207ULL;
const unsigned long long kBlockEntityTypeForPacket5 = 0x100020dULL;
const unsigned long long kBlockEntityTypeForPacket6 = 0x100020cULL;
const unsigned long long kBlockEntityTypeForPacket8 = 0x1300201ULL;
const unsigned long long kBlockEntityTypeForPacket9 = 0x1000202ULL;
const unsigned long long kBlockEntityTypeForPacket10 = 0x120020fULL;
const unsigned long long kBlockEntityTypeForPacket11 = 0x1000210ULL;
}

namespace
{
    Attribute* attributeById(eATTRIBUTE_ID id)
    {
        switch (id)
        {
        case eATTRIBUTE_ID_MaxHealth: return SharedMonsterAttributes::MAX_HEALTH;
        case eATTRIBUTE_ID_FollowRange: return SharedMonsterAttributes::FOLLOW_RANGE;
        case eATTRIBUTE_ID_KnockbackResistance: return SharedMonsterAttributes::KNOCKBACK_RESISTANCE;
        case eATTRIBUTE_ID_MovementSpeed: return SharedMonsterAttributes::MOVEMENT_SPEED;
        case eATTRIBUTE_ID_AttackDamage: return SharedMonsterAttributes::ATTACK_DAMAGE;
        case eATTRIBUTE_ID_HorseJumpStrength: return SharedMonsterAttributes::ATTACK_DAMAGE;
        case eATTRIBUTE_ID_ZombieSpawnReinforcements: return SharedMonsterAttributes::ATTACK_DAMAGE;
        case eATTRIBUTE_ID_AttackSpeed: return SharedMonsterAttributes::ATTACK_SPEED;
        case eATTRIBUTE_ID_Armor: return SharedMonsterAttributes::ARMOR;
        case eATTRIBUTE_ID_ArmorToughness: return SharedMonsterAttributes::ARMOR_TOUGHNESS;
        case eATTRIBUTE_ID_Luck: return SharedMonsterAttributes::LUCK;
        case eATTRIBUTE_ID_FlyingSpeed: return SharedMonsterAttributes::FLYING_SPEED;
        default: return NULL;
        }
    }

    bool blockEntityTypeMatches(BlockEntity* entity, unsigned long long blockEntityType)
    {
        return entity != NULL && entity->GetType() == blockEntityType;
    }

    void replaceFirst(std::wstring& value, const std::wstring& key, const std::wstring& replacement)
    {
        const std::wstring::size_type pos = value.find(key);
        if (pos != std::wstring::npos)
            value.replace(pos, key.size(), replacement);
    }

}

OSFastMutex ClientPacketListener::s_deferredLinkMutex;
std::vector<ClientPacketListener::DeferredEntityLinkPacket> ClientPacketListener::s_deferredLinks;

ClientPacketListener::ClientPacketListener(Minecraft* minecraft, Socket* socket, int playerId)
    : m_closed(false), m_connection(0), mRemoteAddress(L""), m_suppressStartOnPosition(false),
      m_minecraft(minecraft), m_level(0), m_started(false), m_playerId(playerId), m_random(),
      m_savedDataStorage(0)
{
    InitializeCriticalSection(&m_mutex);
    m_savedDataStorage = new SavedDataMemoryStorage();
    if (playerId < 0)
        m_playerId = gProfile->GetPrimaryPad();
    if (socket == 0)
        socket = new Socket(false);
    if (socket->mOpen)
    {
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
        if (blockEntityTypeMatches(blockEntity.get(), kCauldronBlockEntityTypeWiiU))
        {
            boost::shared_ptr<CauldronBlockEntity> cauldron =
                boost::static_pointer_cast<CauldronBlockEntity>(blockEntity);
            cauldron->loadFromPacket(level, pos, tag);
        }
        return;
    }

    if (type == 0xd)
    {
        if (blockEntityTypeMatches(blockEntity.get(), kConduitBlockEntityType))
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
        loadTag = blockEntityTypeMatches(blockEntity.get(), kBlockEntityTypeForPacket01);
        break;
    case 2:
        loadTag = blockEntityTypeMatches(blockEntity.get(), kBlockEntityTypeForPacket2);
        break;
    case 3:
        loadTag = blockEntityTypeMatches(blockEntity.get(), kBlockEntityTypeForPacket3);
        break;
    case 4:
        loadTag = blockEntityTypeMatches(blockEntity.get(), kBlockEntityTypeForPacket4);
        break;
    case 5:
        loadTag = blockEntityTypeMatches(blockEntity.get(), kBlockEntityTypeForPacket5);
        break;
    case 6:
        loadTag = blockEntityTypeMatches(blockEntity.get(), kBlockEntityTypeForPacket6);
        break;
    case 8:
        loadTag = blockEntityTypeMatches(blockEntity.get(), kBlockEntityTypeForPacket8);
        break;
    case 9:
        loadTag = blockEntityTypeMatches(blockEntity.get(), kBlockEntityTypeForPacket9);
        break;
    case 10:
        loadTag = blockEntityTypeMatches(blockEntity.get(), kBlockEntityTypeForPacket10);
        break;
    case 11:
        loadTag = blockEntityTypeMatches(blockEntity.get(), kBlockEntityTypeForPacket11);
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
Socket* ClientPacketListener::getSocket() { return m_connection->m_socket; }
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
        Socket* socket = getSocket();
        if (socket != 0)
            return socket->getPlayer();
    }
    return 0;
}

bool ClientPacketListener::IsPrimaryPlayer()
{
    return m_playerId == CGameNetworkManager::sInstance.GetPrimaryPad();
}

void ClientPacketListener::displayPrivilegeChanges(boost::shared_ptr<MultiplayerLocalPlayer> player, uint32_t changes)
{
    const uint32_t userIndex = player->mUserIndex;
    const uint32_t oldPrivileges =
        player->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All);
    for (uint32_t bit = 0; bit < 0x20; ++bit)
    {
        const uint32_t oldValue = Player::getPlayerGamePrivilege(
            oldPrivileges, static_cast<Player::EPlayerGamePrivileges>(bit));
        const uint32_t newValue = Player::getPlayerGamePrivilege(
            changes, static_cast<Player::EPlayerGamePrivileges>(bit));
        if (oldValue == newValue)
            continue;
        const bool granted = oldValue == 0;
        std::wstring message;
        if (gApp.GetGameHostOption(eGameHostOption_12) == 0)
        {
            switch (bit)
            {
            case 0: message = L10N::GetString(granted ? 0xBB86B9EC : 0x80CBA202); break;
            case 1: message = L10N::GetString(granted ? 0xF9B83295 : 0x590C0C53); break;
            case 2: message = L10N::GetString(granted ? 0x8425EE2C : 0x7A302BC2); break;
            case 3: message = L10N::GetString(granted ? 0xCFCD48CF : 0xD83F3EE9); break;
            case 10: message = L10N::GetString(granted ? 0x3565EA42 : 0x332C0EFC); break;
            case 11: message = L10N::GetString(granted ? 0x0FEF7A7A : 0x5CC73FE4); break;
            case 12: message = L10N::GetString(granted ? 0x0359DA13 : 0xFEEB1CA5); break;
            default: break;
            }
        }
        if (bit == 4)
            message = L10N::GetString(granted ? 0x453A2722 : 0xA5DE08DC);
        if (gApp.GetGameHostOption(eGameHostOption_15) != 0)
        {
            switch (bit)
            {
            case 5: message = L10N::GetString(granted ? 0x0914B658 : 0xD4D6F596); break;
            case 6: message = L10N::GetString(granted ? 0x0E58AD5D : 0x118BE14B); break;
            case 7: message = L10N::GetString(granted ? 0x67448842 : 0x8A95FCFC); break;
            case 8: message = L10N::GetString(granted ? 0x43E9FBD8 : 0x14C25F16); break;
            case 13: message = L10N::GetString(granted ? 0xC94448B1 : 0xE2849697); break;
            case 14: message = L10N::GetString(granted ? 0x02CA1EEB : 0xE40A892D); break;
            case 15: message = L10N::GetString(granted ? 0xB2427456 : 0x25853DE8); break;
            case 16: message = L10N::GetString(granted ? 0x986DB991 : 0xCA33A3F7); break;
            default: break;
            }
        }
        if (!message.empty())
            m_minecraft->mGui->addMessage(message, userIndex, false, false, false, false);
    }
}

void ClientPacketListener::handleLogin(boost::shared_ptr<ClientboundLoginPacket> packet) {
    if (m_closed || packet == 0 || m_minecraft == 0)
        return;

    PlayerUID suid(0);
    gProfile->GetXUID(m_playerId, &suid, true);
    MOJJANG_DATA* mojangData = 0;
    if (!CGameNetworkManager::sInstance.IsLocalGame())
        mojangData = gApp.GetMojangDataForXuid(suid);

    if (!CGameNetworkManager::sInstance.IsHost())
        m_minecraft->mProgressRenderer->progressStagePercentage(75);

    INetworkPlayer* networkPlayer = getNetworkPlayer();
    int userIndex = -1;
    if (!IsPrimaryPlayer() && networkPlayer != 0 && !networkPlayer->IsGuest()
        && CGameNetworkManager::sInstance.IsSignedInLive(networkPlayer->GetUserIndex()))
    {
        for (int i = 0; i < 4; ++i)
        {
            if (CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(i) == networkPlayer)
            {
                userIndex = i;
                break;
            }
        }
    }
    else if (IsPrimaryPlayer())
    {
        userIndex = m_playerId;
        telemetryManagerInstance->SetMultiplayerInstanceId(packet->mChunkDistance);
    }

    if (userIndex != -1)
    {
        if (mojangData == 0)
            gApp.SetBanListCheck(userIndex, false);
        else
        {
            if (mojangData->skinPath[0] != 0 && gApp.IsFileInMemoryTextures(mojangData->skinPath))
                gApp.AddMemoryTextureFile(mojangData->skinPath, 0, 0);
            if (mojangData->capePath[0] != 0 && gApp.IsFileInMemoryTextures(mojangData->capePath))
                gApp.AddMemoryTextureFile(mojangData->capePath, 0, 0);
            gApp.SetBanListCheck(userIndex, false);
        }
    }

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer;
    const GameType* gameType = GameType::byId(packet->mGameType);
    const Difficulty* difficulty = Difficulty::byId(packet->mPlayerFlag);

    if (!IsPrimaryPlayer())
    {
        MultiPlayerLevel* level = m_minecraft->getLevel(packet->mGameMode);
        if (level == 0)
        {
            level = m_minecraft->getLevel(packet->mGameMode == 0 ? -1 : 0);
            if (level == 0)
                level = m_minecraft->getLevel(packet->mGameMode != 0 && packet->mGameMode != -1 ? -1 : 1);
        }

        std::wstring name;
        if (networkPlayer != 0)
            name = networkPlayer->GetDisplayName();

        if (level == 0)
        {
            LevelSettings settings(
                packet->mSeed, gameType, false, false,
                packet->mIsFlat, packet->mLevelType, packet->mXZSize,
                packet->mHellScale, packet->mMaxTick, packet->mIsFlatWorld,
                packet->mPlayerFlags, packet->mSpawnProtection, packet->mSimulationDistance);
            level = new MultiPlayerLevel(this, &settings, packet->mGameMode, difficulty, packet->mUnknownValue3);
            if (m_level != 0)
                level->setSavedDataStorage(m_level->getSavedDataStorage());
            m_level = level;
            localPlayer = m_minecraft->createExtraLocalPlayer(
                m_playerId, name, m_playerId, packet->mGameMode, this, level,
                packet->mGameType, packet->mDifficulty);
            boost::shared_ptr<Player> playerPtr = localPlayer;
            m_minecraft->setLevel(level, -1, playerPtr, true, false);
        }
        else
        {
            m_level = level;
            localPlayer = m_minecraft->createExtraLocalPlayer(
                m_playerId, name, m_playerId, packet->mGameMode, this, 0,
                packet->mGameType, packet->mDifficulty);
        }
    }
    else
    {
        const int primaryPad = gProfile->GetPrimaryPad();
        if (gameType == GameType::LOBBY)
            m_minecraft->mGameMode = new MultiPlayerGameMode(primaryPad, this, packet->mDifficulty, false, true);
        else if (gApp.GetTutorialMode(boost::shared_ptr<Player>()))
            m_minecraft->mGameMode = new MultiPlayerGameMode(primaryPad, this, packet->mDifficulty, true, false);
        else if (!gProfile->IsFullVersion())
            m_minecraft->mGameMode = new TrialMode(primaryPad, this, packet->mDifficulty);
        else
            m_minecraft->mGameMode = new MultiPlayerGameMode(primaryPad, this, packet->mDifficulty, false, false);
        const MiniGameDef* joiningMiniGame = m_minecraft->GetMiniGame();
        if (joiningMiniGame != 0)
            joiningMiniGame->SetDisabledEditActions(false);

        MultiPlayerLevel* level = m_minecraft->getLevel(packet->mGameMode);
        if (level == 0)
        {
            LevelSettings settings(
                packet->mSeed, gameType, false, false,
                packet->mIsFlat, packet->mLevelType, packet->mXZSize,
                packet->mHellScale, packet->mMaxTick, packet->mIsFlatWorld,
                packet->mPlayerFlags, packet->mSpawnProtection, packet->mSimulationDistance);
            if (Minecraft::InMiniGame(static_cast<EMiniGameId>(0xf), true) &&
                !CGameNetworkManager::sInstance.IsHost())
            {
                LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
                if (options != 0)
                {
                    File saveRoot(options->getBaseSaveRoot());
                    File savePath(options->getBaseSavePath());
                    options->setBaseSaveData(savePath);
                    gApp.setLevelGenerationOptions(options);
                }
                ConsoleSaveFileReadOnly* saveFile = new ConsoleSaveFileReadOnly(L"", 0, 0, false, eSavePlatform_WIIU);
                File worldFolder(L"world");
                boost::shared_ptr<McRegionLevelStorage> storage(new McRegionLevelStorage(
                    saveFile, worldFolder, packet->mLevelName, true, m_minecraft->getFixerUpper()));
                level = new MultiPlayerLevel(storage, this, &settings, packet->mGameMode, difficulty,
                                             packet->mUnknownValue3);
            }
            else
                level = new MultiPlayerLevel(this, &settings, packet->mGameMode, difficulty,
                                             packet->mUnknownValue3);
            m_level = level;
            boost::shared_ptr<Player> emptyPlayer;
            m_minecraft->setLevel(level, packet->mGameMode, emptyPlayer, true, false);
        }
        else
        {
            m_level = level;
        }
        m_minecraft->createPrimaryLocalPlayer(m_playerId);
        localPlayer = m_minecraft->mLocalPlayers[m_playerId];
    }

    if (localPlayer == 0)
        localPlayer = m_minecraft->mLocalPlayers[m_playerId];
    if (localPlayer == 0)
        return;

    localPlayer->mDimension = packet->mGameMode;
    localPlayer->setId(packet->mEntityId);
    localPlayer->setCustomSkin(GameSettings::GetPlayerSkinId(m_playerId));
    localPlayer->setCustomCape(GameSettings::GetPlayerCapeId(m_playerId));

    gApp.UpdatePlayerInfo(getSocket()->mPlayerId,
                          static_cast<short>(packet->mReducedDebugInfo),
                          static_cast<unsigned int>(packet->mCurrentTick),
                          static_cast<unsigned char>(packet->mGameType));

    uint32_t privileges = 0;
    Player::enableAllPlayerPrivileges(privileges, true);
    localPlayer->setPlayerGamePrivilege(static_cast<Player::EPlayerGamePrivileges>(0x21), packet->mCurrentTick);
    if (networkPlayer != 0 && networkPlayer->IsHost())
        Player::setPlayerGamePrivilege(privileges, static_cast<Player::EPlayerGamePrivileges>(0x22), 1);
    displayPrivilegeChanges(localPlayer, privileges);

    GameSettings::SetGameSettingsDebugMask(
        m_playerId, GameSettings::GetGameSettingsDebugMask(m_playerId, true));
    if (m_minecraft->mGameModes[m_playerId] != 0)
        m_minecraft->mGameModes[m_playerId]->setLocalMode(gameType);
    localPlayer->UpdateSize();
    localPlayer->updateRichPresence();
    gConsoleUIController.UpdateSelectedItemPos(static_cast<unsigned int>(m_playerId));

    const MiniGameDef* miniGame = m_minecraft->GetMiniGame();
    if (miniGame != 0)
        miniGame->SetDisabledEditActions(false);

    gApp.SetFirstMiniGameType(static_cast<EMiniGameId>(packet->mDifficulty));
    m_minecraft->GetClientMasterGameMode()->Setup(static_cast<EMiniGameId>(packet->mDifficulty));
    telemetryManagerInstance->RecordLevelStart(m_playerId, static_cast<ESen_FriendOrMatch>(1),
        static_cast<ESen_CompeteOrCoop>(3), m_level->getDifficulty()->getId(),
        gApp.GetLocalPlayerCount(), CGameNetworkManager::sInstance.GetOnlinePlayerCount());
    telemetryManagerInstance->RecordMiniGameHostOptions(m_playerId,
        static_cast<EMiniGameId>(packet->mDifficulty));
}
boost::shared_ptr<Entity> ClientPacketListener::resolveEntity(int entityId)
{
    if (m_minecraft != 0)
    {
        for (int i = 0; i < 4; ++i)
        {
            boost::shared_ptr<MultiplayerLocalPlayer>& localPlayer = m_minecraft->mLocalPlayers[i];
            if (localPlayer != 0 && localPlayer->getId() == entityId)
            {
                boost::shared_ptr<MultiplayerLocalPlayer>& current = m_minecraft->mLocalPlayers[m_playerId];
                if (current == 0 || current->getId() != entityId)
                    return boost::shared_ptr<Entity>();
                return localPlayer;
            }
        }
    }
    if (m_level == 0)
        return boost::shared_ptr<Entity>();
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return boost::shared_ptr<Entity>();
    return level->getEntity(entityId);
}

boost::shared_ptr<Entity> ClientPacketListener::getEntity(int entityId)
{
    if (m_minecraft != 0)
    {
        for (int i = 0; i < 4; ++i)
        {
            boost::shared_ptr<Entity> localPlayer = m_minecraft->mLocalPlayers[i];
            if (localPlayer != 0 && localPlayer->getId() == entityId)
            {
                boost::shared_ptr<MultiplayerLocalPlayer>& current = m_minecraft->mLocalPlayers[m_playerId];
                if (current == 0 || current->getId() != entityId)
                    return boost::shared_ptr<Entity>();
                return current;
            }
        }
    }
    if (m_level == 0)
        return boost::shared_ptr<Entity>();
    return m_level->getEntity(entityId);
}
void ClientPacketListener::checkDeferredEntityLinkPackets(int entityId)
{
    EnterCriticalSection(&s_deferredLinkMutex);
    for (std::vector<DeferredEntityLinkPacket>::iterator it = s_deferredLinks.begin();
         it != s_deferredLinks.end();)
    {
        if (it->mPacket == 0)
        {
            it = s_deferredLinks.erase(it);
            continue;
        }
        const unsigned int age = GetTickCount() - it->mTick;
        if (age < 1000 && it->mPacket->getDestId() == entityId)
        {
            handleEntityLinkPacket(it->mPacket);
            it = s_deferredLinks.erase(it);
        }
        else if (age >= 1000)
            it = s_deferredLinks.erase(it);
        else
            ++it;
    }
    LeaveCriticalSection(&s_deferredLinkMutex);
}


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
    case 70: entity.reset(new FallingBlockEntity(level, x, y, z, Block::stateByIdWithData(packet->getData()))); packet->setData(0); break;
    case 71: entity.reset(new ItemFrame(level, BlockPos((int)x, (int)y, (int)z), Direction::from2DDataValue(packet->getData()))); packet->setData(0); break;
    case 73:
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        if (miniGame != 0 && miniGame->HasFeatureSplashLevitatePotions() && packet->getData() == 0x1b6)
        {
            entity.reset(new ThrownPotion(level, x, y, z, miniGame->getEquipmentForSlot(EquipmentSlot::eIndex_MainHand)));
        }
        else
        {
            entity.reset(new ThrownPotion(level, x, y, z,
                not_null_ptr<ItemInstance>(new ItemInstance(Item::byId(packet->getData())))));
        }
        packet->setData(0);
        break;
    }
    case 1: entity.reset(new Boat(level, x, y, z)); break;
    case 2: entity.reset(new ItemEntity(level, x, y, z)); break;
    case 3: entity.reset(new AreaEffectCloud(level, x, y, z)); break;
    case 50: entity.reset(new PrimedTnt(level, x, y, z, boost::shared_ptr<LivingEntity>())); break;
    case 51: entity.reset(new EnderCrystal(level, x, y, z)); break;
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
    entity->absMoveTo(x, y, z, packet->getyRot() * (360.0f / 256.0f), packet->getxRot() * (360.0f / 256.0f));
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
    const double scale = 0.03125;
    boost::shared_ptr<ExperienceOrb> orb(new ExperienceOrb(level,
                                                            static_cast<double>(packet->getX()) * scale,
                                                            static_cast<double>(packet->getY()) * scale,
                                                            static_cast<double>(packet->getZ()) * scale,
                                                            packet->getValue()));
    orb->setId(packet->getId());
    level->putEntity(packet->getId(), orb);
}

void ClientPacketListener::handleAddGlobalEntity(boost::shared_ptr<ClientboundAddGlobalEntityPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0 || packet->getType() != 1)
        return;
    const double scale = 0.03125;
    const double x = static_cast<double>(packet->getX()) * scale;
    const double y = static_cast<double>(packet->getY()) * scale;
    const double z = static_cast<double>(packet->getZ()) * scale;
    boost::shared_ptr<LightningBolt> entity(new LightningBolt(level, x, y, z, false));
    entity->setId(packet->getId());
    level->addGlobalEntity(entity);
}

void ClientPacketListener::handleAddPainting(boost::shared_ptr<ClientboundAddPaintingPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    boost::shared_ptr<Painting> painting(new Painting(level, packet->getPos(),
                                                      packet->getDirection(),
                                                      packet->getMotive()));
    painting->setId(packet->getId());
    level->putEntity(packet->getId(), painting);
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
    if (packet == 0)
        return;
    boost::shared_ptr<Entity> entity = resolveEntity(packet->getId());
    if (entity == 0 || !Entity::instanceof(entity, 0x0C800000ULL))
        return;
    if (static_cast<int>(entity->GetType()) != packet->getEntityType())
        return;
    boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
    if (living == 0)
        return;
    std::vector<DataItem*>* unpacked = packet->getUnpackedData();
    if (unpacked != 0)
        living->getEntityData()->assignValues(unpacked);
}


void ClientPacketListener::send(boost::shared_ptr<Packet> packet)
{
    if (m_connection != 0)
        m_connection->send(packet);
}

void ClientPacketListener::handleAddPlayer(boost::shared_ptr<ClientboundAddPlayerPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    const double scale = 0.03125;
    const double x = static_cast<double>(packet->getX()) * scale;
    const double y = static_cast<double>(packet->getY()) * scale;
    const double z = static_cast<double>(packet->getZ()) * scale;
    const float yRot = packet->getyRot() * (360.0f / 256.0f);
    const float xRot = packet->getxRot() * (360.0f / 256.0f);
    boost::shared_ptr<RemotePlayer> remote(new RemotePlayer(level, packet->getName()));
    remote->setXuid(packet->getPlayerId());
    remote->setId(static_cast<int>(packet->getEntityId()));
    remote->absMoveTo(x, y, z, yRot, xRot);
    remote->setPlayerGamePrivilege(static_cast<Player::EPlayerGamePrivileges>(0x21), packet->mPlayerPrivilege);
    std::vector<DataItem*>* unpacked = packet->getUnpackedData();
    if (unpacked != 0)
        remote->getEntityData()->assignValues(unpacked);
    level->putEntity(static_cast<int>(packet->getEntityId()), remote);
    if (m_minecraft != 0)
        m_minecraft->addPendingClientTextureRequest(packet->getName());
}

void ClientPacketListener::handleTeleportEntity(boost::shared_ptr<ClientboundTeleportEntityPacket> packet)
{
    if (packet == 0)
        return;
    boost::shared_ptr<Entity> entity = resolveEntity(packet->getId());
    if (entity == 0)
        return;
    const double scale = 0.03125;
    const double x = static_cast<double>(packet->getX()) * scale;
    const double y = static_cast<double>(packet->getY()) * scale + 0.21875;
    const double z = static_cast<double>(packet->getZ()) * scale;
    const float yRot = Mth::wrapDegrees(packet->getyRot() * (360.0f / 256.0f));
    const float xRot = Mth::wrapDegrees(packet->getxRot() * (360.0f / 256.0f));
    int steps = 0;
    if (std::fabs(entity->mX - x) >= scale || std::fabs(entity->mY - y) >= 0.015625 ||
        std::fabs(entity->mZ - z) >= scale)
        steps = 3;
    entity->lerpTo(x, y, z, yRot, xRot, steps, true);
    entity->mOnGround = packet->isOnGround() != 0;
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

    float yRot = entity->mYRot;
    float xRot = entity->mXRot;

    if ((entity->GetType() & 0xc000000) == 0xc000000)
    {
        boost::shared_ptr<LivingEntity> livingEntity = boost::static_pointer_cast<LivingEntity>(entity);
        if (livingEntity->isInterpolating())
        {
            yRot = static_cast<float>(livingEntity->getLerpToYRot());
            xRot = static_cast<float>(livingEntity->getLerpToXRot());
        }
    }

    if ((entity->GetType() & 0xa800000) == 0xa800000)
    {
        boost::shared_ptr<Minecart> minecart = boost::static_pointer_cast<Minecart>(entity);
        if (minecart->isInterpolating())
        {
            yRot = minecart->getLerpToYRot();
            xRot = minecart->getLerpToXRot();
        }
    }

    if (packet->hasRotation())
        yRot = packet->getyRot() * (360.0f / 256.0f);
    if (packet->hasRotation())
        xRot = packet->getxRot() * (360.0f / 256.0f);

    entity->lerpTo(entity->mX, entity->mY + 0.015625, entity->mZ,
                    Mth::wrapDegrees(yRot), Mth::wrapDegrees(xRot), 3, false);
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
    if (packet == 0)
        return;
    boost::shared_ptr<Entity> entity = resolveEntity(packet->mEntityId);
    if (entity == 0)
        return;
    const double scale = 0.03125;
    entity->mX += static_cast<double>(packet->mXa) * scale;
    entity->mY += static_cast<double>(packet->mYa) * scale;
    entity->mZ += static_cast<double>(packet->mZa) * scale;
    entity->setPos(entity->mX, entity->mY, entity->mZ);
    boost::shared_ptr<LivingEntity> living =
        boost::dynamic_pointer_cast<LivingEntity>(entity);
    if (living != 0 && living->isInterpolating())
    {
        living->lerpTo(entity->mX, entity->mY, entity->mZ,
                         static_cast<float>(living->getLerpToYRot()),
                         static_cast<float>(living->getLerpToXRot()), 3, false);
    }
    boost::shared_ptr<Minecart> minecart = boost::dynamic_pointer_cast<Minecart>(entity);
    if (minecart != 0)
    {
        minecart->lerpTo(entity->mX, entity->mY, entity->mZ,
                         entity->mYRot, entity->mXRot, 3, false);
    }
    if (packet->myRot != 0 || packet->mxRot != 0)
        entity->setRot(packet->myRot * (360.0f / 256.0f), packet->mxRot * (360.0f / 256.0f));
    entity->mOnGround = packet->isOnGround();
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
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player == 0)
        return;
    const boost::unordered_set<const ClientboundPlayerPositionPacket::RelativeArgument*>& relative =
        packet->getRelativeArguments();
    double x = packet->getX();
    double y = packet->getY();
    double z = packet->getZ();
    float yRot = packet->getYRot();
    float xRot = packet->getXRot();
    if (relative.find(&ClientboundPlayerPositionPacket::RelativeArgument::X) != relative.end())
        x += player->mX;
    else
    {
        player->mXd = 0.0;
        player->mXOld = x;
    }
    if (relative.find(&ClientboundPlayerPositionPacket::RelativeArgument::Y) != relative.end())
        y += player->mY;
    else
    {
        player->mYd = 0.0;
        player->mYOld = y;
    }
    if (relative.find(&ClientboundPlayerPositionPacket::RelativeArgument::Z) != relative.end())
        z += player->mZ;
    else
    {
        player->mZd = 0.0;
        player->mZOld = z;
    }
    if (relative.find(&ClientboundPlayerPositionPacket::RelativeArgument::Y_ROT) != relative.end())
        yRot += player->mYRot;
    if (relative.find(&ClientboundPlayerPositionPacket::RelativeArgument::X_ROT) != relative.end())
        xRot += player->mXRot;
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
        boost::dynamic_pointer_cast<MultiplayerLocalPlayer>(player);
    if (localPlayer != 0)
        localPlayer->fjResetBounceHeight();
    if (packet->hasRot())
    {
        player->absMoveTo(x, y, z, yRot, xRot);
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            player->mYHeadRot = yRot;
    }
    else
        player->absMoveTo(x, y, z);
    send(boost::shared_ptr<Packet>(new ServerboundAcceptTeleportationPacket(packet->getId())));
    if (!m_started)
    {
        if (!CGameNetworkManager::sInstance.IsHost())
            m_minecraft->mProgressRenderer->progressStagePercentage(100);
        player->mXo = player->mXOld = player->mX;
        player->mYo = player->mYOld = player->mY;
        player->mZo = player->mZOld = player->mZ;
        if (!m_suppressStartOnPosition)
            m_started = true;
        m_minecraft->setScreen(0);
        if (!m_suppressStartOnPosition && gApp.GetGameStarted())
            gConsoleUIController.CloseUIScenes(m_playerId, false);
        m_suppressStartOnPosition = false;
    }
}

void ClientPacketListener::handleChunkVisibilityArea(boost::shared_ptr<ChunkVisibilityAreaPacket> packet)
{
    if (m_level == 0 || packet == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    if (!packet->outside)
    {
        for (int x = packet->minX; x <= packet->maxX; ++x)
            for (int z = packet->minZ; z <= packet->maxZ; ++z)
                level->setChunkVisible(x, z, packet->visible, packet->loaded);
        return;
    }
    WorldBorder* border = m_level->getWorldBorder();
    if (border == 0)
        return;
    const int radius = static_cast<int>(border->getSize() / 32.0);
    for (int x = -radius; x <= radius; ++x)
    {
        for (int z = -radius; z <= radius; ++z)
        {
            if (x >= packet->minX && z >= packet->minZ &&
                x <= packet->maxX && z <= packet->maxZ)
                continue;
            if (level->hasChunk(x, z, false))
                level->setChunkVisible(x, z, packet->visible, packet->loaded);
        }
    }
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
    if (packet == 0 || m_minecraft == 0)
        return;
    MultiPlayerLevel* level = m_minecraft->getLevel(packet->dimension);
    if (level == 0)
        return;
    LevelChunk* chunk = level->getChunk(packet->x, packet->z);
    if (chunk == 0)
        return;
    const int baseX = packet->x * 16;
    const int baseZ = packet->z * 16;
    int minLocalX = 0x7fffffff;
    int minY = 0x7fffffff;
    int minLocalZ = 0x7fffffff;
    int maxLocalX = 0;
    int maxY = 0;
    int maxLocalZ = 0;
    for (unsigned int i = 0; i < packet->count; ++i)
    {
        const unsigned short packed = static_cast<unsigned short>(packet->positions.data[i]);
        const int localX = (packed >> 12) & 15;
        const int localZ = (packed >> 8) & 15;
        const int y = packed & 255;
        const unsigned int element = packet->elements.data[i];
        const int blockId = static_cast<int>((element & 0xffff) >> 4);
        const int data = static_cast<int>(element & 0xf);
        const unsigned short extra = static_cast<unsigned short>(element >> 16);
        Block* incoming = Block::byId(blockId);
        minLocalX = std::min(minLocalX, localX);
        minY = std::min(minY, y);
        minLocalZ = std::min(minLocalZ, localZ);
        maxLocalX = std::max(maxLocalX, localX);
        maxY = std::max(maxY, y);
        maxLocalZ = std::max(maxLocalZ, localZ);
        chunk->setBlockAndData(localX, y, localZ, blockId, data, true, false);
        chunk->setExtraElement(localX, y, localZ, extra);
        const BlockPos worldPos(baseX + localX, y, baseZ + localZ);
        level->checkLight(worldPos, false, false);
        level->clearResetRegion(worldPos.x, worldPos.y, worldPos.z,
                                worldPos.x, worldPos.y, worldPos.z);
        Block* actual = chunk->getBlock(worldPos);
        const bool skipDirty =
            (actual == Blocks::FLOWING_WATER && incoming == Blocks::WATER) ||
            (actual == Blocks::WATER && incoming == Blocks::FLOWING_WATER) ||
            (actual == Blocks::flowing_lava && incoming == Blocks::lava) ||
            (actual == Blocks::lava && incoming == Blocks::lava) ||
            (actual == Blocks::lava && incoming == Blocks::flowing_lava);
        if (!skipDirty)
            level->setBlocksDirty(worldPos, worldPos);
    }
    if (packet->count != 0)
    {
        level->removeUnusedBlockEntitiesInRegion(baseX + minLocalX, minY, baseZ + minLocalZ,
                                                 baseX + maxLocalX + 1, maxY + 1,
                                                 baseZ + maxLocalZ + 1);
    }
}


void ClientPacketListener::handleBlockRegionUpdate(boost::shared_ptr<BlockRegionUpdatePacket> packet)
{
    if (packet == 0)
        return;

    MultiPlayerLevel* level = m_minecraft->getLevel(packet->mDimension);
    if (level != 0)
    {
        const int chunkX = packet->mX >> 4;
        const int chunkZ = packet->mZ >> 4;
        if (packet->mHasBlockChanges)
        {
            std::vector<LevelChunk::BlockChange>* changes = packet->GetChanges();
            packet->SetChanges(NULL);
            level->ReloadChunk(chunkX, chunkZ, changes, true);
            instructionSynchronize();
            return;
        }

        PIXBeginNamedEvent(90.0f, "Handle block region update");
        level->StopReloadChunk(chunkX, chunkZ);

        int maxY = packet->mY + packet->mHeight;
        if (packet->mFullChunk && m_level != NULL)
        {
            maxY = 256;
            if (m_level->getChunk(chunkX, chunkZ)->isEmpty())
                static_cast<MultiPlayerLevel*>(m_level)->setChunkVisible(chunkX, chunkZ, true, false);
        }

        PIXBeginNamedEvent(90.0f, "Clear rest region");
        level->clearResetRegion(packet->mX, packet->mY, packet->mZ,
                                packet->mX + packet->mWidth - 1, maxY - 1,
                                packet->mZ + packet->mDepth - 1);
        PIXEndNamedEvent();

        PIXBeginNamedEvent(90.0f, "setBlocksAndData");
        if (packet->mFullChunk)
            level->setFromRawData(packet->mX / 16, packet->mZ / 16, packet->mData);
        else
            level->setBlocksAndData(packet->mX, packet->mY, packet->mZ,
                                    packet->mWidth, packet->mHeight, packet->mDepth,
                                    packet->mData, false);
        PIXEndNamedEvent();

        PIXBeginNamedEvent(90.0f, "removeUnusedBlockEntitiesInRegion");
        level->removeUnusedBlockEntitiesInRegion(packet->mX, packet->mY, packet->mZ,
                                                  packet->mX + packet->mWidth, maxY,
                                                  packet->mZ + packet->mDepth);
        PIXEndNamedEvent();

        if (packet->mFullChunk)
        {
            PIXBeginNamedEvent(90.0f, "dataReceivedForChunk");
            level->dataReceivedForChunk(chunkX, chunkZ);
            PIXEndNamedEvent();
        }
        PIXEndNamedEvent();
        Minecraft::GetInstance()->GetClientMasterGameMode()->RecievedBRUP();
    }
    instructionSynchronize();
}

void ClientPacketListener::handleBlockUpdate(boost::shared_ptr<ClientboundBlockUpdatePacket> packet)
{
    MultiPlayerLevel* level = m_minecraft->getLevel(packet->getLevelIdx());
    if (level != 0)
    {
        PIXBeginNamedEvent(0.0f, "Handle block update");
        PIXBeginNamedEvent(0.0f, "Setting data\n");
        level->doSetBlockAndData(packet->getPos(), packet->getBlock(), packet->getExtraBlock());
        PIXEndNamedEvent();
        level->removeUnusedBlockEntitiesInRegion(
            packet->getPos().x, packet->getPos().y, packet->getPos().z,
            packet->getPos().x + 1, packet->getPos().y + 1, packet->getPos().z + 1);
        PIXEndNamedEvent();
    }
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
    if (getAndSetClosed())
        return;

    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mConnectionFailed[m_playerId] = true;
    minecraft->mDisconnectReasons[m_playerId] = reason;

    if (CGameNetworkManager::sInstance.IsHost() &&
        (reason == DisconnectPacket::eDisconnectReason_TimedOut ||
         reason == DisconnectPacket::eDisconnectReason_SendQueueOverflow) &&
        IsPrimaryPlayer())
    {
        int parameters[1] = { static_cast<int>(0xb9a1bf4d) };
        gConsoleUIController.RequestErrorMessage(
            0x3e0b4895, 0x10dab382, parameters, 1, gProfile->GetPrimaryPad(),
            HostDisconnectReturned, 0, 0, true);
        return;
    }

    g_pApp->SetAction(m_playerId, static_cast<eXuiAction>(3), (void*)1);
}

void ClientPacketListener::sendAndDisconnect(boost::shared_ptr<Packet> packet)
{
    send(packet);
    close();
}

void ClientPacketListener::handleTakeItemEntity(boost::shared_ptr<ClientboundTakeItemEntityPacket> packet)
{
    if (m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    boost::shared_ptr<Entity> itemEntity = getEntity(packet->getItemId());
    boost::shared_ptr<Entity> playerEntity = getEntity(packet->getPlayerId());
    boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(playerEntity);
    for (int i = 0; i < 4; ++i)
    {
        boost::shared_ptr<MultiplayerLocalPlayer>& localPlayer = m_minecraft->mLocalPlayers[i];
        if (localPlayer != 0 && localPlayer->getId() == packet->getPlayerId())
        {
            living = localPlayer;
            break;
        }
    }
    if (living == 0)
    {
        level->removeEntity(packet->getItemId());
        return;
    }
    if (itemEntity != 0)
    {
        boost::shared_ptr<ItemPickupParticle> particle(
            new ItemPickupParticle(level, itemEntity, living, -0.5f));
        m_minecraft->mParticleEngine->add(particle.get());
        level->removeEntity(packet->getItemId());
    }
}

std::wstring ClientPacketListener::GetDisplayNameByGamertag(std::wstring gamertag)
{
    return CGameNetworkManager::sInstance.GetDisplayNameByGamertag(gamertag);
}
void ClientPacketListener::handleChat(boost::shared_ptr<ClientboundChatPacket> packet)
{
    bool replacePlayer = false;
    bool replaceSource = false;
    bool replaceItem = false;
    bool showMessage = true;
    bool singleMessage = false;
    int singleMessageDuration = 0;
    boost::unordered_map<std::wstring, std::wstring> replacements;
    std::wstring playerName(L"");
    std::wstring sourceName(L"");
    std::wstring text;

    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && IsPrimaryPlayer())
    {
        m_minecraft->GetClientMasterGameMode()->OnChatPacket(packet);
    }

    if (packet->strings.size() > 0)
        playerName = GetDisplayNameByGamertag(packet->strings[0]);
    if (packet->strings.size() > 1)
        sourceName = GetDisplayNameByGamertag(packet->strings[1]);

    switch (static_cast<int>(packet->message))
    {
    case 1:
        text = L10N::GetString(0xd08616d9);
        break;
    case 2:
        text = L10N::GetString(0xfa2a57fa);
        break;
    case 3:
        text = L10N::GetString(0xe08b6a47);
        break;
    case 4:
        text = L10N::GetString(0x16c81b7f);
        break;
    case 5:
        text = L10N::GetString(0xf084ec0c);
        break;
    case 6:
        text = L10N::GetString(0x56c64197);
        text.replace(text.find(L"%s"), 2, playerName);
        break;
    case 7:
        text = L10N::GetString(0xfab38114);
        break;
    case 8:
        text = L10N::GetString(0x095fce8b);
        text.replace(text.find(L"%s"), 2, playerName);
        break;
    case 9:
        text = L10N::GetString(0x941d21c3);
        text.replace(text.find(L"%s"), 2, playerName);
        break;
    case 10:
        text = L10N::GetString(0x3677fa23);
        text.replace(text.find(L"%s"), 2, playerName);
        break;
    case 11:
        text = L10N::GetString(0x49b6297d);
        replacements[L"{*PLAYER*}"] = playerName;
        replacements[L"{*PLAYER2*}"] = sourceName;
        replacements[L"{*COUNT*}"] = toString(packet->ints[0]);
        replacements[L"{*MAX_COUNT*}"] = toString(packet->ints[1]);
        break;
    case 12:
        showMessage = false;
        g_pApp->SetGlobalXuiAction(static_cast<eXuiAction>(0x21));
        break;
    case 13:
        text = L10N::GetString(0xefc9fef0);
        replacePlayer = true;
        break;
    case 14:
    case 53:
        text = L10N::GetString(0xf4980932);
        replacePlayer = true;
        break;
    case 15:
        text = L10N::GetString(0xd711d936);
        replacePlayer = true;
        break;
    case 16:
        text = L10N::GetString(0x5eff1005);
        replacePlayer = true;
        break;
    case 17:
        text = L10N::GetString(0x6ae921b6);
        replacePlayer = true;
        break;
    case 19:
        text = L10N::GetString(0x576593ef);
        replacePlayer = true;
        break;
    case 20:
        text = L10N::GetString(0xf244df30);
        replacePlayer = true;
        break;
    case 21:
        text = L10N::GetString(0x2bf5a7dc);
        replacePlayer = true;
        break;
    case 22:
        text = L10N::GetString(0xd63d2190);
        replacePlayer = true;
        break;
    case 23:
        text = L10N::GetString(0x15c8f4ae);
        replacePlayer = true;
        break;
    case 24:
        text = L10N::GetString(0x6f2c7352);
        replacePlayer = true;
        break;
    case 25:
        if (packet->ints[0] == 0)
        {
            if (packet->strings[1].compare(L"") != 0)
            {
                const wchar_t* rulesText = g_pApp->GetGameRulesString(packet->strings[1]);
                if (rulesText != NULL)
                    text = std::wstring(rulesText);
            }
        }
        else
        {
            text = L10N::GetString(packet->ints[0]);
        }
        replacePlayer = true;
        break;
    case 26:
        text = L10N::GetString(0xd98e2236);
        replacePlayer = true;
        break;
    case 27:
    case 40:
        text = L10N::GetString(0xb04cad6a);
        replacePlayer = true;
        break;
    case 28:
        text = L10N::GetString(0x189c795e);
        replacePlayer = true;
        break;
    case 29:
        text = L10N::GetString(0x4bec30bf);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 30:
        text = L10N::GetString(0x3c5da2b4);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 31:
        text = L10N::GetString(0x7a2a89ae);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 32:
        text = L10N::GetString(0x0242d45c);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 33:
        text = L10N::GetString(0xb3fb4f7f);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 34:
        text = L10N::GetString(0x3fc22003);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 35:
        text = L10N::GetString(0x2dcf5941);
        replacePlayer = true;
        break;
    case 36:
        text = L10N::GetString(0x0d235d4b);
        replacePlayer = true;
        break;
    case 37:
        text = L10N::GetString(0xa1216033);
        replacePlayer = true;
        break;
    case 39:
        text = L10N::GetString(0x5ac8afbb);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 42:
        text = L10N::GetString(0x80d4c2e3);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 43:
        text = L10N::GetString(0x62f8280d);
        replacePlayer = true;
        break;
    case 44:
        text = L10N::GetString(0x461ba4e8);
        replacePlayer = true;
        break;
    case 45:
        text = L10N::GetString(0xc130636a);
        replacePlayer = true;
        break;
    case 46:
        text = L10N::GetString(0x44e2dd06);
        replacePlayer = true;
        break;
    case 47:
        text = L10N::GetString(0x8bc1a50e);
        replacePlayer = true;
        break;
    case 48:
        text = L10N::GetString(0x1ece8cfc);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 49:
        text = L10N::GetString(0xd871bb46);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case 50:
        text = L10N::GetString(0x7ba47e90);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 51:
        text = L10N::GetString(0x3ec0c74a);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case 52:
        text = L10N::GetString(0xc30a962c);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 54:
        text = L10N::GetString(0x48ffbd9a);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 55:
        text = L10N::GetString(0xe2b2d991);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 56:
        text = L10N::GetString(0xcf30befb);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 57:
        text = L10N::GetString(0x7ad1a880);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 58:
        text = L10N::GetString(0xaf29a206);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 59:
        text = L10N::GetString(0x516db1a2);
        replacePlayer = true;
        break;
    case 60:
        text = L10N::GetString(0x31c3794e);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case 61:
        text = L10N::GetString(0xf0eba2e4);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case 62:
        text = L10N::GetString(0x31b789a6);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case 63:
        text = L10N::GetString(0xe3be8e0d);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case 64:
        text = L10N::GetString(0x1a3b16db);
        replacePlayer = true;
        break;
    case 65:
        text = L10N::GetString(0x9eeff8cf);
        replacePlayer = true;
        replaceSource = true;
        break;
    case 66:
        text = L10N::GetString(0x64400f01);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case 67:
        text = L10N::GetString(0x06046121);
        replacePlayer = true;
        replaceSource = true;
        replaceItem = true;
        break;
    case 68:
        text = L10N::GetString(0xd3560061);
        text.replace(text.find(L"%s"), 2, playerName);
        break;
    case 69:
        text = L10N::GetString(0x6e3a65e3);
        text.replace(text.find(L"%s"), 2, playerName);
        break;
    case 70:
        text = L10N::GetString(0x9f8bffae);
        break;
    case 71:
        text = L10N::GetString(0x82460728);
        break;
    case 72:
        text = L10N::GetString(0x1e0d17d0);
        break;
    case 73:
        text = L10N::GetString(0x3e35b6e8);
        break;
    case 74:
        text = L10N::GetString(0x70edc806);
        break;
    case 76:
        text = L10N::GetString(0xa82b5a66);
        break;
    case 77:
        text = L10N::GetString(0x16972fbf);
        break;
    case 78:
        text = L10N::GetString(0x1ee681ba);
        break;
    case 79:
        text = L10N::GetString(0x8ecd701e);
        break;
    case 80:
        text = L10N::GetString(0xd97fdba2);
        break;
    case 82:
        text = L10N::GetString(0xb399bd15);
        break;
    case 83:
        text = L10N::GetString(0x99326383);
        break;
    case 84:
        text = L10N::GetString(0x00fe5875);
        break;
    case 85:
        text = L10N::GetString(0xec344e3d);
        break;
    case 86:
        text = L10N::GetString(0x3221ece8);
        break;
    case 87:
        text = L10N::GetString(0xb2145884);
        break;
    case 88:
        text = L10N::GetString(0xb604aaff);
        break;
    case 89:
        text = L10N::GetString(0xfab1deb6);
        break;
    case 90:
        text = L10N::GetString(0xfdf65c18);
        break;
    case 91:
        text = L10N::GetString(0xc952e8b9);
        break;
    case 92:
        text = L10N::GetString(0xef12cc40);
        break;
    case 93:
        text = L10N::GetString(0xb714e0e2);
        break;
    case 94:
        text = L10N::GetString(0x3dcd926b);
        break;
    case 95:
        text = L10N::GetString(0x60a3835d);
        break;
    case 96:
        text = L10N::GetString(0xce88d7b9);
        replacePlayer = true;
        if (packet->ints[0] == 0xc800001)
        {
            replacements[L"{*DESTINATION*}"] = sourceName;
        }
        else
        {
            int auxValue = -1;
            if (packet->ints.size() > 1)
                auxValue = packet->ints[1];
            replacements[L"{*DESTINATION*}"] =
                g_pApp->getEntityName(static_cast<uint64_t>(static_cast<int64_t>(packet->ints[0])), auxValue);
        }
        break;
    case 97:
        text = L10N::GetString(0x5c19fa7a);
        replacePlayer = true;
        break;
    case 98:
        text = L10N::GetString(0x186485a6);
        replacePlayer = true;
        break;
    case 99:
    {
        const int time = packet->ints[0];
        if (time == 1000)
        {
            text = L10N::GetString(0xd982f837);
        }
        else if (time == 14000)
        {
            text = L10N::GetString(0x0087fb07);
        }
        else
        {
            text = L10N::GetString(0x24a5eddc);
            text.replace(text.find(L"%d"), 2, toString(time));
        }
        break;
    }
    case 100:
        text = L10N::GetString(0xe34b099b);
        replacements[L"{*DIFFICULTY*}"] = L10N::GetString(Difficulty::byId(packet->ints[0])->getKey());
        break;
    case 101:
        text = L10N::GetString(0xa816ec2e);
        replacements[L"{*MODE*}"] = L10N::GetString(GameType::byId(packet->ints[0])->getLabel());
        break;
    case 102:
        text = L10N::GetString(0x590236fb);
        text.replace(text.find(L"%d"), 2, toString(packet->ints[0]));
        text.replace(text.find(L"%d"), 2, toString(packet->ints[1]));
        text.replace(text.find(L"%d"), 2, toString(packet->ints[2]));
        break;
    case 103:
        text = L10N::GetString(0x334e4f83);
        break;
    case 104:
        text = L10N::GetString(0xc5814157);
        text.replace(text.find(L"%d"), 2, toString(packet->ints[0]));
        text.replace(text.find(L"%d"), 2, toString(packet->ints[1]));
        text.replace(text.find(L"%d"), 2, toString(packet->ints[2]));
        replacePlayer = true;
        break;
    case 105:
        text = L10N::GetString(0xa7d52e1f);
        replacePlayer = true;
        break;
    case 106:
        text = L10N::GetString(0xb5f2b622);
        break;
    case 107:
        text = L10N::GetString(0x3ced883f);
        break;
    case 108:
        text = L10N::GetString(0x0a09d437);
        break;
    case 109:
        text = L10N::GetString(0xda993cc6);
        replacements[L"{*MODE*}"] = L10N::GetString(GameType::byId(packet->ints[0])->getLabel());
        break;
    case 110:
        text = L10N::GetString(0x5eeb14ce);
        replacements[L"{*MODE*}"] = L10N::GetString(GameType::byId(packet->ints[0])->getLabel());
        replacePlayer = true;
        break;
    case 111:
        text = L10N::GetString(0xdd05c6b4);
        break;
    case 112:
        text = L10N::GetString(0xd2bd3683);
        replacePlayer = true;
        break;
    case 113:
        singleMessage = true;
        text = L10N::GetString(0xcfeb0335);
        break;
    case 114:
        text = L10N::GetString(0x06e12057);
        singleMessage = true;
        break;
    case 115:
        text = L10N::GetString(0x9760a9e7);
        singleMessage = true;
        break;
    case 116:
        text = L10N::GetString(0x52f1ef84);
        singleMessage = true;
        break;
    case 117:
        text = L10N::GetString(0x7cccd9d4);
        singleMessage = true;
        break;
    case 118:
        text = L10N::GetString(0x69e5ca0b);
        singleMessage = true;
        break;
    case 119:
        text = L10N::GetString(0xc4219de3);
        singleMessage = true;
        break;
    case 120:
        text = L10N::GetString(0xb0b23bcd);
        singleMessage = true;
        break;
    case 121:
        if (packet->ints[0] != 1)
        {
            text = L10N::GetString(0x5819df6f);
            text.replace(text.find(L"%d"), 2, toString(packet->ints[0]));
        }
        else
        {
            text = L10N::GetString(0x989f7641);
        }
        singleMessage = true;
        break;
    case 122:
        singleMessage = true;
        text = L10N::GetString(0x2e19c8b8);
        text.replace(text.find(L"%d"), 2, toString(packet->ints[0]));
        break;
    case 123:
        singleMessageDuration = 60;
        text = L10N::GetString(0x464a6890);
        singleMessage = true;
        break;
    case 124:
        text = L10N::GetString(0xf53f833a);
        singleMessage = true;
        break;
    case 125:
        singleMessage = true;
        text = L10N::GetString(0x0e73497d);
        text.replace(text.find(L"%d"), 2, toString(packet->ints[0]));
        break;
    case 126:
        singleMessage = true;
        text = L10N::GetString(0xd5566021);
        text.replace(text.find(L"%d"), 2, toString(packet->ints[0]));
        break;
    case 127:
    {
        const int value = packet->ints[0];
        text = L10N::GetString(0xc1a090d6);
        text.replace(text.find(L"%d"), 2, toString(value));
        singleMessage = true;
        break;
    }
    case 128:
        text = L10N::GetString(0xaf2e63a6);
        text.replace(text.find(L"%d"), 2, toString(packet->ints[0]));
        singleMessage = true;
        break;
    case 129:
        text = L10N::GetString(0xe028e842);
        text.replace(text.find(L"%d"), 2, toString(packet->ints[0]));
        singleMessage = true;
        break;
    case 130:
        singleMessage = true;
        text = L10N::GetString(0x2fb55d20);
        replacements[L"{*QUANTITY*}"] = toString(packet->ints[1]);
        replacements[L"{*OBJECT*}"] = L10N::GetString(packet->ints[2]);
        replacements[L"{*EVENT*}"] = L10N::GetString(packet->ints[3]);
        break;
    case 131:
        singleMessage = true;
        text = L10N::GetString(0xb8af706d);
        replacements[L"{*QUANTITY*}"] = toString(packet->ints[1]);
        replacements[L"{*OBJECT*}"] = L10N::GetString(packet->ints[2]);
        replacements[L"{*EVENT*}"] = L10N::GetString(packet->ints[3]);
        break;
    case 132:
        singleMessage = true;
        text = L10N::GetString(0xa19c12db);
        replacements[L"{*QUANTITY*}"] = toString(packet->ints[1]);
        replacements[L"{*OBJECT*}"] = L10N::GetString(packet->ints[2]);
        replacements[L"{*EVENT*}"] = L10N::GetString(packet->ints[3]);
        break;
    case 133:
        singleMessage = true;
        text = L10N::GetString(0x431eeece);
        replacements[L"{*QUANTITY*}"] = toString(packet->ints[1]);
        replacements[L"{*OBJECT*}"] = L10N::GetString(packet->ints[2]);
        replacements[L"{*EVENT*}"] = L10N::GetString(packet->ints[3]);
        break;
    case 134:
        singleMessage = true;
        text = L10N::GetString(0x1b75ffcc);
        replacements[L"{*QUANTITY*}"] = toString(packet->ints[0]);
        replacements[L"{*OBJECT*}"] = L10N::GetString(packet->ints[2]);
        replacements[L"{*EVENT*}"] = L10N::GetString(packet->ints[3]);
        break;
    case 135:
        singleMessage = true;
        text = L10N::GetString(0xaae0d4ff);
        replacements[L"{*QUANTITY*}"] = toString(packet->ints[0]);
        break;
    case 136:
        singleMessage = true;
        text = L10N::GetString(0x6acb714a);
        replacements[L"{*QUANTITY*}"] = toString(packet->ints[0]);
        break;
    case 137:
        text = L10N::GetString(0xa292a825);
        break;
    case 138:
        text = L10N::GetString(0x3041d7ac);
        break;
    case 139:
        text = L10N::GetString(0x90c46bbb);
        break;
    case 140:
        text = L10N::GetString(0xb7fc5190);
        break;
    case 141:
        text = L10N::GetString(0x40bc050c);
        replacements[L"{*SIZE*}"] = toString(packet->ints[0]);
        break;
    case 142:
        text = L10N::GetString(0x09988b49);
        break;
    case 143:
        text = L10N::GetString(0xc08f5c90);
        replacements[L"{*PLAYER*}"] = packet->strings[0];
        replacements[L"{*TEAM*}"] = packet->strings[1];
        break;
    case 144:
        text = L10N::GetString(0x05f0b230);
        replacements[L"{*PLAYER*}"] = packet->strings[0];
        break;
    case 145:
        text = L10N::GetString(0xadc5f08e);
        replacements[L"{*PLAYER*}"] = packet->strings[0];
        replacements[L"{*TEAM*}"] = packet->strings[1];
        break;
    case 146:
        text = L10N::GetString(0x87c37625);
        replacements[L"{*TEAM*}"] = packet->strings[0];
        break;
    case 147:
        text = L10N::GetString(0xe9d46798);
        replacements[L"{*PLAYER*}"] = packet->strings[0];
        replacements[L"{*TEAM*}"] = packet->strings[1];
        break;
    case 148:
        text = L10N::GetString(0x5bdbd6d3);
        replacements[L"{*TEAM*}"] = packet->strings[0];
        break;
    case 149:
        text = L10N::GetString(0x39b60856);
        replacements[L"{*TEAM*}"] = packet->strings[0];
        break;
    case 150:
        break;
    case 151:
        text = L10N::GetString(0x056d9f12);
        replacements[L"{*NUMBER*}"] = toString(packet->ints[0]);
        break;
    case 152:
        text = L10N::GetString(0xbc3b07cd);
        break;
    case 153:
        text = L10N::GetString(0xd296c94c);
        break;
    case 154:
        singleMessage = true;
        text = L10N::GetString(0xdb4972f1);
        replacePlayer = true;
        text.replace(text.find(L"%d"), 2, toString(packet->ints[0]));
        text.replace(text.find(L"%d"), 2, toString(packet->ints[1]));
        break;
    case 155:
        text = L10N::GetString(0x6a14e1e7);
        break;
    default:
        text = playerName;
        break;
    }

    if (g_pApp->IsConsoleChatCensored() && !g_pApp->IsUUIDLocal(sourceName) && replaceItem)
    {
        replaceItem = false;
        switch (static_cast<int>(packet->message))
        {
        case 49:
        case 51:
            text = L10N::GetString(0x7ba47e90);
            break;
        case 60:
            text = L10N::GetString(0x3c5da2b4);
            break;
        case 61:
            text = L10N::GetString(0x7a2a89ae);
            break;
        case 62:
            text = L10N::GetString(0x0242d45c);
            break;
        case 63:
            text = L10N::GetString(0xb3fb4f7f);
            break;
        case 67:
            text = L10N::GetString(0x3fc22003);
            break;
        default:
            break;
        }
    }

    if (replacePlayer)
        replacements[L"{*PLAYER*}"] = playerName;

    if (replaceSource)
    {
        if (packet->ints[0] == 0xc800001)
            replacements[L"{*SOURCE*}"] = sourceName;
        else
            replacements[L"{*SOURCE*}"] = packet->getMobCustomName(m_level);
    }

    if (replaceItem)
        replacements[L"{*ITEM*}"] = packet->strings[2];

    const bool deathMessage = packet->message > 12 && packet->message < 68;
    const bool aboutThisPlayer = playerName.compare(gProfile->GetMiiName(m_playerId)) == 0;
    if (showMessage)
    {
        makeReplacements(replacements, text);
        if (singleMessage && GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(0x26)) != 0)
            m_minecraft->mGui->setSingleMessage(text, m_playerId, singleMessageDuration);
        else
            m_minecraft->mGui->addMessage(text, m_playerId, deathMessage, aboutThisPlayer, false, false);
    }
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
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    boost::shared_ptr<Entity> entity = level->getEntity(packet->getId());
    if (entity == 0)
        return;
    const int action = packet->getAction();
    const bool local = isLocalPlayer(packet->getId()) != 0;
    if ((action == 0 || action == 3) && !local)
    {
        boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
        if (living != 0)
            living->animateHurt();
    }
    else if (action == 1 && !local)
    {
        boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
        if (living != 0)
            living->swing(InteractionHand::eInteractionHand_MainHand);
    }
    else if (action == 2)
    {
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
        if (player != 0)
            player->stopSleepInBed(false, false, false);
    }
    else if ((action == 4 || action == 5) && m_minecraft != 0 && m_minecraft->mParticleEngine != 0)
    {
        const ParticleType* type = action == 4 ? PARTICLE_CRIT : PARTICLE_MAGIC_CRIT;
        m_minecraft->mParticleEngine->createTrackingEmitter(entity, type);
    }
}

void ClientPacketListener::handleEntityActionAtPosition(boost::shared_ptr<ClientboundPlayerSleepPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    boost::shared_ptr<Player> player = packet->getPlayer(m_level);
    if (player == 0)
        return;
    player->startSleepInBed(packet->getPos(), false);
    if (m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& local = m_minecraft->mLocalPlayers[m_playerId];
    if (local != 0 && local.get() == player.get() && m_minecraft->mGameRenderer != 0)
        m_minecraft->mGameRenderer->moveCameraToPlayer(1.0f);
}


void ClientPacketListener::handlePreLogin(boost::shared_ptr<ClientboundPreLoginPacket> packet)
{
    int friendCheckResult = 1;
    int playCheckResult = 1;
    int secondaryMessageVariant = 1;
    int contentCheckResult = 1;
    int restrictionResult = 0;

    CGameNetworkManager::sInstance.SetIsInPublicJoinableGame(packet->mHardcore);
    const bool publicJoinable = CGameNetworkManager::sInstance.IsInPublicJoinableGame();
    if (!CGameNetworkManager::sInstance.IsHost())
    {
        gApp.SetExtraGameData(static_cast<eGameExtraData>(6), 0);
        gApp.SetGameHostOption(static_cast<eGameHostOption>(0x20), packet->mHostPlayer);
        CGameNetworkManager::sInstance.ClearInviteInfo();
        if (gApp.GetTMSGlobalFileListRead() == 0)
        {
            const int action = gApp.GetTMSAction(gProfile->GetPrimaryPad());
            if (action == 0)
                gApp.SetTMSAction(gProfile->GetPrimaryPad(), 3);
        }
    }

    CGameNetworkManager::sInstance.CheckFriendsCanPlay(
        m_playerId, friendCheckResult, packet, publicJoinable, playCheckResult,
        secondaryMessageVariant, contentCheckResult, restrictionResult);

    if (playCheckResult != 0 && friendCheckResult != 0 && contentCheckResult != 0 && restrictionResult == 0)
    {
        if (IsPrimaryPlayer())
        {
            Minecraft* minecraft = Minecraft::GetInstance();
            minecraft->mTexturePackRepository->selectTexturePackById(packet->mGameType, false);
        }
        if (!CGameNetworkManager::sInstance.IsHost())
            m_minecraft->mProgressRenderer->progressStagePercentage(25);

        PlayerUID onlineUid(0);
        PlayerUID offlineUid(0);
        if (gProfile->IsSignedInLive(m_playerId))
            gProfile->GetXUID(m_playerId, &onlineUid, true);
        gProfile->GetXUID(m_playerId, &offlineUid, false);

        const bool friendsOnly = CGameNetworkManager::sInstance.IsFriendsOnlyUGC(m_playerId);
        boost::shared_ptr<Packet> login(new ClientboundLoginPacket(
            m_minecraft->mUser->name, 0x13c, offlineUid, onlineUid,
            friendsOnly, packet->mSeed, GameSettings::GetPlayerSkinId(m_playerId),
            GameSettings::GetPlayerCapeId(m_playerId), gProfile->IsGuest(m_playerId), false));
        send(login);

        if (!CGameNetworkManager::sInstance.IsHost())
            m_minecraft->mProgressRenderer->progressStagePercentage(50);
        m_minecraft->mGui->clearSingleMessages(m_playerId);
        return;
    }

    if (!IsPrimaryPlayer())
    {
        int parameter = 0xb9a1bf4d;
        int title = secondaryMessageVariant ? 0x4d669d55 : static_cast<int>(0xf9885affu);
        gConsoleUIController.RequestErrorMessage(0x85d6dc81, title, &parameter, 1,
                                                 m_playerId, 0, 0, 0, true);
        gApp.SetAction(m_playerId, static_cast<eXuiAction>(0xc), 0);
        return;
    }

    DisconnectPacket::eDisconnectReason reason =
        DisconnectPacket::eDisconnectReason_ConnectionClosed;
    if (secondaryMessageVariant != 0)
    {
        if (contentCheckResult == 0)
            reason = static_cast<DisconnectPacket::eDisconnectReason>(0x19);
        else if (friendCheckResult != 0)
            reason = static_cast<DisconnectPacket::eDisconnectReason>(restrictionResult != 0 ? 0x16 : 0x18);
        else
            reason = static_cast<DisconnectPacket::eDisconnectReason>(0x14);
    }
    else
        reason = static_cast<DisconnectPacket::eDisconnectReason>(0x1b);
    gApp.SetDisconnectReason(reason);
    gApp.SetAction(gProfile->GetPrimaryPad(), static_cast<eXuiAction>(3), reinterpret_cast<void*>(1));
}

void ClientPacketListener::close()
{
    if (m_connection != 0)
        m_connection->close(DisconnectPacket::eDisconnectReason_None);
    m_closed = true;
}

void ClientPacketListener::handleAddMob(boost::shared_ptr<ClientboundAddMobPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    const double scale = 0.03125;
    const double x = static_cast<double>(packet->getX()) * scale;
    const double y = static_cast<double>(packet->getY()) * scale;
    const double z = static_cast<double>(packet->getZ()) * scale;
    const float rotScale = 360.0f / 256.0f;
    const float yRot = packet->getyRot() * rotScale;
    const float xRot = packet->getxRot() * rotScale;
    const float yHeadRot = packet->getyHeadRot() * rotScale;
    boost::shared_ptr<Entity> entity = EntityIO::newEntity(packet->getType(), level);
    boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
    if (living == 0)
        return;
    living->mXo = x;
    living->mYo = y;
    living->mZo = z;
    living->setYHeadRot(yHeadRot);
    living->mYRot = packet->getyRot();
    living->mXRot = packet->getxRot();
    living->setId(packet->getId());
    living->setUUID(packet->getUUID());
    living->absMoveTo(x, y, z, yRot, xRot);
    living->mXd = static_cast<double>(packet->getXd()) / 8000.0;
    living->mYd = static_cast<double>(packet->getYd()) / 8000.0;
    living->mZd = static_cast<double>(packet->getZd()) / 8000.0;
    level->putEntity(packet->getId(), living);
    std::vector<DataItem*>* unpacked = packet->getUnpackedData();
    if (unpacked != 0)
        living->getEntityData()->assignValues(unpacked);
    boost::shared_ptr<Slime> slime = boost::dynamic_pointer_cast<Slime>(living);
    if (slime != 0)
        slime->setSize(slime->getSize(), true);
    CompoundTag* tag = packet->getAdditionalContructionDataTag();
    if (tag != 0)
        living->readAdditionalSaveData(tag);
    checkDeferredEntityLinkPackets(living->getId());
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
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player == 0)
        return;
    const BlockPos pos = packet->getPos();
    player->setRespawnPosition(new BlockPos(pos), true);
    if (m_minecraft->mLevel != 0 && m_minecraft->mLevel->getLevelData() != 0)
        m_minecraft->mLevel->getLevelData()->setSpawn(pos);
}


void ClientPacketListener::handleSetEntityPassengersPacket(boost::shared_ptr<ClientboundSetPassengersPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    boost::shared_ptr<Entity> vehicle = level->getEntity(packet->getVehicle());
    if (vehicle == 0)
        return;
    vehicle->ejectPassengers();
    arrayWithLength<int> passengers = packet->getPassengers();
    if (passengers.data == 0)
        return;
    for (unsigned int i = 0; i < passengers.length; ++i)
    {
        boost::shared_ptr<Entity> passenger = level->getEntity(passengers.data[i]);
        if (passenger != 0)
            passenger->startRiding(vehicle, true);
    }
}


void ClientPacketListener::handleEntityLinkPacket(boost::shared_ptr<ClientboundSetEntityLinkPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    MultiPlayerLevel* level = dynamic_cast<MultiPlayerLevel*>(m_level);
    if (level == 0)
        return;
    boost::shared_ptr<Entity> source = level->getEntity(packet->getSourceId());
    boost::shared_ptr<Entity> dest = level->getEntity(packet->getDestId());
    if (dest == 0 && packet->getDestId() >= 0)
    {
        EnterCriticalSection(&s_deferredLinkMutex);
        s_deferredLinks.push_back(DeferredEntityLinkPacket(packet));
        LeaveCriticalSection(&s_deferredLinkMutex);
        return;
    }
    if (source == 0)
        return;
    if ((source->GetType() & 0xd000000) == 0xd000000)
    {
        boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(source);
        if (dest != 0)
            mob->setLeashedTo(dest, false);
        else
            mob->dropLeash(false, false);
    }
}

void ClientPacketListener::handleEntityEvent(boost::shared_ptr<ClientboundEntityEventPacket> packet)
{
    if (packet == 0)
        return;
    boost::shared_ptr<Entity> entity = getEntity(packet->getEntityId());
    if (entity == 0)
        return;
    const unsigned char eventId = packet->getEventId();
    if (eventId == 0x15)
    {
        boost::shared_ptr<Guardian> guardian = boost::static_pointer_cast<Guardian>(entity);
        m_minecraft->mSoundEngine->ConsoleSoundEngine::play(new GuardianAttackSoundInstance(guardian));
        return;
    }
    if (eventId != 0x23)
    {
        entity->handleEntityEvent(eventId, packet->getData());
        return;
    }
    if (entity == m_minecraft->mLocalPlayers[m_playerId])
    {
        not_null_ptr<ItemInstance> totemItem(new ItemInstance(Items::totemOfUndying));
        m_minecraft->mGameRenderer->displayItemActivation(totemItem, m_playerId);
    }
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
    const MiniGamePlayerSettings& settings = packet->getSettings();
    MiniGameDef::SetPlayerSettings(settings);
    GameSettings::SetPlayerSkin(m_playerId, GameSettings::GetPlayerSkinId(m_playerId), false);
    GameSettings::SetPlayerCape(m_playerId, GameSettings::GetPlayerCapeId(m_playerId), false);
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
    m_minecraft->respawnPlayer(m_playerId, packet->getDimension(), packet->getDimension(),
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
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (!packet->isKnockbackOnly() && m_level != 0 && player != 0)
    {
        std::vector<BlockPos>* toBlow = packet->getToBlow();
        Explosion explosion(m_level, player, packet->getX(), packet->getY(), packet->getZ(),
                            static_cast<float>(packet->getPower()), toBlow, 0.0f);
        explosion.finalizeExplosion(true, toBlow);
    }
    if (player != 0)
    {
        player->mXd += packet->getKnockbackX();
        player->mYd += packet->getKnockbackY();
        player->mZd += packet->getKnockbackZ();
    }
}


void ClientPacketListener::handleContainerOpen(boost::shared_ptr<ClientboundContainerOpenPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player == 0)
        return;
    const int type = packet->getType();
    if (type == 0)
    {
        boost::shared_ptr<SimpleContainer> container(
            new SimpleContainer(packet->getTitle(), packet->getSize()));
        if (player->openContainer(container) != 0 && player->mContainerMenu != 0)
            player->mContainerMenu->m_containerId = packet->getContainerId();
        return;
    }
    if (type == 6)
    {
        boost::shared_ptr<Entity> entity = resolveEntity(packet->getEntityId());
        boost::shared_ptr<Villager> villager = boost::dynamic_pointer_cast<Villager>(entity);
        if (villager == 0)
            return;
        std::wstring name = packet->hasCustomName() ? packet->getTitle() : villager->getDisplayName();
        boost::shared_ptr<ClientSideMerchant> merchant(new ClientSideMerchant(player, name));
        merchant->setVillager(villager);
        player->openTrading(merchant, name, villager->getId());
        if (player->mContainerMenu != 0)
            player->mContainerMenu->m_containerId = packet->getContainerId();
        return;
    }
    if (type == 11)
    {
        boost::shared_ptr<Entity> entity = resolveEntity(packet->getEntityId());
        boost::shared_ptr<AbstractHorse> horse = boost::dynamic_pointer_cast<AbstractHorse>(entity);
        if (horse == 0)
            return;
        std::wstring title = packet->hasCustomName() ? packet->getTitle() : horse->getDisplayName();
        boost::shared_ptr<HorseChest> chest(
            new HorseChest(packet->getEntityId(), title, packet->hasCustomName() != 0, packet->getSize()));
        if (player->openHorseInventory(horse, chest) != 0 && player->mContainerMenu != 0)
            player->mContainerMenu->m_containerId = packet->getContainerId();
        return;
    }
    if (type != 12)
    {
        if (packet->isContainer())
        {
            boost::shared_ptr<Entity> entity = resolveEntity(packet->getEntityId());
            boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(entity);
            if (container != 0 && player->openContainer(container) != 0 && player->mContainerMenu != 0)
                player->mContainerMenu->m_containerId = packet->getContainerId();
            return;
        }
        boost::shared_ptr<SimpleContainer> container(
            new LocalMenuContainer(type, packet->getTitle(), packet->getSize()));
        if (player->openContainer(boost::static_pointer_cast<Container>(container)) != 0
            && player->mContainerMenu != 0)
            player->mContainerMenu->m_containerId = packet->getContainerId();
    }
}

void ClientPacketListener::handleContainerSetSlot(boost::shared_ptr<ClientboundContainerSetSlotPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player == 0)
        return;

    not_null_ptr<ItemInstance> item = packet->getItem();
    int slot = packet->getSlot();
    int containerId = packet->getContainerId();

    if (containerId == -1)
    {
        player->mInventory->setCarried(item);
        return;
    }

    if (containerId == -2)
    {
        player->mInventory->setItem(static_cast<unsigned int>(slot), item);
        return;
    }

    if (containerId == 0)
    {
        if (slot > 0x23 && slot < 0x2d)
        {
            if (!item->isEmpty())
            {
                Slot* invSlot = player->mInventoryMenu->getSlot(slot);
                not_null_ptr<ItemInstance> existing = invSlot->getItem();
                if (existing->isEmpty() || existing->getCount() < item->getCount())
                    item->setPopTime(5);
            }
            player->mInventoryMenu->setItem(static_cast<unsigned int>(slot), item);
            return;
        }
    }

    if (containerId == player->mContainerMenu->m_containerId)
        player->mContainerMenu->setItem(static_cast<unsigned int>(slot), item);
}

void ClientPacketListener::handleContainerAck(boost::shared_ptr<ClientboundContainerAckPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player == 0)
        return;
    AbstractContainerMenu* menu;
    if (packet->getContainerId() == 0)
    {
        menu = player->mInventoryMenu;
    }
    else
    {
        if (packet->getContainerId() != player->mContainerMenu->m_containerId)
            return;
        menu = player->mContainerMenu;
    }
    if (menu != 0 && !packet->isAccepted())
    {
        send(boost::shared_ptr<Packet>(new ServerboundContainerAckPacket(
            packet->getContainerId(), static_cast<short>(packet->getUid()), true)));
    }
}


void ClientPacketListener::handleContainerContent(boost::shared_ptr<ClientboundContainerSetContentPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player == 0)
        return;
    if (packet->getContainerId() == 0)
    {
        if (player->mContainerMenu != 0)
            player->mContainerMenu->setAll(packet->getItems());
        return;
    }
    if (player->mContainerMenu != 0 &&
        player->mContainerMenu->m_containerId == packet->getContainerId())
        player->mContainerMenu->setAll(packet->getItems());
}


void ClientPacketListener::handleOpenSignEditor(boost::shared_ptr<ClientboundOpenSignEditorPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    boost::shared_ptr<BlockEntity> blockEntity = m_level->getBlockEntity(packet->getPos());
    if (blockEntity == 0 || blockEntity->GetType() != 0x1000202)
    {
        boost::shared_ptr<SignBlockEntity> newSign(new SignBlockEntity());
        newSign->setLevel(m_level);
        newSign->setPosition(packet->getPos());
        blockEntity = newSign;
    }
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    boost::shared_ptr<SignBlockEntity> sign = boost::dynamic_pointer_cast<SignBlockEntity>(blockEntity);
    player->openTextEdit(sign);
}


void ClientPacketListener::handleBlockEntityData(boost::shared_ptr<ClientboundBlockEntityDataPacket> packet)
{
    if (packet == 0 || m_minecraft == 0)
        return;
    MultiPlayerLevel* level = m_minecraft->getLevel(packet->getDimension());
    if (level == 0)
        return;
    if (level->shouldQueueBlockEntityDataPackets())
        level->QueueBlockEntityDataPacket(packet);
    else
        handleBlockEntityDataInternal(packet, level);
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
    if (packet == 0 || m_minecraft == 0)
        return;
    const int event = packet->getEvent();
    const float param = packet->getParam();
    const int ticks = static_cast<int>(std::floor(param + 0.5f));
    if (event == 1 && m_level != 0)
    {
        LevelData* levelData = m_level->getLevelData();
        if (levelData != 0)
            levelData->setRaining(true);
        m_level->setRainLevel(1.0f);
        if (ticks != 0 && levelData != 0)
            levelData->setRainTime(ticks);
    }
    else if (event == 2 && m_level != 0)
    {
        LevelData* levelData = m_level->getLevelData();
        if (levelData != 0)
            levelData->setRaining(false);
        m_level->setRainLevel(0.0f);
        if (ticks != 0 && levelData != 0)
            levelData->setRainTime(ticks);
    }
    else if (event == 8 && m_level != 0)
    {
        LevelData* levelData = m_level->getLevelData();
        if (levelData != 0)
            levelData->setThundering(ticks != 0);
        m_level->setThunderLevel(param);
    }
    else if (event == 7 && m_level != 0)
        m_level->setRainLevel(param);
    else if (event == 3)
    {
        boost::shared_ptr<Player> player = m_minecraft->GetPlayerByPlayerIndex(packet->getPlayerIndex());
        boost::shared_ptr<AbstractClientPlayer> clientPlayer =
            boost::dynamic_pointer_cast<AbstractClientPlayer>(player);
        if (clientPlayer != 0)
        {
            clientPlayer->SetGameType(GameType::byId(ticks));
            if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            {
                const MiniGameDef* miniGame = m_minecraft->GetMiniGame();
                if (miniGame != 0 && miniGame->SupressGameModeMessages())
                    return;
            }
        }
    }
    else if (event == 4)
    {
        if (!gApp.GetGameStarted())
        {
            for (int i = 0; i < 4; ++i)
            {
                if (m_minecraft->mLocalPlayers[i] != 0)
                    gApp.SetAction(i, static_cast<eXuiAction>(6), 0);
            }
        }
        else if (!gConsoleUIController.IsSceneInStack(m_playerId, static_cast<EUIScene>(0x30)))
        {
            gConsoleUIController.SetWinUserIndex(static_cast<unsigned int>(packet->getPlayerIndex()));
            MultiPlayerGameMode* mode = m_minecraft->mGameModes[m_playerId];
            if (mode != 0 && mode->getTutorial() != 0)
                mode->getTutorial()->showTutorialPopup(false);
            gConsoleUIController.CloseAllPlayersScenes(false);
            gConsoleUIController.NavigateToScene(
                m_playerId, static_cast<EUIScene>(0x30), 0,
                static_cast<EUILayer>(6), static_cast<EUIGroup>(1));
        }
    }
    else if (event == 11 && !CGameNetworkManager::sInstance.IsHost() && IsPrimaryPlayer())
    {
        gApp.SetGameStarted(false);
        gApp.SetAction(m_playerId, static_cast<eXuiAction>(0x1b), 0);
    }
    else if (event == 12 && !CGameNetworkManager::sInstance.IsHost() && IsPrimaryPlayer())
        gApp.SetGameStarted(true);
    else if (event == 6 && m_level != 0 && m_minecraft->mLocalPlayers[m_playerId] != 0)
    {
        boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
        const double eyeHeight = player->getEyeHeight();
        const BlockPos pos(static_cast<int>(player->mX),
                           static_cast<int>(player->mY + eyeHeight),
                           static_cast<int>(player->mZ));
        m_level->levelEvent(2005, pos, static_cast<int>(param));
    }
    else if (event == 10 && m_level != 0 && m_minecraft->mLocalPlayers[m_playerId] != 0)
    {
        boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
        m_level->addParticle(
            PARTICLE_TOTEM, false, player->mX, player->mY, player->mZ,
            0.0, 0.0, 0.0, arrayWithLength<int>());
    }
    else if (event == 0 && m_minecraft->mLocalPlayers[m_playerId] != 0)
        m_minecraft->mLocalPlayers[m_playerId]->playSound(SoundEvents::PLAYER_LEVELUP, 1.0f, 1.0f);
}

void ClientPacketListener::handleMapItemData(boost::shared_ptr<ClientboundMapItemDataPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_minecraft->mGameRenderer == 0)
        return;
    MapRenderer* mapRenderer = m_minecraft->mGameRenderer->getMapRenderer();
    boost::shared_ptr<MapItemSavedData> mapData =
        MapItem::getSavedDataIfExists(packet->getMapId(), m_minecraft->mLevel);
    if (mapData != 0)
    {
        packet->applyToMap(mapData.get());
        mapRenderer->update(mapData);
    }
}

void ClientPacketListener::handleLevelEvent(boost::shared_ptr<ClientboundLevelEventPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_minecraft->mLevel == 0)
        return;
    if (packet->isGlobalEvent())
        m_minecraft->mLevel->globalLevelEvent(packet->getType(), packet->getPos(), packet->getData());
    else
        m_minecraft->mLevel->levelEvent(packet->getType(), packet->getPos(), packet->getData());
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
    if (packet == 0)
        return;
    boost::shared_ptr<Entity> entity = resolveEntity(static_cast<int>(packet->getEntityId()));
    if (entity == 0 || !Entity::instanceof(entity, 0x0C000000ULL))
        return;
    boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
    if (living == 0)
        return;
    MobEffect* effect = MobEffect::byId(packet->getEffectId());
    if (effect == 0)
        return;
    living->addEffect(new MobEffectInstance(effect, packet->getEffectDurationTicks(),
                                            packet->getEffectAmplifier(),
                                            packet->isEffectAmbient() != 0,
                                            packet->isEffectVisible() != 0),
                      boost::shared_ptr<Entity>());
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
    if (packet == 0 || m_level == 0)
        return;
    boost::shared_ptr<Entity> entity = packet->getEntity(m_level);
    boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
    if (living == 0)
        return;
    MobEffect* effect = packet->getEffect();
    if (effect != 0)
        living->removeEffect(effect);
}


bool ClientPacketListener::isServerPacketListener() { return false; }
void ClientPacketListener::handlePlayerInfo(boost::shared_ptr<PlayerInfoPacket> packet)
{
    if (packet == 0)
        return;
    uint32_t privileges = g_pApp->GetPlayerPrivileges(static_cast<uint8_t>(packet->mPlayerId));

    INetworkPlayer* networkPlayer =
        CGameNetworkManager::sInstance.GetPlayerBySmallId(static_cast<uint8_t>(packet->mPlayerId));
    if (networkPlayer != 0 && networkPlayer->IsHost())
    {
        Player::enableAllPlayerPrivileges(privileges, true);
        Player::setPlayerGamePrivilege(privileges, Player::ePlayerGamePrivilege_Moderator, 1);
    }

    g_pApp->UpdatePlayerInfo(static_cast<uint8_t>(packet->mPlayerId), packet->mPlayerIndex,
                             packet->mPrivileges, packet->mGameType);

    boost::shared_ptr<Entity> entity = getEntity(packet->mEntityId);
    if (entity != 0 && (entity->GetType() & 0xc800000) == 0xc800000)
    {
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All, packet->mPrivileges);
    }

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
    if (packet == 0 || m_minecraft == 0)
        return;
    if (packet->getSoundInstance() == ClientboundSoundPacket::eSoundInstances_Default)
    {
        if (packet->getPlayUI())
        {
            if (m_minecraft->mSoundEngine != 0)
                m_minecraft->mSoundEngine->playUI(packet->getSound(),
                                                  static_cast<float>(packet->getVolume()),
                                                  static_cast<float>(packet->getPitch()));
            return;
        }
        if (m_minecraft->mSoundEngine == 0)
            return;
        m_minecraft->mSoundEngine->play(packet->getSound(),
                                         static_cast<float>(packet->getX()),
                                         static_cast<float>(packet->getY()),
                                         static_cast<float>(packet->getZ()),
                                         static_cast<float>(packet->getVolume()),
                                         static_cast<float>(packet->getPitch()),
                                         NULL);
        return;
    }
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player == 0 || m_minecraft->mSoundEngine == 0)
        return;
    m_minecraft->mSoundEngine->ConsoleSoundEngine::play(new FollowPlayerSoundInstance(
        player, packet->getSound(),
        static_cast<float>(packet->getVolume()), static_cast<float>(packet->getPitch()),
        packet->getLooping()));
}

void ClientPacketListener::handleMoveVehicle(boost::shared_ptr<ClientboundMoveVehiclePacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player == 0)
        return;
    boost::shared_ptr<Entity> vehicle = player->getVehicle();
    if (vehicle == 0 || vehicle.get() == player.get() || !player->isPassenger())
        return;
    vehicle->absMoveTo(packet->getX(), packet->getY(), packet->getZ(),
                       static_cast<float>(packet->getYRot()), static_cast<float>(packet->getXRot()));
    send(boost::shared_ptr<Packet>(new ServerboundMoveVehiclePacket(vehicle)));
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
    if (packet == 0 || m_minecraft == 0 || m_playerId < 0 || m_playerId >= 4)
        return;
    if (packet->getIdentifier() != L"MC|TrList")
        return;
    arrayWithLength<unsigned char> payload = packet->getData();
    ByteArrayInputStream byteStream(payload);
    DataInputStream input(&byteStream);
    const int containerId = input.readInt();
    if (!gConsoleUIController.IsSceneInStack(m_playerId, static_cast<EUIScene>(0x32)))
    {
        byteStream.mBuffer.data = NULL;
        byteStream.mBuffer.length = 0;
        byteStream.mCount = 0;
        byteStream.mMark = 0;
        byteStream.mPosition = 0;
        return;
    }
    boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
    if (player == 0 || player->mContainerMenu == 0 ||
        player->mContainerMenu->m_containerId != containerId)
    {
        byteStream.mBuffer.data = NULL;
        byteStream.mBuffer.length = 0;
        byteStream.mCount = 0;
        byteStream.mMark = 0;
        byteStream.mPosition = 0;
        return;
    }
    void* scene = gConsoleUIController.GetTopScene(m_playerId, static_cast<EUILayer>(6),
                                                   static_cast<EUIGroup>(7));
    if (scene == 0)
    {
        byteStream.mBuffer.data = NULL;
        byteStream.mBuffer.length = 0;
        byteStream.mCount = 0;
        byteStream.mMark = 0;
        byteStream.mPosition = 0;
        return;
    }
    UIScene_TradingMenu* tradingMenu = static_cast<UIScene_TradingMenu*>(scene);
    boost::shared_ptr<MerchantMenu> menu = tradingMenu->getMerchant();
    if (menu == 0)
    {
        byteStream.mBuffer.data = NULL;
        byteStream.mBuffer.length = 0;
        byteStream.mCount = 0;
        byteStream.mMark = 0;
        byteStream.mPosition = 0;
        return;
    }
    boost::shared_ptr<Merchant> merchant = menu->getMerchant();
    boost::shared_ptr<ClientSideMerchant> clientMerchant =
        boost::dynamic_pointer_cast<ClientSideMerchant>(merchant);
    if (clientMerchant == 0)
    {
        byteStream.mBuffer.data = NULL;
        byteStream.mBuffer.length = 0;
        byteStream.mCount = 0;
        byteStream.mMark = 0;
        byteStream.mPosition = 0;
        return;
    }
    MerchantRecipeList* offers = MerchantRecipeList::createFromStream(&input);
    clientMerchant->overrideOffers(offers);
    byteStream.mBuffer.data = NULL;
    byteStream.mBuffer.length = 0;
    byteStream.mCount = 0;
    byteStream.mMark = 0;
    byteStream.mPosition = 0;
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
    if (packet == 0)
        return;
    if (packet->length == 0)
    {
        unsigned char* data = 0;
        unsigned int length = 0;
        gApp.GetMemFileDetails(packet->textureName, &data, &length);
        if (length != 0)
            send(boost::shared_ptr<Packet>(new TexturePacket(packet->textureName, data, length)));
    }
    else
    {
        gApp.AddMemoryTextureFile(packet->textureName, packet->data, packet->length);
        Minecraft::GetInstance()->handleClientTextureReceived(packet->textureName);
    }
}

void ClientPacketListener::handleTextureAndGeometry(boost::shared_ptr<TextureAndGeometryPacket> packet)
{
    if (packet->length == 0)
        return;
    gApp.AddMemoryTextureFile(packet->textureName, packet->data, packet->length);
    if (packet->additionalBoxCount != 0)
        gApp.SetAdditionalSkinBoxes(packet->geometryId, packet->additionalBoxes,
                                    packet->additionalBoxCount, packet->skinAdjustments);
    gApp.SetSkinAdjustments(packet->geometryId, packet->skinAdjustments);
    Minecraft::GetInstance()->handleClientTextureReceived(packet->textureName);
}

void ClientPacketListener::handleUpdateProgress(boost::shared_ptr<UpdateProgressPacket> packet)
{
    if (!CGameNetworkManager::sInstance.IsHost() && m_minecraft != 0 && packet != 0 &&
        m_minecraft->mProgressRenderer != 0)
        m_minecraft->mProgressRenderer->progressStagePercentage(packet->mProgress);
}

void ClientPacketListener::handleTextureChange(boost::shared_ptr<TextureChangePacket> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(packet->mEntityId);
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
    if (player == 0)
        return;
    for (int i = 0; i < 4; ++i)
    {
        if (m_minecraft->mLocalPlayers[i] != 0 && m_minecraft->mLocalPlayers[i]->getId() == packet->mEntityId)
            return;
    }
    if (packet->mChangeType == TextureChangePacket::eTextureChangeType_Skin)
    {
        const unsigned int skinId = CMinecraftApp::getSkinIdFromPath(packet->mTextureName);
        if (!gApp.IsFileInMemoryTextures(packet->mTextureName))
        {
            if (m_minecraft != 0 && m_minecraft->addPendingClientTextureRequest(packet->mTextureName))
                send(boost::shared_ptr<Packet>(new TexturePacket(packet->mTextureName, 0, 0)));
        }
        player->setCustomSkin(skinId);
    }
    else if (packet->mChangeType == TextureChangePacket::eTextureChangeType_Cape)
    {
        const unsigned int capeId = Player::getCapeIdFromPath(packet->mTextureName);
        if (!gApp.IsFileInMemoryTextures(packet->mTextureName))
        {
            if (m_minecraft != 0 && m_minecraft->addPendingClientTextureRequest(packet->mTextureName))
                send(boost::shared_ptr<Packet>(new TexturePacket(packet->mTextureName, 0, 0)));
        }
        player->setCustomCape(capeId);
    }
}

void ClientPacketListener::handleTextureAndGeometryChange(boost::shared_ptr<TextureAndGeometryChangePacket> packet)
{
    boost::shared_ptr<Entity> entity = getEntity(packet->entityId);
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
    if (player == 0)
        return;
    for (int i = 0; i < 4; ++i)
    {
        if (m_minecraft->mLocalPlayers[i] != 0 && m_minecraft->mLocalPlayers[i]->getId() == packet->entityId)
            return;
    }
    if (!gApp.IsFileInMemoryTextures(packet->geometryName))
    {
        if (m_minecraft != 0 && m_minecraft->addPendingClientTextureRequest(packet->geometryName))
            send(boost::shared_ptr<Packet>(
                new TextureAndGeometryPacket(packet->geometryName, 0, 0)));
    }
    player->setCustomSkin(CMinecraftApp::getSkinIdFromPath(packet->geometryName));
}

void ClientPacketListener::handleUpdateGameRuleProgressPacket(boost::shared_ptr<UpdateGameRuleProgressPacket> packet)
{
    if (packet == 0)
        return;
    const wchar_t* rulesText = gApp.GetGameRulesString(packet->mDescription);
    if (rulesText == 0)
        return;
    std::wstring description(rulesText);
    std::wstring message = GameRuleDefinition::generateDescriptionString(
        packet->mRuleType, description, packet->mPayload.data,
        static_cast<int>(packet->mPayload.length));
    if (m_minecraft != 0 && m_playerId >= 0 && m_playerId < 4 &&
        m_minecraft->mGameModes[m_playerId] != 0)
    {
        Tutorial* tutorial = m_minecraft->mGameModes[m_playerId]->getTutorial();
        if (tutorial != 0)
            tutorial->setMessage(message, packet->mAuxValue, packet->mDataTag);
    }
    if (packet->mDataTag > 0 && packet->mDataTag < 33)
        GameSettings::SetSpecialTutorialCompletionFlag(m_playerId, packet->mDataTag - 1);
    if (packet->mPayload.data != 0)
    {
        delete[] packet->mPayload.data;
        packet->mPayload.data = 0;
        packet->mPayload.length = 0;
    }
}


void ClientPacketListener::handleXZ(boost::shared_ptr<XZPacket> packet)
{
    if (packet == 0 || m_minecraft == 0 || m_minecraft->mLevel == 0)
        return;
    if (packet->getMode() != 6)
        return;
    LevelData* levelData = m_minecraft->mLevel->getLevelData();
    if (levelData == 0)
        return;
    levelData->setXSpawn(packet->getX());
    levelData->setZSpawn(packet->getZ());
}


void ClientPacketListener::handleScoreboardPacket(boost::shared_ptr<ScoreboardPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    ClientScoreboard* scoreboard = dynamic_cast<ClientScoreboard*>(m_level->GetScoreboard());
    if (scoreboard == 0)
        return;
    switch (packet->GetMessage())
    {
    case ScoreboardPacket::ADD_PLAYER_STATS:
    {
        ScoreboardPacket::AddPlayerStatsData* data =
            dynamic_cast<ScoreboardPacket::AddPlayerStatsData*>(packet->getData());
        if (data == 0 || data->mStats == 0)
            return;
        scoreboard->SyncPlayerStats(data->mStats->getRecordSUID(), data->mStats, data->mRound);
        break;
    }
    case ScoreboardPacket::UPDATE_PLAYER_STATS:
    {
        ScoreboardPacket::UpdatePlayerStatsData* data =
            dynamic_cast<ScoreboardPacket::UpdatePlayerStatsData*>(packet->getData());
        if (data == 0 || data->mStats == 0)
            return;
        scoreboard->UpdatePlayerStats(data->mStats->getRecordSUID(), data->mStats,
                                      data->mRound, data->mLife, data->mChanged);
        if (Minecraft::InMiniGame(eMiniGameId_Glide, false) && m_minecraft != 0)
        {
            const int checkpoint = scoreboard->GetLastCheckpointID(data->mStats->getRecordSUID(),
                                                                 data->mRound);
            ClientMasterGameMode* master = m_minecraft->GetClientMasterGameMode();
            if (master != 0)
                master->SetLastCheckpointID(data->mStats->getRecordSUID(), checkpoint, true);
            if (checkpoint == 0)
            {
                const int banked = scoreboard->GetLastBankedCheckpointID(
                    data->mStats->getRecordSUID(), data->mRound);
                if (banked != 0 && master != 0)
                    master->SetLocalRoundTimeStartOverride(System::processTimeInMilliSecs());
            }
        }
        break;
    }
    case ScoreboardPacket::REMOVE_PLAYER_STATS:
    {
        ScoreboardPacket::RemovePlayerStatsData* data =
            dynamic_cast<ScoreboardPacket::RemovePlayerStatsData*>(packet->getData());
        if (data == 0)
            return;
        scoreboard->RemovePlayerStats(data->mUID);
        break;
    }
    case ScoreboardPacket::CLEAR_PLAYER_STATS:
        scoreboard->ResetPlayerScores();
        break;
    default:
        break;
    }
}


void ClientPacketListener::handleClientboundSetPlayerTeamPacket(boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    Scoreboard* scoreboard = m_level->GetScoreboard();
    if (scoreboard == 0)
        return;
    PlayerTeam* team = 0;
    const int method = packet->getMethod();
    if (method == 0)
        team = scoreboard->addPlayerTeam(packet->getName());
    else
        team = scoreboard->getPlayerTeam(packet->getName());
    if (team == 0)
        return;
    if (method == 0 || method == 2)
    {
        team->unpackOptions(packet->getOptions());
        team->setNameTagVisibility(static_cast<Team::Visibility>(packet->getNameTagVisibility()));
    }
    if (method == 0 || method == 3)
    {
        std::vector<PlayerUID>* players = packet->getPlayers();
        if (players != 0)
        {
            for (std::vector<PlayerUID>::iterator it = players->begin(); it != players->end(); ++it)
                scoreboard->addPlayerToTeam(*it, team);
        }
    }
    if (method == 4)
    {
        std::vector<PlayerUID>* players = packet->getPlayers();
        if (players != 0)
        {
            for (std::vector<PlayerUID>::iterator it = players->begin(); it != players->end(); ++it)
                scoreboard->removePlayerFromTeam(*it, team);
        }
    }
    if (method == 1)
        scoreboard->removePlayerTeam(team);
}


void ClientPacketListener::handleParticleEvent(boost::shared_ptr<ClientboundLevelParticlesPacket> packet)
{
    if (packet == 0 || m_level == 0)
        return;
    const ParticleType* type = packet->getType();
    if (type == 0)
        return;
    const double x = packet->getX();
    const double y = packet->getY();
    const double z = packet->getZ();
    const int count = packet->getCount();
    if (count == 0)
    {
        double xa = 0.0;
        double ya = 0.0;
        double za = 0.0;
        if (packet->getHasAux() == 0)
        {
            xa = static_cast<double>(packet->getMaxSpeed()) * packet->getXDist();
            ya = static_cast<double>(packet->getMaxSpeed()) * packet->getYDist();
            za = static_cast<double>(packet->getMaxSpeed()) * packet->getZDist();
        }
        m_level->addParticle(type, packet->isOverrideLimiter(), x, y, z, xa, ya, za, packet->getParams());
        return;
    }
    for (int i = 0; i < count; ++i)
    {
        const double px = x + (m_random.nextGaussian() * packet->getXDist());
        const double py = y + (m_random.nextGaussian() * packet->getYDist());
        const double pz = z + (m_random.nextGaussian() * packet->getZDist());
        double xa = m_random.nextGaussian() * packet->getMaxSpeed();
        double ya = m_random.nextGaussian() * packet->getMaxSpeed();
        double za = m_random.nextGaussian() * packet->getMaxSpeed();
        if (packet->getHasAux() != 0)
        {
            xa = packet->getXAux();
            ya = packet->getYAux();
            za = packet->getZAux();
        }
        m_level->addParticle(type, packet->isOverrideLimiter(), px, py, pz, xa, ya, za, packet->getParams());
    }
}

void ClientPacketListener::handleUpdateAttributes(boost::shared_ptr<ClientboundUpdateAttributesPacket> packet)
{
    if (packet == 0)
        return;
    boost::shared_ptr<Entity> entity = resolveEntity(packet->getEntityId());
    if (entity == 0 || !Entity::instanceof(entity, 0x0C800000ULL))
        return;
    if (static_cast<int>(entity->GetType()) != packet->getEntityType())
        return;
    boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
    if (living == 0 || living->getAttributes() == 0)
        return;
    boost::unordered_set<ClientboundUpdateAttributesPacket::AttributeSnapshot*>* snapshots =
        packet->getValues();
    if (snapshots == 0)
        return;
    for (boost::unordered_set<ClientboundUpdateAttributesPacket::AttributeSnapshot*>::iterator it =
             snapshots->begin();
         it != snapshots->end();
         ++it)
    {
        ClientboundUpdateAttributesPacket::AttributeSnapshot* snapshot = *it;
        if (snapshot == 0)
            continue;
        Attribute* attribute = attributeById(snapshot->getId());
        if (attribute == 0)
            continue;
        AttributeInstance* instance = living->getAttributes()->getInstance(attribute);
        if (instance == 0)
            instance = living->getAttributes()->registerAttribute(attribute);
        if (instance == 0)
            continue;
        instance->setBaseValue(snapshot->getBase());
        instance->removeModifiers();
        boost::unordered_set<AttributeModifier*>* modifiers = snapshot->getModifiers();
        if (modifiers != 0)
            instance->addModifiers(modifiers);
    }
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
    if (packet == 0 || m_minecraft == 0)
        return;
    ClientMasterGameMode* master = m_minecraft->GetClientMasterGameMode();
    switch (packet->GetMessage())
    {
    case 3:
    {
        if (!IsPrimaryPlayer())
            break;
        GameModePacket::TransitionToRoundData data;
        packet->ReadPacket(data);
        GameModeTransitionParams* params = new GameModeTransitionParams(
            0, false, false, 1, data.x, data.y, data.level, data.round);
        if (master != 0)
        {
            master->queueTransition(params);
            master->SetCurrentTheme(data.round, data.seed);
        }
        gConsoleUIController.HidePressStart();
        if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
        {
            LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
            if (options != 0)
            {
                const std::wstring worldNameKey = options->getWorldNameKey();
                const MiniGameDef* miniGame = m_minecraft->GetMiniGame();
                const bool hidePoints = miniGame != 0 && !miniGame->ArePointsActive();
                GhostController* ghost = m_minecraft->GetGhostController();
                if (ghost == 0)
                {
                    ghost = new GhostController(m_playerId, worldNameKey, hidePoints);
                    m_minecraft->SetGhostController(ghost);
                }
                else
                    ghost->reset(m_playerId, worldNameKey, hidePoints);
            }
        }
        break;
    }
    case 4:
    {
        if (!IsPrimaryPlayer())
            break;
        GameModePacket::TransitionData data;
        packet->ReadPacket(data);
        GameModeTransitionParams* params = new GameModeTransitionParams(
            0, false, true, 0, data.x, data.y, data.level, 0x14);
        if (master != 0)
            master->queueTransition(params);
        break;
    }
    case 5:
    {
        if (!IsPrimaryPlayer() || !CGameNetworkManager::sInstance.IsHost())
            break;
        GameModePacket::IntData data;
        packet->ReadPacket(data);
        if (master != 0)
        {
            master->SetBRUPSToWaitOn(data.value);
            master->SetServerIsWaitingOnChunkLoading();
        }
        break;
    }
    case 7:
    {
        if (!IsPrimaryPlayer())
            break;
        GameModePacket::RoundStartGameModeData data;
        packet->ReadPacket(data);
        break;
    }
    case 9:
    {
        GameModePacket::SyncTeamInviteData data;
        packet->ReadPacket(data);
        boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
        if (player != 0 && player->GetLocalGameMode() != 0)
            player->GetLocalGameMode()->SyncTeamInvite(data.invite);
        break;
    }
    case 10:
    {
        GameModePacket::TeamIDData data;
        packet->ReadPacket(data);
        boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
        if (player != 0 && player->GetLocalGameMode() != 0)
            player->GetLocalGameMode()->RemoveTeamInvite(static_cast<unsigned int>(data.teamId));
        break;
    }
    case 0xd:
    {
        GameModePacket::AddAreaBoundsData data;
        packet->ReadPacket(data);
        MultiPlayerGameMode* mode = m_minecraft->mGameModes[m_playerId];
        if (mode != 0 && mode->getGameModeConstraints() != 0)
        {
            AreaConstraint* constraint = new AreaConstraint(
                0, static_cast<double>(data.minX), static_cast<double>(data.minY),
                static_cast<double>(data.minZ), static_cast<double>(data.maxX),
                static_cast<double>(data.maxY), static_cast<double>(data.maxZ),
                true, true, true, 0);
            mode->getGameModeConstraints()->AddConstraint(constraint, true);
        }
        break;
    }
    case 0xe:
    {
        GameModePacket::AddAreaBoundsData data;
        packet->ReadPacket(data);
        MultiPlayerGameMode* mode = m_minecraft->mGameModes[m_playerId];
        if (mode != 0 && mode->getGameModeConstraints() != 0)
        {
            mode->getGameModeConstraints()->ClearAllAreaConstraints();
            AreaConstraint* constraint = new AreaConstraint(
                0, static_cast<double>(data.minX), static_cast<double>(data.minY),
                static_cast<double>(data.minZ), static_cast<double>(data.maxX),
                static_cast<double>(data.maxY), static_cast<double>(data.maxZ),
                false, true, true, 0);
            mode->getGameModeConstraints()->AddConstraint(constraint, true);
        }
        break;
    }
    case 0xf:
    {
        MultiPlayerGameMode* mode = m_minecraft->mGameModes[m_playerId];
        if (mode != 0 && mode->getGameModeConstraints() != 0)
            mode->getGameModeConstraints()->ClearAllAreaConstraints();
        break;
    }
    case 0x11:
    {
        if (!IsPrimaryPlayer() || CGameNetworkManager::sInstance.IsHost())
            break;
        GameModePacket::ChangeCustomGameMode data;
        packet->ReadPacket(data);
        if (master != 0)
        {
            unsigned int previousUniqueId = master->GetCurrentUniqueId();
            master->SetCurrentUniqueId(data.customGameMode);
            m_minecraft->SetupMiniGameInstance(
                *MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(data.gameMode), true), 0);

            if (previousUniqueId != static_cast<unsigned int>(data.customGameMode))
            {
                if (data.customGameMode == -1)
                {
                    std::vector<LevelGenerationOptions*>* generators =
                        gApp.getLevelGenerators()->getLevelGenerators();
                    for (std::vector<LevelGenerationOptions*>::iterator it = generators->begin();
                         it != generators->end(); ++it)
                    {
                        LevelGenerationOptions* options = *it;
                        if (options->isGameModeLobby())
                        {
                            std::wstring savePath = options->getBaseSaveRoot();
                            savePath += options->getBaseSavePath();
                            options->setBaseSaveData(File(savePath));
                            gApp.setLevelGenerationOptions(options);
                            break;
                        }
                    }

                    MultiPlayerLevel* level = m_minecraft->getLevel(0);
                    level->BlockUntilSafeToChangeSaveData(true);
                    level->refreshFromLevelGen();
                    gRegionFileCache._clear();
                    gApp.setLevelGenerationOptions(NULL);
                }
                else
                {
                    std::vector<LevelRuleset*> rules;
                    gApp.getLevelRules()->getRulesByType(
                        static_cast<LevelRules::ELevelRuleType>(data.gameMode), &rules, false);
                    for (std::vector<LevelRuleset*>::iterator it = rules.begin();
                         it != rules.end(); ++it)
                    {
                        LevelGenerationOptions* options = (*it)->getGenerationOptions();
                        if (options->getUniqueId() == static_cast<unsigned int>(data.customGameMode))
                        {
                            m_minecraft->getLevel(0)->BlockUntilSafeToChangeSaveData(true);
                            gApp.setLevelGenerationOptions(options);
                            master->SetNextLevelGenerationOptions(options, true);
                            break;
                        }
                    }
                }

                if (m_minecraft->IsJoiningFromFriendsTabOrInvite())
                {
                    CMinecraftApp::SetupMiniGameMusic();
                    m_minecraft->SetJoiningFromFriendsTabOrInvite(false);
                }
            }
            master->onChangeCustomGameMode();
        }
        break;
    }
    case 0x12:
    {
        GameModePacket::ChangeCustomGameMode data;
        packet->ReadPacket(data);
        if (IsPrimaryPlayer() && master != 0)
        {
            std::vector<LevelRuleset*> rules;
            gApp.getLevelRules()->getRulesByType(
                static_cast<LevelRules::ELevelRuleType>(data.gameMode), &rules, false);
            for (std::vector<LevelRuleset*>::iterator it = rules.begin(); it != rules.end(); ++it)
            {
                LevelGenerationOptions* options = (*it)->getGenerationOptions();
                if (options->getUniqueId() == static_cast<unsigned int>(data.customGameMode))
                {
                    master->SetNextLevelGenerationOptions(options, false);
                    break;
                }
            }
        }
        break;
    }
    case 0x13:
    {
        GameModePacket::LockPlayerLocation data;
        packet->ReadPacket(data);
        boost::shared_ptr<MultiplayerLocalPlayer>& player = m_minecraft->mLocalPlayers[m_playerId];
        if (player != 0)
            player->SetPositionLocked(data.locked);
        break;
    }
    case 0x14:
    {
        GameModePacket::ChestRefill data;
        packet->ReadPacket(data);
        const bool splitScreen = gApp.GetLocalPlayerCount() > 1;
        gConsoleUIController.ShowChestRefillAnimation(splitScreen);
        for (size_t i = 0; i < data.positions.size(); ++i)
        {
            BlockEntity* blockEntity =
                m_level != 0 ? m_level->getBlockEntity(data.positions[i]).get() : 0;
            ChestBlockEntity* chest = dynamic_cast<ChestBlockEntity*>(blockEntity);
            if (chest != 0)
                chest->startRefilledAnimation();
        }
        updateTutorialOnGameMode(static_cast<eTutorial_GameMode>(4));
        if (master != 0)
            master->OnChestRefill();
        break;
    }
    case 0x15:
    {
        GameModePacket::TutorialGameModeData data;
        packet->ReadPacket(data);
        updateTutorialOnGameMode(static_cast<eTutorial_GameMode>(data.tutorial));
        break;
    }
    case 0x16:
    {
        if (!IsPrimaryPlayer())
            break;
        if (master != 0)
            master->OnShowdown();
        if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
            gConsoleUIController.ShowTimedSplash(
                m_playerId, L10N::GetString(static_cast<int>(0x3143fcab)), 3000);
        break;
    }
    case 0x17:
    {
        GameModePacket::StateChangeData data;
        packet->ReadPacket(data);
        if (IsPrimaryPlayer() && master != 0)
            master->OnStateChange(data.state);
        break;
    }
    case 0x18:
    {
        if (!IsPrimaryPlayer())
            break;
        GameModePacket::SoundPositionData data;
        packet->ReadPacket(data);
        if (master != 0)
        {
            Vec3* pos = Vec3::newTemp(data.x, data.y, data.z);
            master->OnDeath(pos);
        }
        break;
    }
    case 0x19:
    {
        if (!IsPrimaryPlayer())
            break;
        GameModePacket::UpdateSeedGameModeData data;
        packet->ReadPacket(data);
        if (master != 0)
            master->SetGeneratorSeed(data.getSeed(), static_cast<float>(data.getMapSizeMultiplier()));
        break;
    }
    case 0x1a:
    {
        if (!IsPrimaryPlayer())
            break;
        GameModePacket::UpdateThemeGameModeData data;
        packet->ReadPacket(data);
        if (master != 0)
            master->SetCurrentTheme(data.getThemeData(), 0);
        break;
    }
    case 0x1b:
    {
        if (!IsPrimaryPlayer())
            break;
        GameModePacket::UpdateThemeIdData data;
        packet->ReadPacket(data);
        if (master != 0)
            master->SetCurrentTheme(data.themeId, data.previousThemeId);
        break;
    }
    case 0x1c:
    {
        if (!IsPrimaryPlayer())
            break;
        GameModePacket::UpdateStartTimeData data;
        packet->ReadPacket(data);
        if (master != 0)
            master->SetLocalRoundTimeStart(data.startTime);
        break;
    }
    default:
        break;
    }
}

void ClientPacketListener::handleMapSelectInfo(boost::shared_ptr<MapSelectInfoPacket> packet)
{
    if (packet == 0)
        return;
    ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
    if (gameMode == 0)
        return;
    gameMode->ClearPlaylist();
    arrayWithLength<unsigned char> data = packet->getData();
    if (data.data == 0)
        return;
    ByteArrayInputStream bytes(data);
    DataInputStream input(&bytes);
    const int count = input.readInt();
    const int miniGameType = gApp.GetFirstMiniGameType();
    if (miniGameType == 1 || miniGameType == 3)
    {
        for (int i = 0; i < count; ++i)
        {
            const int uniqueId = input.readInt();
            const std::wstring name = input.readUTF();
            const int mapSize = input.readInt();
            gameMode->AddMapToPlaylist(uniqueId, name, mapSize);
        }
    }
    else if (miniGameType == 4)
    {
        std::vector<int> layerIds;
        for (int i = 0; i < count; ++i)
            layerIds.push_back(input.readInt());
        gameMode->clearBuildOffVotes();
        gameMode->mLayerIds = layerIds;
        gameMode->SetCurrentUniqueId(-1);
    }
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

void ClientPacketListener::staticCtor()
{
    InitializeCriticalSection(&s_deferredLinkMutex);
}


ClientPacketListener::DeferredEntityLinkPacket::DeferredEntityLinkPacket(
    boost::shared_ptr<ClientboundSetEntityLinkPacket> packet)
    : mTick(GetTickCount()), mPacket(packet)
{
}
