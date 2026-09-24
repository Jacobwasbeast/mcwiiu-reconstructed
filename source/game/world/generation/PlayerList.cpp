#include "game/world/generation/PlayerList.hpp"

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/unordered_set.hpp>
#include <cmath>
#include <limits>
#include <set>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/Settings.hpp"
#include "game/core/Socket.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/PortalForcer.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/world/generation/PlayerIO.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Items.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/ClientboundLoginPacket.hpp"
#include "game/network/TexturePacket.hpp"
#include "game/network/TextureAndGeometryPacket.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/ClientboundPlayerAbilitiesPacket.hpp"
#include "game/network/ClientboundRespawnPacket.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/network/ClientboundSetCarriedItemPacket.hpp"
#include "game/network/ClientboundSetPlayerTeamPacket.hpp"
#include "game/network/ClientboundSetSpawnPositionPacket.hpp"
#include "game/network/ClientboundSetTimePacket.hpp"
#include "game/network/ClientboundUpdateMobEffectPacket.hpp"
#include "game/network/PlayerInfoPacket.hpp"
#include "game/network/ProgressListener.hpp"
#include "game/network/ServerConnection.hpp"
#include "game/network/ServerConnectionListener.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include "game/server/ServerScoreboard.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/PlayerChunkMap.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/LevelSettings.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/Level.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/MinecraftServer.hpp"

class MobEffectInstance;

namespace
{
double snapHorizontal(double value)
{
    const double margin = 0.5;
    if (value >= 0.0)
        return floor(value + margin);
    return ceil(value - margin);
}

double snapVertical(double value, bool hasCeiling)
{
    return hasCeiling ? floor(value) : ceil(value);
}

bool hasSpawnCollisions(Level* level, boost::shared_ptr<ServerPlayer> player)
{
    if (!level || !player || !player->mBoundingBox)
        return false;
    std::vector<AABB*>* boxes =
        level->getCollisionAABBs(player, player->mBoundingBox, false, false, false);
    return boxes != NULL && !boxes->empty();
}

void resolveSpawnCollisions(Level* level, boost::shared_ptr<ServerPlayer> player)
{
    if (!level || !player)
        return;
    const double maxY = 512.0;
    const double collisionHeight = 1.0;
    const double x = player->mX;
    const double z = player->mZ;
    while (hasSpawnCollisions(level, player) && player->mY < maxY)
    {
        player->moveTo(x, player->mY + collisionHeight, z,
                       player->mYRot, player->mXRot);
    }
}

void relocateInvalidDimensionSpawn(PlayerList* playerList, boost::shared_ptr<ServerPlayer> player,
                                   bool forced)
{
    if (!playerList || !player || !playerList->getServer())
        return;
    ServerLevel* overworld = playerList->getServer()->getLevel(0);
    if (!overworld)
        return;
    if (player->mGameMode)
        player->mGameMode->setLevel(overworld);
    player->mDimension = 0;
    BlockPos spawn = overworld->getSharedSpawnPos();
    player->moveTo(spawn.x + 0.5, spawn.y, spawn.z + 0.5, player->mYRot, player->mXRot);
    BlockPos* respawnPos = player->getRespawnPosition();
    if (respawnPos != NULL)
    {
        BlockPos* validPos =
            Player::checkBedValidRespawnPosition(overworld, *respawnPos, forced);
        if (validPos != NULL)
        {
            player->moveTo(validPos->x + 0.5, validPos->y, validPos->z + 0.5,
                           player->mYRot, player->mXRot);
            player->setRespawnPosition(validPos, forced);
            delete validPos;
        }
    }
    resolveSpawnCollisions(overworld, player);
}
}

PlayerList::PlayerList(MinecraftServer* server, MasterGameMode* gameMode)
    : mServer(server), mGameMode(gameMode), mMaxPlayers(0), mViewDistance(10),
      mMaxRange(0), mAllowCheatsForAllPlayers(false), mOverrideGameMode(NULL),
      mUnknownState(0)
{
    mMaxPlayers = server->mSettings->getInt(static_cast<EServerSetting>(10), 20);
}

PlayerList::~PlayerList()
{
    mPlayers.clear();
    mPendingPlayers.clear();
}

bool PlayerList::canReceiveAllPackets(const boost::shared_ptr<ServerPlayer>& player)
{
    if (!player)
        return false;
    int group = 0;
    if (player->mDimension == -1)
        group = 1;
    else if (player->mDimension == 1)
        group = 2;
    std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it;
    for (it = mReceivingPlayers[group].begin(); it != mReceivingPlayers[group].end(); ++it)
        if (*it == player)
            return true;
    return false;
}

bool PlayerList::isOp(boost::shared_ptr<ServerPlayer> player)
{
    if (!player || !g_pApp || g_pApp->GetGameHostOption(eGameHostOption_15) == 0)
        return false;
    INetworkPlayer* networkPlayer = player->mConnection != NULL
        ? player->mConnection->getNetworkPlayer() : NULL;
    return player->isModerator() ||
           (networkPlayer != NULL && networkPlayer->IsHost());
}

int PlayerList::getViewDistance()
{
    return mViewDistance;
}

boost::shared_ptr<ServerPlayer> PlayerList::findAlivePlayerOnSystem(boost::shared_ptr<ServerPlayer> player)
{
    if (!player || !player->mConnection || player->mConnection->getNetworkPlayer() == NULL)
        return boost::shared_ptr<ServerPlayer>();
    INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it;
    for (it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        if (*it == player || !*it || !(*it)->isAlive())
            continue;
        if ((*it)->mDimension == player->mDimension && (*it)->mConnection &&
            (*it)->mConnection->getNetworkPlayer() != NULL &&
            networkPlayer->IsSameSystem((*it)->mConnection->getNetworkPlayer()))
            return *it;
    }
    return boost::shared_ptr<ServerPlayer>();
}

