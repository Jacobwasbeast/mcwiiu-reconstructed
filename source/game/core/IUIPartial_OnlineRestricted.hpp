#ifndef GAME_CORE_IUIPARTIAL_ONLINERESTRICTED_HPP
#define GAME_CORE_IUIPARTIAL_ONLINERESTRICTED_HPP

#include "game/minigame/EMiniGameId.hpp"

class MiniGameDef;

class IUIPartial_OnlineRestricted
{
public:
    virtual EMiniGameId GetMiniGameId() = 0;

    bool IsUGCRestrictedMode();
    bool IsUGCRestrictedMode(int userIndex);
    MiniGameDef* GetMiniGame();
};

#endif
