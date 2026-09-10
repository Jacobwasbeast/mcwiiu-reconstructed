#include "game/world/generation/PlayerList.hpp"

#include <algorithm>
#include <boost/assert.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/PlayerChunkMap.hpp"
#include "game/world/generation/Player.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/Level.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/MinecraftServer.hpp"

PlayerList::PlayerList(MinecraftServer* server, MasterGameMode* gameMode)
    : mServer(server), mGameMode(gameMode), mMaxPlayers(0), mViewDistance(10),
      mMaxRange(0), mAllowCheatsForAllPlayers(false), mOverrideGameMode(false),
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

void PlayerList::validatePlayerSpawnPosition(boost::shared_ptr<ServerPlayer>)
{
}

void PlayerList::updatePlayerGameMode(boost::shared_ptr<ServerPlayer>,
                                      boost::shared_ptr<ServerPlayer>, Level*)
{
}

int PlayerList::getMaxPlayers()
{
    return mMaxPlayers;
}

void PlayerList::updateEntireScoreboard(ServerScoreboard*,
                                        boost::shared_ptr<ServerPlayer>)
{
}

void PlayerList::sendLevelInfo(boost::shared_ptr<ServerPlayer> player, ServerLevel*)
{
    if (player)
        player->resetSentInfo();
}

void PlayerList::changeDimension(boost::shared_ptr<ServerPlayer> player, ServerLevel*)
{
    if (player)
        player->resetSentInfo();
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

void PlayerList::move(boost::shared_ptr<ServerPlayer>)
{
}

boost::shared_ptr<ServerPlayer> PlayerList::getPlayerForLogin(ServerConnectionListener*,
                                                              const std::wstring&,
                                                              PlayerUID, PlayerUID)
{
    return boost::shared_ptr<ServerPlayer>();
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

void PlayerList::sendAllPlayerInfo(boost::shared_ptr<ServerPlayer>)
{
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

void PlayerList::broadcastToTeam(boost::shared_ptr<Player>, boost::shared_ptr<Packet>)
{
}

void PlayerList::broadcastToAllExceptTeam(boost::shared_ptr<Player>,
                                          boost::shared_ptr<Packet>)
{
}

void PlayerList::broadcastPlaylistInfo(std::vector<LevelRules::LevelRulesetGroup*>*)
{
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
