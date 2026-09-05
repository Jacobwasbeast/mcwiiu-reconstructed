#ifndef GAME_WORLD_GENERATION_PLAYERUID_HPP
#define GAME_WORLD_GENERATION_PLAYERUID_HPP

#include <string>

#include "game/util/ACTUuid.hpp"

struct FFLCreateID;

struct PlayerUID
{
    union
    {
        ACTUuid uuid;
        struct
        {
            unsigned char createId[10];
            union
            {
                unsigned int principalId;
                unsigned char principalIdBytes[4];
            };
        } mii;
    };
    unsigned int flags;

    PlayerUID(int uid);
    PlayerUID(ACTUuid uuid, bool onlineAccount, bool drcController);
    PlayerUID(FFLCreateID createId, unsigned int principalId, bool hasPrincipalId, bool drcController);
    PlayerUID(std::wstring uidString);

    std::wstring UuidString() const;
    std::wstring toString() const;
    bool operator==(const PlayerUID& other) const;
    bool operator!=(const PlayerUID& other) const;
};

#endif