int PlayerList::removePlayerFromReceiving(boost::shared_ptr<ServerPlayer> player, bool force, int mode)
{
    int first = mode < 0 ? 0 : mode;
    int last = mode < 0 ? 3 : mode + 1;
    int removed = 0;
    for (int group = first; group < last && group < 3; ++group)
    {
        std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mReceivingPlayers[group].begin();
        while (it != mReceivingPlayers[group].end())
        {
            if (*it == player)
            {
                it = mReceivingPlayers[group].erase(it);
                ++removed;
            }
            else
                ++it;
        }
    }
    return force ? removed : (removed != 0);
}

int PlayerList::addPlayerToReceiving(boost::shared_ptr<ServerPlayer> player, bool force)
{
    int group = 0;
    if (player->mDimension == -1)
        group = 1;
    else if (player->mDimension == 1)
        group = 2;

    if (player->mConnection->getNetworkPlayer() == NULL)
        return 0;

    if (!force && canReceiveAllPackets(player))
        return 0;

    mReceivingPlayers[group].push_back(player);
    return 1;
}

void PlayerList::broadcastAll(boost::shared_ptr<Packet> packet)
{
    if (!packet)
        return;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        BOOST_ASSERT(*it);
        if (*it && (*it)->mConnection)
            (*it)->mConnection->send(packet);
    }
}

void PlayerList::save(boost::shared_ptr<ServerPlayer> player)
{
    if (!player || !mLevelStorage)
        return;
    PlayerIO* playerIO = mLevelStorage->getPlayerIO();
    if (playerIO)
        playerIO->save(boost::static_pointer_cast<Player>(player));
}

void PlayerList::saveAll(ProgressListener* listener, bool)
{
    if (listener != NULL)
        listener->progressStart(0x195381d);

    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it;
    for (it = mPlayers.begin(); it != mPlayers.end(); ++it)
        save(*it);

    if (mLevelStorage)
    {
        PlayerIO* playerIO = mLevelStorage->getPlayerIO();
        if (playerIO)
        {
            playerIO->saveMapIdLookup();
            playerIO->saveAllCachedData();
            playerIO->clearOldPlayerFiles();
        }
    }
}

void PlayerList::remove(boost::shared_ptr<ServerPlayer> player)
{
    if (mServer->mMasterGameMode)
        mServer->mMasterGameMode->OnLeave(player);
    else
        save(player);

    ServerLevel* level = player->getLevel();
    if (player->isPassenger())
    {
        boost::shared_ptr<Entity> rootVehicle = player->getRootVehicle();
        if (rootVehicle)
        {
            std::set<boost::shared_ptr<Entity> >* players =
                rootVehicle->getIndirectPassengers(0xC800001ULL);
            const bool onlyPassenger = players->size() == 1;
            delete players;

            if (onlyPassenger)
            {
                player->stopRiding();
                level->removeEntityImmediately(rootVehicle);
                std::set<boost::shared_ptr<Entity> >* passengers =
                    rootVehicle->getIndirectPassengers();
                for (std::set<boost::shared_ptr<Entity> >::const_iterator it =
                         passengers->begin(); it != passengers->end(); ++it)
                    level->removeEntityImmediately(*it);
                delete passengers;
                level->getChunk(player->mChunkX, player->mChunkZ)->markUnsaved();
            }
        }
    }

    level->removeEntity(player);
    level->getChunkMap()->remove(player);
    level->getChunkMap()->onPlayerLeave(player);

    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        if (*it == player)
            it = mPlayers.erase(it);
        else
            ++it;
    }

    removePlayerFromReceiving(player, true, 0);
    player->mConnection = boost::shared_ptr<ServerGamePacketListenerImpl>();
    saveAll(NULL, false);
}

void PlayerList::removeAll()
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        if (*it && (*it)->mConnection)
            (*it)->mConnection->disconnect(DisconnectPacket::eDisconnectReason_ConnectionClosed);
    }
    mPlayers.clear();
    for (int group = 0; group < 3; ++group)
        mReceivingPlayers[group].clear();
}

CompoundTag* PlayerList::load(boost::shared_ptr<ServerPlayer> player)
{
    if (!player || !mLevelStorage)
        return 0;
    PlayerIO* playerIO = mLevelStorage->getPlayerIO();
    if (!playerIO)
        return 0;
    CompoundTag* tag = playerIO->load(boost::static_pointer_cast<Player>(player));
    return tag;
}

void PlayerList::validatePlayerSpawnPosition(boost::shared_ptr<ServerPlayer> player)
{
    BOOST_ASSERT(player);
    if (!player || !mServer)
        return;
    const bool forced = player->isRespawnForced() != 0;
    ServerLevel* level = mServer->getLevel(player->mDimension);
    if (!level || !level->mDimension)
        return;
    const bool hasCeiling = level->mDimension->isHasCeiling() != 0;
    const double x = snapHorizontal(player->mX);
    const double y = snapVertical(player->mY, hasCeiling);
    const double z = snapHorizontal(player->mZ);
    const bool collidedAtStart = hasSpawnCollisions(level, player);
    const double originalX = player->mX;
    const double originalY = player->mY;
    const double originalZ = player->mZ;
    player->moveTo(x, y, z, player->mYRot, player->mXRot);
    if (collidedAtStart)
        resolveSpawnCollisions(level, player);
    else if (hasSpawnCollisions(level, player))
        player->moveTo(originalX, originalY, originalZ, player->mYRot, player->mXRot);
    const int dimensionId = level->mDimension->getType()->getId();
    const int netherId = DimensionType::NETHER->getId();
    const int endId = DimensionType::THE_END->getId();
    if ((dimensionId == netherId && player->mY > 125.0)
        || (dimensionId == endId && player->mY == std::numeric_limits<double>::max()))
        relocateInvalidDimensionSpawn(this, player, forced);
}

