#include "game/world/generation/PlayerList.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/unordered_set.hpp>
#include <cmath>
#include <limits>
#include <set>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameRulesInstance.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/Settings.hpp"
#include "game/core/Socket.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PortalForcer.hpp"
#include "game/server/ServerChunkCache.hpp"
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
#include "game/network/ClientboundSetExperiencePacket.hpp"
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
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
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
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/minigame/Tutorial.hpp"
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
    : mServer(server), mGameMode(gameMode), mPlayerIO(NULL), mUnknownState(false), mOverrideGameMode(NULL),
      mAllowCheatsForAllPlayers(false), mViewDistance(10), mMaxRange(0)
{
    mMaxPlayers = server->mSettings->getInt(eServerSetting_MaxPlayers, 20);
    InitializeCriticalSection(&mPlayersToKickLock);
    InitializeCriticalSection(&mListenersToCloseLock);
}

PlayerList::~PlayerList()
{
    DeleteCriticalSection(&mPlayersToKickLock);
    DeleteCriticalSection(&mListenersToCloseLock);
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
    if (!player || !g_pApp || g_pApp->GetGameHostOption(eGameHostOption_CheatsEnabled) == 0)
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
    mPlayerIO->save(boost::static_pointer_cast<Player>(player));
}

void PlayerList::saveAll(ProgressListener* listener, bool)
{
    if (listener != NULL)
        listener->progressStart(IDS_PROGRESS_SAVING_PLAYERS);

    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it;
    for (it = mPlayers.begin(); it != mPlayers.end(); ++it)
        save(*it);

    if (mPlayerIO)
    {
        mPlayerIO->saveMapIdLookup();
        mPlayerIO->saveAllCachedData();
        mPlayerIO->clearOldPlayerFiles();
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
                rootVehicle->getIndirectPassengers(EntityType::SERVER_PLAYER);
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
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
        mPlayers[i]->mConnection->disconnect(DisconnectPacket::eDisconnectReason_ConnectionClosed);
}

CompoundTag* PlayerList::load(boost::shared_ptr<ServerPlayer> player)
{
    return mPlayerIO->load(boost::static_pointer_cast<Player>(player));
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
    const bool daylightCycle = gameRules->getBoolean(GameRules::RULE_DAYLIGHT);
    player->mConnection->send(boost::shared_ptr<Packet>(
        new ClientboundSetTimePacket(gameTime, dayTime, daylightCycle)));
    if (level->isRaining())
    {
        player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundGameEventPacket(ClientboundGameEventPacket::START_RAINING, 0.0f)));
    }
    else
    {
        player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundGameEventPacket(ClientboundGameEventPacket::STOP_RAINING, 0.0f)));
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
    changeDimension(player, NULL);
    level->addEntity(player);

    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it;
    for (it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        if ((*it)->mConnection->getNetworkPlayer() != NULL)
            player->mConnection->send(boost::shared_ptr<Packet>(new PlayerInfoPacket(*it)));
    }

    if (level->somePlayerSleeping)
    {
        boost::shared_ptr<ServerPlayer> firstSleeper;
        for (unsigned int i = 0; i < mPlayers.size(); ++i)
        {
            boost::shared_ptr<ServerPlayer> sleeper = mPlayers[i];
            if (!sleeper->isSleeping())
                continue;
            if (firstSleeper == NULL)
                firstSleeper = sleeper;
            sleeper->mConnection->send(boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(
                sleeper->getName(), ClientboundChatPacket::eChatPacketMessage_BedMeSleep, -1)));
        }
        player->mConnection->send(boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(
            firstSleeper->getName(), ClientboundChatPacket::eChatPacketMessage_BedPlayerSleep, -1)));
    }

    if (mGameMode)
        mGameMode->OnJoin(player);
    level->getChunkMap()->onPlayerJoin(player);
}

