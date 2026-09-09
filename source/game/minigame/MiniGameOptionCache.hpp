#ifndef GAME_MINIGAME_MINIGAMEOPTIONCACHE_HPP
#define GAME_MINIGAME_MINIGAMEOPTIONCACHE_HPP

#include <boost/unordered_map.hpp>
#include <utility>

#include "game/minigame/EMiniGameId.hpp"
#include "game/world/generation/MiniGamePlayerSettings.hpp"
#include "game/world/generation/PlayerUID.hpp"

class MiniGameOptionCacheKeyHasher
{
public:
    unsigned int operator()(const std::pair<PlayerUID, EMiniGameId>& key) const;
};

class MiniGameOptionCache : public boost::unordered_map<std::pair<PlayerUID, EMiniGameId>, MiniGamePlayerSettings, MiniGameOptionCacheKeyHasher>
{
public:
    typedef MiniGameOptionCacheKeyHasher KeyHasher;

    MiniGameOptionCache();
    ~MiniGameOptionCache();

    void updateCachedOptions(int playerIndex, EMiniGameId miniGameId, const MiniGamePlayerSettings& options);
    MiniGamePlayerSettings& getPreviousOptions(int playerIndex, EMiniGameId miniGameId);
};

#endif