void PlayerList::updatePlayerGameMode(boost::shared_ptr<ServerPlayer> oldPlayer,
                                      boost::shared_ptr<ServerPlayer> newPlayer, Level* level)
{
    if (!oldPlayer || !oldPlayer->mGameMode)
        return;
    if (newPlayer && newPlayer->mGameMode)
    {
        oldPlayer->mGameMode->SetGameType(newPlayer->mGameMode->GetGameType());
        return;
    }
    if (mOverrideGameMode)
    {
        oldPlayer->mGameMode->SetGameType(mOverrideGameMode);
        return;
    }
    if (level && level->getLevelData())
        oldPlayer->mGameMode->updateGameMode(level->getLevelData()->getGameType());
}

int PlayerList::getMaxPlayers()
{
    return mMaxPlayers;
}

MinecraftServer* PlayerList::getServer()
{
    return mServer;
}

void PlayerList::updateEntireScoreboard(ServerScoreboard* scoreboard,
                                        boost::shared_ptr<ServerPlayer> player)
{
    if (!scoreboard || !player || !player->mConnection)
        return;
    std::vector<PlayerTeam*>* teams = scoreboard->getPlayerTeams();
    if (!teams)
        return;
    for (std::vector<PlayerTeam*>::iterator it = teams->begin(); it != teams->end(); ++it)
    {
        if (!*it)
            continue;
        player->mConnection->send(boost::shared_ptr<Packet>(
            new ClientboundSetPlayerTeamPacket(*it, 0)));
    }
}

void PlayerList::sendLevelInfo(boost::shared_ptr<ServerPlayer> player, ServerLevel* level)
{
    const long long gameTime = level->getGameTime();
    const long long dayTime = level->getDayTime();
    GameRules* gameRules = level->getGameRules();
    const bool daylightCycle = gameRules->getBoolean(8);
    player->mConnection->send(boost::shared_ptr<Packet>(
        new ClientboundSetTimePacket(gameTime, dayTime, daylightCycle)));
    if (level->isRaining())
    {
        player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundGameEventPacket(1, 0.0f)));
    }
    else
    {
        player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundGameEventPacket(2, 0.0f)));
    }
    player->mConnection->send(boost::shared_ptr<Packet>(
        new ClientboundSetSpawnPositionPacket(level->getSharedSpawnPos())));
}

void PlayerList::changeDimension(boost::shared_ptr<ServerPlayer> player, ServerLevel* level)
{
    BOOST_ASSERT(player);
    ServerLevel* newLevel = player->getLevel();
    if (level != NULL)
        level->getChunkMap()->remove(player);
    newLevel->getChunkMap()->add(player);
    newLevel->getChunkSource()->create(static_cast<int>(player->mX) >> 4,
                                       static_cast<int>(player->mZ) >> 4);
}

void PlayerList::add(boost::shared_ptr<ServerPlayer> player)
{
    if (player->mConnection->getNetworkPlayer() != NULL)
        broadcastAll(boost::shared_ptr<Packet>(new PlayerInfoPacket(player)));

    mPlayers.push_back(player);
    addPlayerToReceiving(player, false);

    ServerLevel* level = mServer->getLevel(player->mDimension);
    changeDimension(player, level);
    level->addEntity(player);

    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it;
    for (it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        if ((*it)->mConnection->getNetworkPlayer() != NULL)
            player->mConnection->send(boost::shared_ptr<Packet>(new PlayerInfoPacket(*it)));
    }

    if (mGameMode)
        mGameMode->OnJoin(player);
}