void PlayerList::placeNewPlayer(Connection* connection, boost::shared_ptr<ServerPlayer> player,
                                boost::shared_ptr<ClientboundLoginPacket> loginPacket)
{
    CompoundTag* playerTag = NULL;
    if (g_pApp->GetGameHostOption(eGameHostOption_MiniGame) == 0)
        playerTag = load(player);

    ServerLevel* level = mServer->getLevel(player->mDimension);
    player->setLevel(level);
    player->mGameMode->setLevel(level);

    INetworkPlayer* networkPlayer = connection->m_socket->getPlayer();
    if (networkPlayer != NULL && networkPlayer->IsHost())
    {
        player->enableAllPlayerPrivileges(true);
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Moderator, 1);
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
        if (g_pApp->GetGameHostOption(eGameHostOption_MiniGame) == 0)
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
        (Player::getPlayerGamePrivilege(privileges, Player::ePlayerGamePrivilege_CreativeMode) ?
            GameType::CREATIVE : GameType::SURVIVAL)->getId());
    if (player->mGameMode->isAdventure())
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CreativeMode, 0);
    else if (player->mGameMode->GetGameType() != privilegeGameType)
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CreativeMode,
                                       player->mGameMode->GetGameType()->getId());

    player->mConnection = connectionListener;
    connectionListener->mFriendsOnly = loginPacket->mHardcore;
    connectionListener->mPlayerUID = loginPacket->mPlayerUID;
    connectionListener->mOldPlayerUID = loginPacket->mPlayerUID2;
    if (loginPacket->mHardcore)
        ++mServer->ugcPlayersVersion;
    addPlayerToReceiving(player, false);

    LevelData* levelData = level->getLevelData();
    connectionListener->send(boost::shared_ptr<Packet>(new ClientboundLoginPacket(
        L"", player->getId(), levelData->getGeneratorType(), level->getSeed(),
        player->mGameMode->GetGameType()->getId(),
        g_pApp->GetGameHostOption(eGameHostOption_MiniGame),
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
        player->getName(), ClientboundChatPacket::eChatPacketMessage_PlayerJoined, -1)));

    add(player);
    player->doTick(true, true, true);

    connectionListener->teleport(player->mX, player->mY, player->mZ,
                                 player->mYRot, player->mXRot, true, true);
    mServer->getConnection()->addServerGamePacketListenerImpl(connectionListener);

    connectionListener->send(boost::shared_ptr<Packet>(new ClientboundSetTimePacket(
        level->getGameTime(), level->getDayTime(), level->getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT))));

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
    if (playerTag != NULL && playerTag->contains(L"RootVehicle", Tag::TAG_COMPOUND))
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
    mPlayerIO = levels.data[0]->getLevelStorage()->getPlayerIO();
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
    newPlayer->mOnlineXuid = oldUid;

    INetworkPlayer* networkPlayer = listener->m_connection->m_socket->getPlayer();
    if (networkPlayer != NULL && !networkPlayer->IsHost())
    {
        newPlayer->enableAllPlayerPrivileges(gApp.GetGameHostOption(eGameHostOption_TrustPlayers) != 0);
    }

    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (ruleset != NULL)
    {
        newPlayer->mGameMode->setGameRules(static_cast<GameRulesInstance*>(GameRuleDefinition::generateNewGameRulesInstance(
            GameRulesInstance::eGameRulesInstanceType_Player, ruleset, listener->m_connection)));
        newPlayer->UpdateSize();
    }
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

    const bool receivingAllPackets = canReceiveAllPackets(player);
    const int oldDimension = player->mDimension;
    bool leavingSystem = dimension != oldDimension;
    if (leavingSystem)
    {
        INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
        for (unsigned int i = 0; i < mPlayers.size(); ++i)
        {
            boost::shared_ptr<ServerPlayer> other = mPlayers[i];
            if (other == player || other->mDimension != oldDimension)
                continue;
            INetworkPlayer* otherNetworkPlayer = other->mConnection->getNetworkPlayer();
            if (otherNetworkPlayer != NULL && networkPlayer->IsSameSystem(otherNetworkPlayer))
                leavingSystem = false;
        }
    }
    if (leavingSystem && receivingAllPackets)
        player->getLevel()->getTracker()->clear(player);
    else
        player->getLevel()->getTracker()->removeEntity(player);

    player->getLevel()->getChunkMap()->remove(player);
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator oldPlayer =
        std::find(mPlayers.begin(), mPlayers.end(), player);
    if (oldPlayer != mPlayers.end())
        mPlayers.erase(oldPlayer);
    mServer->getLevel(player->mDimension)->removeEntityImmediately(player);

    BlockPos* respawnPosition = player->getRespawnPosition();
    const bool respawnForced = player->isRespawnForced() != 0;
    const bool wasReceiving = removePlayerFromReceiving(player, true, 0) != 0;
    player->mDimension = dimension;
    ServerLevel* newLevel = mServer->getLevel(player->mDimension);

    ServerPlayerGameMode* newGameMode = new ServerPlayerGameMode(newLevel, mGameMode);
    boost::shared_ptr<ServerPlayer> replacement(new ServerPlayer(
        mServer, newLevel, player->getName(), newGameMode));
    replacement->mConnection = player->mConnection;
    replacement->restoreFrom(player, conqueredEnd);

    replacement->setMainArm(player->getMainArm());
    if (conqueredEnd)
        replacement->mInventory->m_selected = player->mInventory->m_selected;
    replacement->mIsGuest = player->mIsGuest;
    replacement->setXuid(player->mXuid);
    replacement->mOnlineXuid = player->mOnlineXuid;
    replacement->mSuid = player->mSuid;
    replacement->setPlayerDefaultSkin(static_cast<EDefaultSkins>(player->mField0x7d8));
    replacement->mPlayerIndex = player->mPlayerIndex;
    replacement->setCustomSkin(player->mCustomSkin);
    replacement->setCustomCape(player->mCustomCape);
    replacement->setPlayerGamePrivilege(
        Player::ePlayerGamePrivilege_All,
        player->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All));
    replacement->mGameMode->setGameRules(player->mGameMode->mGameRules);
    player->mGameMode->nullifyGameRules();
    replacement->mDimension = dimension;
    replacement->mConnection->mPlayer = replacement;

    addPlayerToReceiving(replacement, wasReceiving && oldDimension == dimension);
    updatePlayerGameMode(replacement, player, mServer->getLevel(player->mDimension));
    replacement->resetPos();
    const bool useSavedRespawn = !player->mRespawnState || dimension != oldDimension
        || player->getHealth() <= 0.0f;
    if (useSavedRespawn)
    {
        if (respawnPosition != NULL)
        {
            BlockPos* validPosition = Player::checkBedValidRespawnPosition(
                mServer->getLevel(player->mDimension), *respawnPosition, respawnForced);
            if (validPosition == NULL)
            {
                replacement->mConnection->send(boost::shared_ptr<Packet>(
                    new ClientboundChatPacket(
                        ClientboundChatPacket::eChatPacketMessage_BedNotValid, -1)));
            }
            else
            {
                replacement->moveTo(validPosition->x + 0.5f, validPosition->y + 0.1f,
                                    validPosition->z + 0.5f, 0.0f, 0.0f);
                replacement->setRespawnPosition(new BlockPos(*respawnPosition), respawnForced);
                delete validPosition;
            }
        }
    }
    else
    {
        replacement->moveTo(player->mX, player->mY, player->mZ,
                            player->mYRot, player->mXRot);
        if (respawnPosition != NULL)
            replacement->setRespawnPosition(new BlockPos(*respawnPosition), respawnForced);
        replacement->mInventory->m_selected = player->mInventory->m_selected;
    }

    if (mGameMode != NULL)
        mGameMode->OnSpawn(replacement, player, true);

    newLevel->getChunkSource()->create(static_cast<int>(replacement->mX) >> 4,
                                       static_cast<int>(replacement->mZ) >> 4);
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        std::vector<AABB*>* collisions = newLevel->getCollisionAABBs(
            replacement, replacement->mBoundingBox, false, false, false);
        while (!collisions->empty())
        {
            replacement->setPos(replacement->mX, replacement->mY + 1.0, replacement->mZ);
            collisions = newLevel->getCollisionAABBs(
                replacement, replacement->mBoundingBox, false, false, false);
        }
    }

    LevelData* levelData = newLevel->getLevelData();
    const int packetXZSize = levelData->getXZSize();
    const int packetHellScale = levelData->getHellScale();
    const int packetCloudHeight = levelData->getCloudHeight();
    const bool packetModernEnd = levelData->getModernEnd();
    const unsigned char packetBiomeScale =
        static_cast<unsigned char>(levelData->getBiomeScale());
    const int packetBiomeCentreX = levelData->getBiomeCentreXChunk();
    const int packetBiomeCentreZ = levelData->getBiomeCentreZChunk();
    const bool seaLevelEnabled = newLevel->useNewSeaLevel();
    const int seaLevel = seaLevelEnabled ? newLevel->getSeaLevel() : -1;
    const long long seed = newLevel->getSeed();
    const GameType* gameType = replacement->mGameMode->GetGameType();
    const Difficulty* difficulty = newLevel->getDifficulty();
    LevelType* levelType = levelData->getGeneratorType();
    const bool packetSeaLevelEnabled = newLevel->useNewSeaLevel();
    replacement->mConnection->send(boost::shared_ptr<Packet>(
        new ClientboundRespawnPacket(
            replacement->mX, replacement->mY, replacement->mZ, replacement->mYRot,
            replacement->mXRot, static_cast<char>(replacement->mDimension),
            seed, gameType, difficulty, levelType,
            packetSeaLevelEnabled, replacement->getId(), packetXZSize, packetHellScale,
            packetCloudHeight, packetModernEnd, packetBiomeScale,
            packetBiomeCentreX, packetBiomeCentreZ,
            conqueredEnd, seaLevel)));
    replacement->mConnection->teleport(replacement->mX, replacement->mY, replacement->mZ,
                                       replacement->mYRot, replacement->mXRot, true, true);
    replacement->mConnection->send(boost::shared_ptr<Packet>(
        new ClientboundSetExperiencePacket(replacement->mExperienceProgress,
                                           replacement->mTotalExperience,
                                           replacement->mExperienceLevel)));

    if (conqueredEnd)
    {
        std::vector<MobEffectInstance*>* effects = player->getActiveEffects();
        for (std::vector<MobEffectInstance*>::iterator it = effects->begin();
             it != effects->end(); ++it)
            replacement->addEffect(new MobEffectInstance(*it), boost::shared_ptr<Entity>());
        delete effects;

        boost::shared_ptr<SynchedEntityData> entityData = replacement->getEntityData();
        entityData->set(Entity::DATA_SHARED_FLAGS_ID,
                        entityData->get(Entity::DATA_SHARED_FLAGS_ID), true);
    }

    sendLevelInfo(replacement, newLevel);
    newLevel->getChunkMap()->add(replacement);
    newLevel->addEntity(replacement);
    mPlayers.push_back(replacement);
    replacement->initMenu();

    replacement->setHealth(conqueredEnd ? player->getHealth() : replacement->getHealth());

    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->isTutorial())
    {
        Tutorial* tutorial = minecraft->mGameMode->getTutorial();
        if (tutorial != NULL
            && !tutorial->isStateCompleted(eTutorial_State_FoodBar))
            g_pApp->getGameRuleDefinitions()->postProcessPlayer(replacement);
    }
    if (oldDimension == 1 && replacement->mDimension != 1)
        replacement->displayClientMessage(IDS_PLAYER_LEFT_END, true);
    if (mGameMode != NULL)
        mGameMode->OnSpawned(replacement, true);

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
        if (*it)
            (*it)->notifyPlayerRespawn(&player, &replacement);

    return replacement;
}

