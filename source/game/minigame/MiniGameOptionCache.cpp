#include "game/minigame/MiniGameOptionCache.hpp"

#include "game/core/GameModeOptions.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/PlayerUID.hpp"

PlayerUID getPlayerUID(int playerIndex, bool includeOffline);

MiniGameOptionCache gMiniGameOptionCache;

unsigned int MiniGameOptionCacheKeyHasher::operator()(const std::pair<PlayerUID, EMiniGameId>& key) const
{
    return hash_value(key.first) ^ static_cast<unsigned int>(key.second);
}

MiniGameOptionCache::MiniGameOptionCache()
{
}

MiniGameOptionCache::~MiniGameOptionCache()
{
}

void MiniGameOptionCache::updateCachedOptions(int playerIndex, EMiniGameId miniGameId, const MiniGamePlayerSettings& options)
{
    PlayerUID playerUid = getPlayerUID(playerIndex, true);
    (*this)[std::make_pair(playerUid, miniGameId)] = options;
}

MiniGamePlayerSettings& MiniGameOptionCache::getPreviousOptions(int playerIndex, EMiniGameId miniGameId)
{
    PlayerUID playerUid = getPlayerUID(playerIndex, true);
    std::pair<PlayerUID, EMiniGameId> key(playerUid, miniGameId);
    iterator it = find(key);
    if (it == end())
    {
        MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(miniGameId, true);
        MiniGamePlayerSettings options = miniGame->GetDefaultPlayerSettings();
        if (miniGameId == eMiniGameId_Tumble || miniGameId == eMiniGameId_Build)
        {
            options.mHostOptions.Set(eCustomGameModeHostOption_SpectateMode, 0);
        }
        (*this)[key] = options;
        it = find(key);
    }
    return it->second;
}