void PlayerList::placeNewPlayer(Connection* connection, boost::shared_ptr<ServerPlayer> player,
                                boost::shared_ptr<ClientboundLoginPacket> loginPacket)
{
    CompoundTag* playerTag = NULL;
    if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(31)) == 0)
        playerTag = load(player);

    ServerLevel* level = mServer->getLevel(player->mDimension);
    player->setLevel(level);
    player->mGameMode->setLevel(level);

    INetworkPlayer* networkPlayer = connection->m_socket->getPlayer();
    if (networkPlayer != NULL && networkPlayer->IsHost())
    {
        player->enableAllPlayerPrivileges(true);
        player->setPlayerGamePrivilege(static_cast<Player::EPlayerGamePrivileges>(34), 1);
    }

    validatePlayerSpawnPosition(player);
    level = mServer->getLevel(player->mDimension);

    bool occupiedSlots[8] = {};
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
        occupiedSlots[(*it)->mPlayerIndex] = true;
    unsigned char playerSlot = 0;
    for (unsigned char slot = 0; slot < 8; ++slot)
    {
        if (!occupiedSlots[slot])
        {
            playerSlot = slot;
            break;
        }
    }
    player->mPlayerIndex = playerSlot;
    player->setCustomSkin(loginPacket->mWorldHeight);
    player->setCustomCape(loginPacket->mSeaLevel);

    boost::shared_ptr<ServerGamePacketListenerImpl> connectionListener(
        new ServerGamePacketListenerImpl(mServer, connection, player));

    if (playerTag == NULL)
    {
        if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(31)) == 0)
        {
            int mapId = level->getAuxValueForMap(player->mXuid, 0, BlockPos(player), 3);
            player->mInventory->setItem(9, not_null_ptr<ItemInstance>(
                new ItemInstance(Items::map, 1, mapId)));
        }
        if (g_pApp->getGameRuleDefinitions() != NULL)
            g_pApp->getGameRuleDefinitions()->postProcessPlayer(player);
    }

    if (!player->mSkinLocation.empty() && player->mSkinLocation.compare(0, 3, L"def") != 0 &&
        !g_pApp->IsFileInMemoryTextures(player->mSkinLocation))
    {
        if (mServer->getConnection()->addPendingTextureRequest(player->mSkinLocation))
            connectionListener->send(boost::shared_ptr<Packet>(
                new TextureAndGeometryPacket(player->mSkinLocation, NULL, 0)));
    }
    else if (!player->mSkinLocation.empty() && g_pApp->IsFileInMemoryTextures(player->mSkinLocation))
        g_pApp->AddMemoryTextureFile(player->mSkinLocation, NULL, 0);

    if (!player->mCapeLocation.empty() && player->mCapeLocation.compare(0, 3, L"def") != 0 &&
        !g_pApp->IsFileInMemoryTextures(player->mCapeLocation))
    {
        if (mServer->getConnection()->addPendingTextureRequest(player->mCapeLocation))
            connectionListener->send(boost::shared_ptr<Packet>(
                new TexturePacket(player->mCapeLocation, NULL, 0)));
    }

    player->mIsGuest = loginPacket->mAllowCommands;

    if (!mGameMode)
        updatePlayerGameMode(player, boost::shared_ptr<ServerPlayer>(), level);
    else
        player->mGameMode->updateGameMode(mGameMode->InitialGameType(player));
    player->UpdateSize();

    unsigned int privileges = player->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All);
    const GameType* privilegeGameType = LevelSettings::validateGameType(
        (Player::getPlayerGamePrivilege(privileges, Player::ePlayerGamePrivilege_9) ?
            GameType::CREATIVE : GameType::SURVIVAL)->getId());
    if (player->mGameMode->isAdventure())
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_9, 0);
    else if (player->mGameMode->GetGameType() != privilegeGameType)
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_9,
                                       player->mGameMode->GetGameType()->getId());

    player->mConnection = connectionListener;
    connectionListener->mFriendsOnly = loginPacket->mHardcore;
    connectionListener->mPlayerUID = loginPacket->mPlayerUID;
    connectionListener->mOldPlayerUID = loginPacket->mPlayerUID2;
    if (loginPacket->mHardcore)
        ++mServer->statusMessageCapacity;
    addPlayerToReceiving(player, false);

    LevelData* levelData = level->getLevelData();
    connectionListener->send(boost::shared_ptr<Packet>(new ClientboundLoginPacket(
        L"", player->getId(), levelData->getGeneratorType(), level->getSeed(),
        player->mGameMode->GetGameType()->getId(),
        g_pApp->GetGameHostOption(static_cast<eGameHostOption>(31)),
        g_pApp->GetFirstMiniGameType(), static_cast<char>(level->mDimension->getType()->getId()),
        static_cast<unsigned char>(level->getMaxBuildHeight()),
        static_cast<unsigned char>(getMaxPlayers()), static_cast<char>(level->getDifficulty()->getId()),
        telemetryManagerInstance->GetMultiplayerInstanceID(), playerSlot, level->useNewSeaLevel(),
        player->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All), levelData->getXZSize(),
        levelData->getHellScale(), levelData->getCloudHeight(), levelData->getModernEnd(),
        levelData->getBiomeScale(), levelData->getBiomeCentreXChunk(), levelData->getBiomeCentreZChunk(),
        level->useNewSeaLevel() ? level->getSeaLevel() : -1)));

    connectionListener->send(boost::shared_ptr<Packet>(
        new ClientboundPlayerAbilitiesPacket(&player->mAbilities, player->getId())));
    connectionListener->send(boost::shared_ptr<Packet>(
        new ClientboundSetCarriedItemPacket(player->mInventory->m_selected)));

    updateEntireScoreboard(static_cast<ServerScoreboard*>(level->GetScoreboard()), player);
    sendLevelInfo(player, level);

    broadcastAll(boost::shared_ptr<Packet>(new ClientboundChatPacket(
        player->getName(), static_cast<ClientboundChatPacket::EChatPacketMessage>(9), -1)));

    add(player);
    player->doTick(true, true, true);

    connectionListener->teleport(player->mX, player->mY, player->mZ,
                                 player->mYRot, player->mXRot, true, true);
    mServer->getConnection()->addServerGamePacketListenerImpl(connectionListener);

    connectionListener->send(boost::shared_ptr<Packet>(new ClientboundSetTimePacket(
        level->getGameTime(), level->getDayTime(), level->getGameRules()->getBoolean(8))));

    std::vector<MobEffectInstance*>* effects = player->getActiveEffects();
    if (effects != NULL)
    {
        std::vector<MobEffectInstance*>::iterator it;
        for (it = effects->begin(); it != effects->end(); ++it)
            connectionListener->send(boost::shared_ptr<Packet>(
                new ClientboundUpdateMobEffectPacket(player->getId(), *it)));
        delete effects;
    }

    player->initMenu();
    if (playerTag != NULL && playerTag->contains(L"RootVehicle", 10))
    {
        CompoundTag* rootVehicle = playerTag->getCompound(L"RootVehicle");
        boost::shared_ptr<Entity> vehicle = EntityIO::newEntity(rootVehicle->getCompound(L"Entity"), level);
        if (vehicle)
        {
            if (vehicle->mDimension != player->mDimension)
            {
                vehicle->setLevel(player->mLevel);
                vehicle->mDimension = player->mDimension;
                vehicle->setPos(player->mX, player->mY, player->mZ);
            }
            vehicle->mForcedLoading = true;
            bool added = level->addEntity(vehicle);
            vehicle->mForcedLoading = false;
            if (added)
            {
                const std::wstring attach = rootVehicle->getString(L"Attach");
                if (vehicle->getUUID() == attach)
                    player->startRiding(vehicle);
                else
                {
                    std::set<boost::shared_ptr<Entity> >* passengers = vehicle->getIndirectPassengers();
                    for (std::set<boost::shared_ptr<Entity> >::const_iterator it = passengers->begin();
                         it != passengers->end(); ++it)
                    {
                        if ((*it)->getUUID() == attach)
                            player->startRiding(*it);
                    }
                    delete passengers;
                }
                if (!player->isPassenger())
                {
                    level->removeEntityImmediately(vehicle);
                    std::set<boost::shared_ptr<Entity> >* passengers = vehicle->getIndirectPassengers();
                    for (std::set<boost::shared_ptr<Entity> >::const_iterator it = passengers->begin();
                         it != passengers->end(); ++it)
                        level->removeEntityImmediately(*it);
                    delete passengers;
                }
            }
        }
    }
    delete playerTag;

    networkPlayer = player->mConnection->getNetworkPlayer();
    if (networkPlayer != NULL)
    {
        for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = mPlayers.begin();
             it != mPlayers.end(); ++it)
        {
            INetworkPlayer* other = (*it)->mConnection->getNetworkPlayer();
            if (other != networkPlayer && other != NULL && networkPlayer->IsSameSystem(other) &&
                (*it)->mRespawnState)
            {
                player->mRespawnState = true;
                break;
            }
        }
    }
}