void PlayerList::repositionAcrossDimension(boost::shared_ptr<Entity> entity, int lastDimension,
                                           ServerLevel* oldLevel, ServerLevel* newLevel)
{
    const double originalX = entity->mX;
    const double originalZ = entity->mZ;
    double x = entity->mX;
    double z = entity->mZ;
    const float rotation = entity->mYRot;
    const double scale = static_cast<double>(newLevel->getLevelData()->getHellScale());

    if (entity->mDimension == -1)
    {
        x /= scale;
        z /= scale;
        entity->moveTo(x, entity->mY, z, entity->mYRot, entity->mXRot);
        if (entity->isAlive())
        {
            oldLevel->tick(entity, false);
        }
    }
    else if (entity->mDimension == 0)
    {
        x *= scale;
        z *= scale;
        entity->moveTo(x, entity->mY, z, entity->mYRot, entity->mXRot);
        if (entity->isAlive())
        {
            oldLevel->tick(entity, false);
        }
    }
    else
    {
        BlockPos spawn;
        if (lastDimension == 1)
        {
            spawn = newLevel->getSharedSpawnPos();
        }
        else
        {
            spawn = *newLevel->getDimensionSpecificSpawn();
        }
        x = static_cast<double>(spawn.x);
        entity->mY = static_cast<double>(spawn.y);
        z = static_cast<double>(spawn.z);
        entity->moveTo(x, entity->mY, z, 90.0f, 0.0f);
        if (entity->isAlive())
        {
            oldLevel->tick(entity, false);
        }
    }

    if (entity->GetType() == EntityType::SERVER_PLAYER)
    {
        boost::shared_ptr<ServerPlayer> serverPlayer = boost::dynamic_pointer_cast<ServerPlayer>(entity);
        removePlayerFromReceiving(serverPlayer, false, lastDimension);
        addPlayerToReceiving(serverPlayer, false);
    }

    if (lastDimension != 1)
    {
        newLevel->clampWithinLevelBounds(x, z, 0.0);
        if (entity->isAlive())
        {
            entity->moveTo(x, entity->mY, z, entity->mYRot, entity->mXRot);
            ServerChunkCache* chunkCache = static_cast<ServerChunkCache*>(newLevel->getChunkSource());
            chunkCache->m_createOnRequest = true;
            newLevel->getPortalForcer()->force(entity, rotation, static_cast<int>(originalX),
                                               static_cast<int>(originalZ));
            chunkCache->m_createOnRequest = false;
            newLevel->addEntity(entity);
            newLevel->tick(entity, false);
        }
    }

    entity->setLevel(newLevel);
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
            player->displayClientMessage(static_cast<int>(IDS_PLAYER_LEFT_END), true);
    }
    else
    {
        player->displayClientMessage(static_cast<int>(IDS_PLAYER_ENTERED_END), true);
    }

    player->mDimension = dimension;
    ServerLevel* newLevel = mServer->getLevel(player->mDimension);
    player->clearFire();
    player->flushEntitiesToRemove();

    LevelData* levelData = newLevel->getLevelData();
    const GameType* gameType = player->mGameMode->GetGameType();
    const Difficulty* difficulty = newLevel->getDifficulty();
    LevelType* levelType = levelData->getGeneratorType();
    const bool useNewSeaLevel = newLevel->useNewSeaLevel();
    const int seaLevel = useNewSeaLevel ? newLevel->getSeaLevel() : -1;
    boost::shared_ptr<Packet> respawn(new ClientboundRespawnPacket(
        static_cast<char>(dimension), newLevel->getSeed(), gameType, difficulty, levelType,
        useNewSeaLevel, player->getId(), levelData->getXZSize(), levelData->getHellScale(),
        levelData->getCloudHeight(), levelData->getModernEnd(),
        static_cast<unsigned char>(levelData->getBiomeScale()), levelData->getBiomeCentreXChunk(),
        levelData->getBiomeCentreZChunk(), true, seaLevel));
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
            chunk, newLevel->mDimension->getType()->getId());
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
    EnterCriticalSection(&mListenersToCloseLock);
    while (!mListenersToClose.empty())
    {
        unsigned char smallId = mListenersToClose.front();
        mListenersToClose.pop_front();
        boost::shared_ptr<ServerPlayer> closing;
        for (unsigned int i = 0; i < mPlayers.size(); ++i)
        {
            boost::shared_ptr<ServerPlayer> player = mPlayers.at(i);
            if (player && player->mConnection && player->mConnection->mConnection
                && player->mConnection->mConnection->m_socket
                && player->mConnection->mConnection->m_socket->mPlayerId == smallId)
            {
                closing = player;
                break;
            }
        }
        if (closing)
            closing->mConnection->disconnect(DisconnectPacket::eDisconnectReason_ConnectionClosed);
    }
    LeaveCriticalSection(&mListenersToCloseLock);

    EnterCriticalSection(&mPlayersToKickLock);
    while (!mPlayersToKick.empty())
    {
        unsigned char smallId = mPlayersToKick.front();
        mPlayersToKick.pop_front();
        INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerBySmallId(smallId);
        if (networkPlayer == NULL || networkPlayer->IsLocal())
            continue;

        PlayerUID uid = networkPlayer->GetUID();
        boost::shared_ptr<ServerPlayer> kicked;
        for (unsigned int i = 0; i < mPlayers.size(); ++i)
        {
            boost::shared_ptr<ServerPlayer> player = mPlayers.at(i);
            if (player && gProfile->AreXUIDSEqual(player->mOnlineXuid, uid))
            {
                kicked = player;
                break;
            }
        }
        if (kicked)
        {
            mBannedPlayers.push_back(kicked->mOnlineXuid);
            kicked->enableAllPlayerPrivileges(false);
            kicked->mConnection->mUseAltLeaveMessage = true;
            kicked->mConnection->send(boost::shared_ptr<Packet>(
                new DisconnectPacket(DisconnectPacket::eDisconnectReason_Kicked)));
        }
    }
    LeaveCriticalSection(&mPlayersToKickLock);

    for (int dimension = 0; dimension < 3; ++dimension)
    {
        for (unsigned int i = 0; i < mReceivingPlayers[dimension].size(); ++i)
        {
            boost::shared_ptr<ServerPlayer> player = mReceivingPlayers[dimension][i];
            if (player && player->mRemoved)
            {
                boost::shared_ptr<ServerPlayer> replacement = findAlivePlayerOnSystem(player);
                if (replacement)
                    mReceivingPlayers[dimension][i] = replacement;
            }
        }
    }
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
        if ((*it)->mXuid == uid || (*it)->mOnlineXuid == uid)
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
    PlayerUID uid = source->mXuid == PlayerUID(0) ? source->mOnlineXuid : source->mXuid;
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
            PlayerUID uid = source->mXuid == PlayerUID(0) ? source->mOnlineXuid : source->mXuid;
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
        PlayerUID uid = (*it)->mXuid == PlayerUID(0) ? (*it)->mOnlineXuid : (*it)->mXuid;
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
    int count = 0;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        if ((*it)->mLevel == level)
            ++count;
    }
    return count;
}

void PlayerList::kickPlayerByShortId(unsigned char id)
{
    EnterCriticalSection(&mPlayersToKickLock);
    mPlayersToKick.push_back(id);
    LeaveCriticalSection(&mPlayersToKickLock);
}

void PlayerList::closeServerGamePacketListenerImplBySmallId(unsigned char id)
{
    EnterCriticalSection(&mListenersToCloseLock);
    mListenersToClose.push_back(id);
    LeaveCriticalSection(&mListenersToCloseLock);
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
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin(); it < mPlayers.end(); ++it)
        (*it)->resetChunkFlagsOutsideArea(minX, minZ, maxX, maxZ, flags);
}
