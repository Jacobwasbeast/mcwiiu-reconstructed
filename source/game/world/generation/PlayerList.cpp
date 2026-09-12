#include "game/world/generation/PlayerList.hpp"

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/unordered_set.hpp>
#include <cmath>
#include <limits>

#include "game/core/MasterGameMode.hpp"
#include "game/item/Inventory.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/ClientboundSetCarriedItemPacket.hpp"
#include "game/network/ClientboundSetPlayerTeamPacket.hpp"
#include "game/network/ClientboundSetSpawnPositionPacket.hpp"
#include "game/network/ClientboundSetTimePacket.hpp"
#include "game/network/ServerConnectionListener.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include "game/server/ServerScoreboard.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/PlayerChunkMap.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/world/LevelData.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/Level.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/MinecraftServer.hpp"

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
    double x = player->mX;
    double y = player->mY;
    double z = player->mZ;
    while (hasSpawnCollisions(level, player) && y < maxY)
    {
        y += 1.0;
        player->moveTo(x, y, z, player->mYRot, player->mXRot);
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
    player->changeDimension(0, 0);
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
    mMaxPlayers = 0;
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
    for (int group = 0; group < 3; ++group)
    {
        std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it;
        for (it = mReceivingPlayers[group].begin(); it != mReceivingPlayers[group].end(); ++it)
            if (*it == player)
                return true;
    }
    return false;
}

bool PlayerList::isOp(boost::shared_ptr<ServerPlayer> player)
{
    return player != 0 && canReceiveAllPackets(player);
}

int PlayerList::getViewDistance()
{
    return mViewDistance;
}

boost::shared_ptr<ServerPlayer> PlayerList::findAlivePlayerOnSystem(boost::shared_ptr<ServerPlayer> player)
{
    if (!player)
        return boost::shared_ptr<ServerPlayer>();
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it;
    for (it = mPlayers.begin(); it != mPlayers.end(); ++it)
        if (*it != player && *it)
            return *it;
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
    if (!player)
        return 0;
    if (!force && canReceiveAllPackets(player))
        return 0;
    mReceivingPlayers[0].push_back(player);
    return 1;
}

void PlayerList::broadcastAll(boost::shared_ptr<Packet> packet)
{
    if (!packet)
        return;
}

void PlayerList::save(boost::shared_ptr<ServerPlayer> player)
{
    if (player)
        mPendingPlayers.push_back(player);
}

void PlayerList::saveAll(ProgressListener*, bool)
{
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it;
    for (it = mPlayers.begin(); it != mPlayers.end(); ++it)
        save(*it);
}

void PlayerList::remove(boost::shared_ptr<ServerPlayer> player)
{
    removePlayerFromReceiving(player, true, -1);
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        if (*it == player)
            it = mPlayers.erase(it);
        else
            ++it;
    }
}

void PlayerList::removeAll()
{
    mPlayers.clear();
    for (int group = 0; group < 3; ++group)
        mReceivingPlayers[group].clear();
}

void PlayerList::load(boost::shared_ptr<ServerPlayer> player)
{
    if (player)
        mPendingPlayers.push_back(player);
}

void PlayerList::validatePlayerSpawnPosition(boost::shared_ptr<ServerPlayer> player)
{
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
    player->moveTo(x, y, z, player->mYRot, player->mXRot);
    resolveSpawnCollisions(level, player);
    const int dimensionId = level->mDimension->getType()->getId();
    const int netherId = DimensionType::NETHER->getId();
    const int endId = DimensionType::THE_END->getId();
    if ((dimensionId == netherId && player->mY > 125.0)
        || (dimensionId == endId && player->mY >= std::numeric_limits<double>::max()))
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
    ServerLevel* newLevel = player->getLevel();
    if (level != NULL)
        level->getChunkMap()->remove(player);
    newLevel->getChunkMap()->add(player);
    newLevel->getChunkSource()->create(static_cast<int>(player->mX) >> 4,
                                              static_cast<int>(player->mZ) >> 4);
}

void PlayerList::add(boost::shared_ptr<ServerPlayer> player)
{
    if (player)
        mPlayers.push_back(player);
}

void PlayerList::placeNewPlayer(Connection*, boost::shared_ptr<ServerPlayer> player,
                                boost::shared_ptr<ClientboundLoginPacket>)
{
    add(player);
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
                                                    int, bool)
{
    return player;
}

boost::shared_ptr<Entity> PlayerList::repositionAcrossDimension(boost::shared_ptr<Entity> entity,
                                                                int, ServerLevel*, ServerLevel*)
{
    return entity;
}

void PlayerList::sendAllPlayerInfo(boost::shared_ptr<ServerPlayer> player)
{
    if (!player)
        return;
    player->refreshContainer(player->mContainerMenu);
    player->resetSentInfo();
    if (!player->mConnection || !player->mInventory)
        return;
    player->mConnection->send(boost::shared_ptr<Packet>(
        new ClientboundSetCarriedItemPacket(player->mInventory->m_selected)));
}

void PlayerList::toggleDimension(boost::shared_ptr<ServerPlayer> player, int dimension)
{
    if (player)
        player->changeDimension(0, dimension);
}

void PlayerList::tick()
{
    mPendingPlayers.clear();
}

bool PlayerList::isTrackingBlock(const BlockPos& pos, const Dimension* dimension)
{
    return !mPlayers.empty();
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
    (void)packet;
    (void)excluded;
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
    if (!packet)
        return;
    const double radiusSquared = radius * radius;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
    {
        if (!*it || *it == source || !(*it)->mConnection)
            continue;
        (void)x; (void)y; (void)z; (void)radiusSquared; (void)dimension;
        (void)packet;
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

int PlayerList::getPlayerCount(ServerLevel*)
{
    return getPlayerCount();
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