void PlayerList::setLevel(arrayWithLength<ServerLevel*> levels)
{
    if (levels.data != 0 && levels.length != 0 && levels.data[0] != 0)
        mLevelStorage = levels.data[0]->getLevelStorage();
}

int PlayerList::getMaxRange()
{
    return PlayerChunkMap::convertChunkRangeToBlock(getViewDistance());
}

void PlayerList::move(boost::shared_ptr<ServerPlayer> player)
{
    if (!player)
        return;
    ServerLevel* level = player->getLevel();
    if (level && level->getChunkMap())
        level->getChunkMap()->move(player);
}

boost::shared_ptr<ServerPlayer> PlayerList::getPlayerForLogin(ServerConnectionListener* listener,
                                                              const std::wstring& name,
                                                              PlayerUID uid, PlayerUID oldUid)
{
    if (getMaxPlayers() <= static_cast<int>(mPlayers.size()))
    {
        listener->disconnect(DisconnectPacket::eDisconnectReason_GameFull);
        return boost::shared_ptr<ServerPlayer>();
    }
    ServerPlayerGameMode* gameMode = new ServerPlayerGameMode(mServer->getLevel(0), mGameMode);
    boost::shared_ptr<ServerPlayer> newPlayer(
        new ServerPlayer(mServer, mServer->getLevel(0), name, gameMode));
    newPlayer->setXuid(uid);
    newPlayer->m_0x798 = oldUid;
    return newPlayer;
}

std::vector<boost::shared_ptr<ServerPlayer> >* PlayerList::getPlayers()
{
    return &mPlayers;
}

boost::shared_ptr<ServerPlayer> PlayerList::respawn(boost::shared_ptr<ServerPlayer> player,
                                                    int dimension, bool conqueredEnd)
{
    BOOST_ASSERT(player);
    if (!player || !mServer)
        return boost::shared_ptr<ServerPlayer>();

    const int oldDimension = player->mDimension;
    ServerLevel* oldLevel = mServer->getLevel(oldDimension);
    ServerLevel* newLevel = mServer->getLevel(dimension);
    if (!newLevel)
        return player;

    const bool receivingAllPackets = canReceiveAllPackets(player);
    if (oldLevel && oldLevel->getTracker())
    {
        if (receivingAllPackets)
            oldLevel->getTracker()->clear(player);
        else
            oldLevel->getTracker()->removeEntity(player);
    }
    removePlayerFromReceiving(player, true, -1);
    if (oldLevel)
        oldLevel->removeEntityImmediately(player);

    ServerPlayerGameMode* gameMode = new ServerPlayerGameMode(newLevel, mGameMode);
    if (player->mGameMode)
        gameMode->RestoreFromOld(player->mGameMode);
    boost::shared_ptr<ServerPlayer> replacement(
        new ServerPlayer(mServer, newLevel, player->getName(), gameMode));
    replacement->mDimension = dimension;
    replacement->setXuid(player->mXuid);
    replacement->m_0x798 = player->m_0x798;
    replacement->mSuid = player->mSuid;
    replacement->setMainArm(player->getMainArm());
    replacement->setPlayerGamePrivilege(
        Player::ePlayerGamePrivilege_All,
        player->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All));
    replacement->mConnection = player->mConnection;
    replacement->mGameMode->setLevel(newLevel);

    BlockPos spawn = newLevel->getSharedSpawnPos();
    if (!conqueredEnd)
    {
        BlockPos* respawnPos = player->getRespawnPosition();
        if (respawnPos != NULL)
        {
            BlockPos* validPos = Player::checkBedValidRespawnPosition(
                newLevel, *respawnPos, player->isRespawnForced() != 0);
            if (validPos != NULL)
            {
                spawn = *validPos;
                delete validPos;
            }
        }
    }
    replacement->moveTo(spawn.x + 0.5, spawn.y, spawn.z + 0.5,
                        player->mYRot, player->mXRot);

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        if (*it == player)
        {
            *it = replacement;
            break;
        }
    }
    newLevel->addEntity(replacement);
    addPlayerToReceiving(replacement, false);
    newLevel->getChunkMap()->add(replacement);
    newLevel->getChunkSource()->create(static_cast<int>(replacement->mX) >> 4,
                                       static_cast<int>(replacement->mZ) >> 4);

    if (replacement->mConnection)
    {
        const GameType* gameType = replacement->mGameMode->GetGameType();
        const Difficulty* difficulty = newLevel->getDifficulty();
        LevelData* levelData = newLevel->getLevelData();
        LevelType* levelType = levelData ? levelData->getGeneratorType() : NULL;
        const bool keepData = newLevel->useNewSeaLevel();
        const int seaLevel = keepData ? newLevel->getSeaLevel() : -1;
        replacement->mConnection->send(boost::shared_ptr<Packet>(
            new ClientboundRespawnPacket(replacement->mX, replacement->mY,
                replacement->mZ, replacement->mYRot, replacement->mXRot,
                static_cast<char>(dimension), newLevel->getSeed(), gameType, difficulty,
                levelType, keepData, replacement->getId(), gameType ? gameType->getId() : 0,
                difficulty ? difficulty->getId() : 0, dimension,
                levelData != NULL && levelData->isHardcore(), 0, mViewDistance,
                mViewDistance, true, seaLevel)));
        replacement->mConnection->teleport(replacement->mX, replacement->mY,
                                            replacement->mZ, replacement->mYRot,
                                            replacement->mXRot, true, true);
    }
    replacement->markAttributesDirty();
    sendLevelInfo(replacement, newLevel);
    sendAllPlayerInfo(replacement);
    if (mServer->mMasterGameMode)
    {
        mServer->mMasterGameMode->OnSpawn(replacement, player, false);
        mServer->mMasterGameMode->OnSpawned(replacement, false);
    }
    std::vector<MobEffectInstance*>* effects = replacement->getActiveEffects();
    if (replacement->mConnection && effects)
    {
        for (std::vector<MobEffectInstance*>::iterator it = effects->begin();
             it != effects->end(); ++it)
            if (*it)
                replacement->mConnection->send(boost::shared_ptr<Packet>(
                    new ClientboundUpdateMobEffectPacket(replacement->getId(), *it)));
    }
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
        if (*it && *it != replacement)
            (*it)->notifyPlayerRespawn(&player, &replacement);
    return replacement;
}

