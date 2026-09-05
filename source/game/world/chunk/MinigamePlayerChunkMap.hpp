#ifndef GAME_WORLD_CHUNK_MINIGAMEPLAYERCHUNKMAP_HPP
#define GAME_WORLD_CHUNK_MINIGAMEPLAYERCHUNKMAP_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/chunk/PlayerChunkMap.hpp"

class ServerLevel;
class ServerPlayer;

class MinigamePlayerChunkMap : public PlayerChunkMap
{
public:
    MinigamePlayerChunkMap(ServerLevel* level, int viewDistance, int radius);
    virtual ~MinigamePlayerChunkMap();

    void updatePlayerReferences(boost::shared_ptr<ServerPlayer> player);
    virtual void add(boost::shared_ptr<ServerPlayer> player);
    virtual void remove(boost::shared_ptr<ServerPlayer> player);
    virtual void move(boost::shared_ptr<ServerPlayer> player);
    virtual void removeOutsideAreaForAllPlayers(int minX, int minZ, int maxX, int maxZ);
    void loadPlayerInActiveArea(boost::shared_ptr<ServerPlayer> player);
    virtual void reloadAllPlayers(int minX, int minZ, int maxX, int maxZ);
    virtual void onPlayerJoin(boost::shared_ptr<ServerPlayer> player);
    virtual void onPlayerLeave(boost::shared_ptr<ServerPlayer> player);
    bool containsPlayer(boost::shared_ptr<ServerPlayer> player);
};

#endif
