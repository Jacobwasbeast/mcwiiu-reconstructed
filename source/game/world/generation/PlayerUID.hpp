#ifndef GAME_WORLD_GENERATION_PLAYERUID_HPP
#define GAME_WORLD_GENERATION_PLAYERUID_HPP

#include <string>

#include "game/util/ACTUuid.hpp"

struct FFLCreateID;

class PlayerUID
{
public:
    PlayerUID(int uid);
    PlayerUID(ACTUuid uuid, bool isOnlineAccount, bool isDRCController);
    PlayerUID(FFLCreateID createId, unsigned int principalId, bool hasPrincipalId, bool isDRCController);
    PlayerUID(std::wstring uidString);

    std::wstring UuidString() const;
    std::wstring toString() const;

    bool operator==(const PlayerUID& other) const;
    bool operator!=(const PlayerUID& other) const;

private:
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

    unsigned int isUuid : 1;
    unsigned int isOnlineAccount : 1;
    unsigned int hasPrincipalId : 1;
    unsigned int isDRCController : 1;
    unsigned int reserved : 28;
};

#endif