boost::shared_ptr<Entity> PlayerList::repositionAcrossDimension(boost::shared_ptr<Entity> entity,
                                                                int dimension, ServerLevel* oldLevel,
                                                                ServerLevel* newLevel)
{
    if (!entity || !oldLevel || !newLevel)
        return entity;

    double x = entity->mX;
    double z = entity->mZ;
    const int currentDimension = entity->mDimension;
    const double scale = static_cast<double>(newLevel->getLevelData()->getHellScale());
    if (currentDimension == -1)
    {
        x /= scale;
        z /= scale;
    }
    else if (currentDimension == 0)
    {
        x *= scale;
        z *= scale;
    }

    newLevel->clampWithinLevelBounds(x, z, 0.0);
    entity->moveTo(x, entity->mY, z, entity->mYRot, entity->mXRot);

    if (dimension == 1)
    {
        BlockPos spawn = newLevel->getSharedSpawnPos();
        entity->moveTo(spawn, entity->mYRot, entity->mXRot);
    }
    else if (dimension != -1 && dimension != 0)
    {
        BlockPos* spawn = newLevel->getDimensionSpecificSpawn();
        if (spawn != NULL)
            entity->moveTo(*spawn, entity->mYRot, entity->mXRot);
    }

    if (currentDimension == -1 || currentDimension == 0)
        newLevel->getPortalForcer()->force(entity, 0.0f,
                                            static_cast<int>(entity->mX) >> 4,
                                            static_cast<int>(entity->mZ) >> 4);

    newLevel->getChunkSource()->create(static_cast<int>(entity->mX) >> 4,
                                       static_cast<int>(entity->mZ) >> 4);

    boost::shared_ptr<ServerPlayer> serverPlayer =
        boost::dynamic_pointer_cast<ServerPlayer>(entity);
    if (serverPlayer)
    {
        removePlayerFromReceiving(serverPlayer, false, currentDimension);
        addPlayerToReceiving(serverPlayer, false);
    }
    else
    {
        oldLevel->removeEntityImmediately(entity);
        newLevel->addEntity(entity);
    }
    return entity;
}

void PlayerList::sendAllPlayerInfo(boost::shared_ptr<ServerPlayer> player)
{
    player->refreshContainer(player->mContainerMenu);
    player->resetSentInfo();
    player->mConnection->send(boost::shared_ptr<Packet>(
        new ClientboundSetCarriedItemPacket(player->mInventory->m_selected)));
}

void PlayerList::toggleDimension(boost::shared_ptr<ServerPlayer> player, int dimension)
{
    BOOST_ASSERT(player);
    ServerLevel* oldLevel = mServer->getLevel(player->mDimension);
    const int oldDimension = player->mDimension;
    const bool receivingAllPackets = canReceiveAllPackets(player);
    bool anotherLocalPlayerInDimension = true;
    INetworkPlayer* localNetworkPlayer =
        player->mConnection != NULL ? player->mConnection->getNetworkPlayer() : NULL;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> candidate = *it;
        if (!candidate || candidate.get() == player.get() || candidate->mDimension != oldDimension)
            continue;
        INetworkPlayer* candidateNetworkPlayer =
            candidate->mConnection != NULL ? candidate->mConnection->getNetworkPlayer() : NULL;
        if (candidateNetworkPlayer != NULL && localNetworkPlayer != NULL &&
            CGameNetworkManager::sInstance.SystemFlagGet(candidateNetworkPlayer, 0x24))
        {
            anotherLocalPlayerInDimension = false;
            break;
        }
    }

    if (oldLevel != NULL && oldLevel->getTracker() != NULL)
    {
        if (!receivingAllPackets)
            oldLevel->getTracker()->removeEntity(player);
        else if (anotherLocalPlayerInDimension)
            oldLevel->getTracker()->clear(player);
        else
            oldLevel->getTracker()->removeEntity(player);
    }

    if (oldDimension == 1 || dimension != 1)
    {
        if (oldDimension == 1)
            player->displayClientMessage(static_cast<int>(0x6e3a65e3), true);
    }
    else
    {
        player->displayClientMessage(static_cast<int>(0xd3560061), true);
    }

    player->mDimension = dimension;
    ServerLevel* newLevel = mServer->getLevel(player->mDimension);
    player->clearFire();
    player->flushEntitiesToRemove();

    LevelData* levelData = newLevel->getLevelData();
    const GameType* gameType = player->mGameMode != NULL
                                   ? player->mGameMode->GetGameType()
                                   : NULL;
    const Difficulty* difficulty = newLevel->getDifficulty();
    LevelType* levelType = levelData != NULL ? levelData->getGeneratorType() : NULL;
    const bool useNewSeaLevel = newLevel->useNewSeaLevel();
    const int seaLevel = useNewSeaLevel ? newLevel->getSeaLevel() : -1;
    const int gameMode = player->mGameMode != NULL && player->mGameMode->GetGameType() != NULL
                             ? player->mGameMode->GetGameType()->getId()
                             : 0;
    boost::shared_ptr<Packet> respawn(new ClientboundRespawnPacket(
        static_cast<char>(dimension), newLevel->getSeed(), gameType, difficulty, levelType,
        useNewSeaLevel, player->getId(), gameMode, difficulty != NULL ? difficulty->getId() : 0,
        dimension, levelData != NULL && levelData->isHardcore(), 0, mViewDistance,
        mViewDistance, true, seaLevel));
    if (player->mConnection != NULL)
        player->mConnection->send(respawn);

    oldLevel->removeEntityImmediately(player);
    repositionAcrossDimension(player, dimension, oldLevel, newLevel);
    changeDimension(player, newLevel);
    player->mGameMode->setLevel(newLevel);

    if (player->mConnection != NULL && newLevel != NULL &&
        newLevel->mDimension != NULL)
    {
        ChunkPos chunk(player->mChunkX, player->mChunkZ);
        const int flagIndex = ServerPlayer::getFlagIndexForChunk(
            &chunk, newLevel->mDimension->getType()->getId());
        const bool chunkAlreadyKnown = localNetworkPlayer != NULL &&
            CGameNetworkManager::sInstance.SystemFlagGet(localNetworkPlayer, flagIndex);
        if (!chunkAlreadyKnown)
        {
            player->mConnection->teleport(player->mX, player->mY, player->mZ,
                                          player->mYRot, player->mXRot, false, true);
            player->doTick(true, true, true);
        }
        player->mConnection->teleport(player->mX, player->mY, player->mZ,
                                      player->mYRot, player->mXRot, true, true);
    }

    std::vector<MobEffectInstance*>* effects = player->getActiveEffects();
    if (player->mConnection != NULL && effects != NULL)
    {
        for (std::vector<MobEffectInstance*>::iterator it = effects->begin();
             it != effects->end(); ++it)
        {
            if (*it != NULL)
                player->mConnection->send(boost::shared_ptr<Packet>(
                    new ClientboundUpdateMobEffectPacket(player->getId(), *it)));
        }
    }

    player->markAttributesDirty();
    sendLevelInfo(player, newLevel);
    sendAllPlayerInfo(player);
}

void PlayerList::tick()
{
    for (std::deque<unsigned char>::iterator id = mPlayersToKick.begin();
         id != mPlayersToKick.end(); ++id)
    {
        INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerBySmallId(*id);
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
             it != mPlayers.end(); ++it)
        {
            if (!*it || !(*it)->mConnection ||
                (*it)->mConnection->getNetworkPlayer() != networkPlayer)
                continue;
            boost::shared_ptr<ServerPlayer> replacement = findAlivePlayerOnSystem(*it);
            if (replacement)
                replacement->enableAllPlayerPrivileges(false);
            mBannedPlayers.push_back((*it)->mXuid);
            (*it)->mDisconnected = true;
            (*it)->mConnection->send(boost::shared_ptr<Packet>(
                new DisconnectPacket(DisconnectPacket::eDisconnectReason_Kicked)));
            (*it)->mConnection->disconnect(DisconnectPacket::eDisconnectReason_Kicked);
        }
    }
    mPlayersToKick.clear();

    for (std::deque<unsigned char>::iterator id = mListenersToClose.begin();
         id != mListenersToClose.end(); ++id)
    {
        INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerBySmallId(*id);
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
             it != mPlayers.end(); ++it)
        {
            if (*it && (*it)->mConnection &&
                (*it)->mConnection->getNetworkPlayer() == networkPlayer)
                (*it)->mConnection->disconnect(DisconnectPacket::eDisconnectReason_ConnectionClosed);
        }
    }
    mListenersToClose.clear();
}

bool PlayerList::isTrackingBlock(const BlockPos& pos, const Dimension* dimension)
{
    if (!mServer || !dimension)
        return false;
    ServerLevel* level = mServer->getLevel(dimension->getType()->getId());
    return level != NULL && level->getChunkMap()->isTrackingBlock(pos.x, pos.y, pos.z);
}

void PlayerList::prioritiseBlockChanges(const BlockPos& pos, int dimension)
{
    ServerLevel* level = mServer->getLevel(dimension);
    if (level != NULL)
        level->getChunkMap()->prioritiseBlockChanges(pos.x, pos.y, pos.z);
}

void PlayerList::broadcastAll(boost::shared_ptr<Packet> packet, int excluded)
{
    if (!packet)
        return;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        BOOST_ASSERT(*it);
        if (*it && (*it)->getId() != excluded && (*it)->mConnection)
            (*it)->mConnection->send(packet);
    }
}

boost::shared_ptr<ServerPlayer> PlayerList::getPlayer(PlayerUID uid)
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        BOOST_ASSERT(*it);
        if ((*it)->mXuid == uid || (*it)->m_0x798 == uid)
            return *it;
    }
    return boost::shared_ptr<ServerPlayer>();
}

void PlayerList::broadcastToTeam(boost::shared_ptr<Player> source,
                               boost::shared_ptr<Packet> packet)
{
    if (!source || !packet)
        return;
    Scoreboard* scoreboard = source->GetScoreboard();
    if (!scoreboard)
        return;
    PlayerUID uid = source->mXuid == PlayerUID(0) ? source->m_0x798 : source->mXuid;
    PlayerTeam* team = scoreboard->getPlayersTeam(uid);
    if (!team)
        return;
    boost::unordered_set<PlayerUID, boost::hash<PlayerUID>, std::equal_to<PlayerUID> >* members =
        team->getPlayers();
    if (!members)
        return;
    for (boost::unordered_set<PlayerUID, boost::hash<PlayerUID>,
                              std::equal_to<PlayerUID> >::iterator it = members->begin();
         it != members->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> target = getPlayer(*it);
        if (!target || target.get() == source.get() || !target->mConnection)
            continue;
        target->mConnection->send(packet);
    }
}

void PlayerList::broadcastToAllExceptTeam(boost::shared_ptr<Player> source,
                                          boost::shared_ptr<Packet> packet)
{
    if (!packet)
        return;
    PlayerTeam* sourceTeam = NULL;
    if (source)
    {
        Scoreboard* scoreboard = source->GetScoreboard();
        if (scoreboard)
        {
            PlayerUID uid = source->mXuid == PlayerUID(0) ? source->m_0x798 : source->mXuid;
            sourceTeam = scoreboard->getPlayersTeam(uid);
        }
    }
    if (!sourceTeam)
    {
        broadcastAll(packet);
        return;
    }
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        if (!*it || !(*it)->mConnection)
            continue;
        if (source && *it == source)
            continue;
        Scoreboard* scoreboard = (*it)->GetScoreboard();
        if (!scoreboard)
            continue;
        PlayerUID uid = (*it)->mXuid == PlayerUID(0) ? (*it)->m_0x798 : (*it)->mXuid;
        if (scoreboard->getPlayersTeam(uid) == sourceTeam)
            continue;
        (*it)->mConnection->send(packet);
    }
}

void PlayerList::broadcastPlaylistInfo(std::vector<LevelRules::LevelRulesetGroup*>* groups)
{
    if (!mGameMode)
        return;
    mGameMode->UpdateRulePlayList(groups);
    mGameMode->GeneratePlaylistSyncInfo();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        if (*it)
            mGameMode->SendPlaylistInfo(*it);
    }
}

std::wstring PlayerList::getPlayerNames()
{
    std::wstring names;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        if (it != mPlayers.begin())
            names.append(L", ");
        BOOST_ASSERT(*it);
        const std::wstring playerName = (*it)->getName();
        names.append(playerName);
    }
    return names;
}

boost::shared_ptr<ServerPlayer> PlayerList::getPlayerByName(const std::wstring& name)
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        BOOST_ASSERT(*it);
        if ((*it)->getName() == name)
            return *it;
    }
    return boost::shared_ptr<ServerPlayer>();
}

boost::shared_ptr<ServerPlayer> PlayerList::getPlayerFromStatsUID(const StatsUID& uid)
{
    return getPlayer(uid.GetID());
}

void PlayerList::broadcast(boost::shared_ptr<Player> source, double x, double y, double z,
                           double radius, int dimension, boost::shared_ptr<Packet> packet)
{
    const double radiusSquared = radius * radius;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> candidate = *it;
        if (!candidate || !candidate->mConnection)
            continue;
        if (candidate.get() == source.get())
            continue;
        if (candidate->mDimension != dimension)
            continue;
        const double dx = x - candidate->mX;
        const double dy = y - candidate->mY;
        const double dz = z - candidate->mZ;
        if (dx * dx + dy * dy + dz * dz < radiusSquared)
            candidate->mConnection->send(packet);
    }
}

void PlayerList::broadcast(double x, double y, double z, double radius, int dimension,
                           boost::shared_ptr<Packet> packet)
{
    broadcast(boost::shared_ptr<Player>(), x, y, z, radius, dimension, packet);
}

int PlayerList::getPlayerCount()
{
    return static_cast<int>(mPlayers.size());
}

int PlayerList::getPlayerCount(ServerLevel* level)
{
    if (!level || !level->mDimension)
        return 0;
    const int dimension = level->mDimension->getType()->getId();
    int count = 0;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        BOOST_ASSERT(*it);
        if (*it && (*it)->mDimension == dimension)
            ++count;
    }
    return count;
}

void PlayerList::kickPlayerByShortId(unsigned char id)
{
    mPlayersToKick.push_back(id);
}

void PlayerList::closeServerGamePacketListenerImplBySmallId(unsigned char id)
{
    mListenersToClose.push_back(id);
}

bool PlayerList::isXuidBanned(PlayerUID uid)
{
    std::vector<PlayerUID>::const_iterator it;
    for (it = mBannedPlayers.begin(); it != mBannedPlayers.end(); ++it)
        if (*it == uid)
            return true;
    return false;
}

void PlayerList::resetAllChunkFlagsOutsideArea(int minX, int minZ, int maxX, int maxZ, int flags)
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
        if (*it)
            (*it)->resetChunkFlagsOutsideArea(minX, minZ, maxX, maxZ, flags);
}
